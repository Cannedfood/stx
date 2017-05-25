#include "../include/stx/socket.hpp"
#include "../include/stx/logging.hpp"

#include <utility>

#ifdef _WIN32
extern "C" {
#	include <winsock.h>
}
#else // -> Unix

extern "C" {
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <unistd.h>

#	include <errno.h> // errno
#	include <string.h> // strerror
}

#	define SOCKET_ERROR -1

	inline
	int closesocket(int s) { return ::close(s); }

	inline
	const char* sockerr() { return ::strerror(errno); }
#endif

namespace stx {

// == stx::detail::socket_based ==============================================================

namespace detail {

socket_based::socket_based() :
	m_socket(SOCKET_ERROR),
	m_type(socket_type_invalid),
	m_protocol(socket_protocol_invalid)
{}

socket_based::socket_based(socket_based&& s) :
	m_socket(s.m_socket),
	m_type(s.m_type),
	m_protocol(s.m_protocol)
{
	s.m_socket    = SOCKET_ERROR;
	s.m_type      = socket_type_invalid;
	s.m_protocol  = socket_protocol_invalid;
}

socket_based::~socket_based() noexcept {
	if(m_socket) {
		close();
	}
}

socket_based& socket_based::operator=(socket_based&& other) noexcept {
	std::swap(m_socket,    other.m_socket);
	std::swap(m_type,      other.m_type);
	std::swap(m_protocol, other.m_protocol);
	other.close();
	return *this;
}

bool socket_based::open(socket_type type, socket_protocol proto) {
	if(m_socket) return false;

	if(type  == socket_type_invalid)     return false;
	if(proto == socket_protocol_invalid) return false;

	constexpr static const int domain[] = {
		-1,           //<! INVALID
		AF_INET,      //<! ipv4, internet protocol (ip) version 4
		AF_INET6,     //<! ipv6, internet protocol (ip) version 6
		AF_IRDA,      //<! irda, e.g. infrared sensors
		AF_BLUETOOTH, //<! bluetooth
		AF_UNIX       //<! unix_local; unix domain sockets for interprocess communication
	};

	constexpr static const int type_values[] = {
		-1,          //<! INVALID
		SOCK_STREAM, //<! tcp
		SOCK_DGRAM   //<! udp
	};

	m_socket = ::socket(domain[type], type_values[type], 0);
	if(!is_open()) {

		error("Error opening socket: ", sockerr());
		return false;
	}
	else {
		m_type     = type;
		m_protocol = proto;
		return true;
	}
}

void socket_based::close() {
	if(is_open()) {
		if(::closesocket(m_socket)) {
			error("Failed closing socket: ", sockerr());
		}
		m_socket   = SOCKET_ERROR;
		m_type     = socket_type_invalid;
		m_protocol = socket_protocol_invalid;
	}
}

} // namespace detail

// == stx::server ==============================================================

server::server() :
	socket_based()
{}

server::~server() noexcept {
	close();
}

bool server::open(socket_type type, socket_protocol proto) noexcept {
	socket_based::open(type, proto);
	return false;
}

void server::close() noexcept {
	// if(is_open()) {
		socket_based::close();
	// }
}


// == stx::client ==============================================================

client::client() :
	socket_based()
{}

client::~client() noexcept {
	close();
}

void client::close() noexcept {
	// if(is_open()) {
		socket_based::close();
	// }
}

} // namespace stx
