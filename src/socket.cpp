#include "../include/stx/socket.hpp"

extern "C" {
#include <fcntl.h>
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>
#include <unistd.h> // close
}

#include <cstring> // strerror

using namespace std::chrono;
using namespace std::chrono_literals;

namespace stx {

// == socket ==============================================================

// -- socket: Constructors; Moving & Copying --------------------------------------

socket::socket() : m_handle(-1), m_refcount(0) {}
socket::socket(socket const& other) : socket() { *this = other; }
socket::socket(socket&& other) : socket() { *this = std::move(other); }
socket::~socket() { close(); }

socket& socket::operator=(socket&& other) {
	close();
	std::swap(m_handle, other.m_handle);
	std::swap(m_refcount, other.m_refcount);
	return *this;
}

socket& socket::operator=(stx::socket const& other) {
	m_handle   = other.m_handle;
	m_refcount = other.m_refcount;
	if(is_open()) ++*m_refcount;
	return *this;
}


// -- socket: General -----------------------------------------------------

socket& socket::open(sockdomain d, sockprotocol p) {
	if(is_open()) {
		throw invalid_operation(
		    "Cannot open an already open socket. Please close it first or "
		    "create a new one");
	}

	if(d == sockdomain_invalid || d >= _num_sockdomains) {
		throw invalid_operation("invalid socket domain: " + std::to_string(d));
	}

	if(p == sockprotocol_invalid || p >= _num_sockprotocols) {
		throw invalid_operation("invalid socket protocol: " +
		                        std::to_string(p));
	}

	constexpr static int domains[_num_sockdomains] = {
	    -1, AF_INET, AF_INET6, AF_IRDA, AF_BLUETOOTH, AF_UNIX};

	constexpr static int protocols[_num_sockprotocols] = {
	    -1, SOCK_DGRAM, SOCK_STREAM};

	m_handle = ::socket(domains[d], protocols[p], 0);
	if(!is_open()) {
		throw failed_operation(std::string("Failed opening socket: ") +
		                       strerror(errno));
	}

	m_refcount = new int(1);

	return *this;
}

socket& socket::open(sockdomain d, sockprotocol p, uint16_t port) {
	return open(d, p).bind(port);
}

socket& socket::close() {
	if(is_open()) {
		if(--*m_refcount == 0) {
			delete m_refcount;
			::close(m_handle);
		}
		m_handle   = -1;
		m_refcount = nullptr;
	}
	return *this;
}

socket& socket::bind(uint16_t port) {
	if(!is_open())
		throw invalid_operation("Cannot bind a socket which was not opened.");

	sockaddr_in address;
	address.sin_family      = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port        = htons(port);

	if(::bind(m_handle, (sockaddr*)&address, sizeof(address))) {
		throw failed_operation("Failed binding socket to port " +
		                       std::to_string(port) + ": " + strerror(errno));
	}

	return *this;
}

// -- socket: Server -------------------------------------------------------

connection socket::accept(size_t                    max_queued,
                          std::chrono::milliseconds timeout) {
	if(!is_open()) {
		throw invalid_operation(
		    "Cannot accept connections on an unopened socket.");
	}

	// TODO: check if bound?

	{
		int status;

		if(timeout < 0ms) {
			blocking(true);
			status = ::listen(m_handle, max_queued);
		} else {
			throw invalid_operation("Timeouts not yet implemented");

			auto beg = high_resolution_clock::now();
			do {
				status = ::listen(m_handle, max_queued);

				if(high_resolution_clock::now() - beg > timeout) {
					puts("Timeout");
					return connection();
				}
			} while(status == EWOULDBLOCK);
		}

		if(status) {
			printf("Error waiting for incoming connections in accept: %s\n", strerror(errno));
			return connection();
		}
	}

	connection c;

	// TODO: What connection members should we extract from the adress struct?
	sockaddr_in address;
	socklen_t   len = sizeof(address);

	c.m_handle = ::accept(m_handle, (sockaddr*)&address, &len);
	if(c) { // Only assign socket if connection is valid
		c.m_socket = *this;
	}
	else {
		printf("Failed opening connection: %s\n", strerror(errno));
	}

	return c;
}

connection socket::accept(sockdomain                d,
                          sockprotocol              p,
                          uint16_t                  port,
                          size_t                    max_queued,
                          std::chrono::milliseconds timeout) {
	return open(d, p, port).accept(max_queued, timeout);
}

// -- socket: Client --------------------------------------------------------------


// -- socket: Getters & Setters --------------------------------------------------


sockprotocol socket::protocol() const noexcept {
	if(!is_open()) { return sockprotocol_invalid; }

	int       type;
	socklen_t type_size = sizeof(type);

	if(getsockopt(m_handle, SOL_SOCKET, SO_TYPE, &type, &type_size)) {
		return sockprotocol_invalid;
	}

	if(type == SOCK_STREAM) return sockprotocol::tcp;
	if(type == SOCK_DGRAM) return sockprotocol::udp;
	return sockprotocol_invalid;
}

// TODO: implement sockdomain socket::domain() const noexcept;

size_t socket::references() const noexcept {
	if(!is_open()) return 0;
	return *m_refcount;
}

bool socket::listening() const noexcept {
	if(!is_open()) return false;

	int       listens;
	socklen_t listens_size = sizeof(listens);

	if(getsockopt(
	       m_handle, SOL_SOCKET, SO_ACCEPTCONN, &listens, &listens_size)) {
		return false;
	}

	return listens != 0;
}

bool socket::blocking() const noexcept {
	return (fcntl(m_handle, F_GETFL, 0) & O_NONBLOCK) == 0;
}

socket& socket::blocking(bool b) {
	int flags = fcntl(m_handle, F_GETFL, 0);

	int result;
	if(b) {
		result = fcntl(m_handle, F_SETFL, flags & ~O_NONBLOCK);
	}
	else {
		result = fcntl(m_handle, F_SETFL, flags | O_NONBLOCK);
	}

	if(result != 0) {
		close();
		throw failed_operation(
		    std::string("Could not set the socket::blocking to ") + (b ? "true" : "false") + ": " +
		    strerror(errno));
	}

	return *this;
}

// == connection ==============================================================

// -- Constructors; Copy & Move behavior --------------------------------------
connection::connection() : m_socket(), m_handle(-1) {}

connection::connection(connection&& other) : connection() {
	*this = std::move(other);
}

connection::~connection() { close(); }

connection& connection::operator=(connection&& other) {
	close();
	std::swap(m_handle, other.m_handle);
	std::swap(m_socket, other.m_socket);
	return *this;
}

// -- connection: General --------------------------------------------------------------
connection& connection::close() {
	if(*this) {
		::close(m_handle);
		m_handle = -1;
		m_socket.close();
	}
	return *this;
}

// -- connection: IO --------------------------------------------------------------
void connection::write_s(const char* s) {
	if(!(*this)) {
		throw socket::invalid_operation("Cannot write to invalid connection");
	}

	write(m_handle, s, strlen(s));
}

} // namespace stx
