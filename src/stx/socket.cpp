#include "socket.hpp"

#include <cassert>
#include <string>

extern "C" {
	#include <memory.h>
}

namespace stx {

address ipv4(uint32_t ip, uint16_t port) noexcept {
	address addr;
	sockaddr_in addr_thing {
		.sin_family = (sa_family_t)domain::ipv4,
		.sin_port = htons(port),
		.sin_addr = { .s_addr = htonl(ip) },
		.sin_zero = {}
	};
	memcpy((void*)(sockaddr_storage*)&addr, &addr_thing, sizeof(addr_thing));
	addr.length = sizeof(addr_thing);
	return addr;
}
address ipv4(uint8_t ip_1, uint8_t ip_2, uint8_t ip_3, uint8_t ip_4, uint16_t port) noexcept {
	return ipv4(uint32_t(ip_1) << 24 | uint32_t(ip_2) << 16 | uint32_t(ip_3) << 8 | uint32_t(ip_4) << 0, port);
}
address ipv4(uint16_t port) noexcept {
	return ipv4(INADDR_ANY, port);
}

std::string to_string(address const& addr) noexcept {
	switch(addr.ss_family){
		case (int)domain::ipv4: {
			uint32_t ip   = ntohl(((sockaddr_in&)addr).sin_addr.s_addr);
			uint16_t port = ntohs(((sockaddr_in&)addr).sin_port);

			return
				std::to_string(0xFFu & (ip >> 24)) + "." +
				std::to_string(0xFFu & (ip >> 24)) + "." +
				std::to_string(0xFFu & (ip >> 24)) + "." +
				std::to_string(0xFFu & (ip >> 24)) + ":" +
				std::to_string(port);
		} break;
		default: return "";
	}
}

// -- socket -------------------------------------------------------

bool socket::open(domain d, socktype t, protocol p) noexcept {
	close();
	m_handle = ::socket((int)d, (int)t, (int)p);
	return m_handle >= 0;
}
void socket::close() noexcept {
	if(m_handle >= 0) {
		#ifdef STX_WINDOWS_SOCKET
			::closesocket(m_handle);
		#else
			::close(m_handle);
		#endif
		m_handle = -1;
	}
}

// Client
bool socket::connect(address const& addr) noexcept {
	int err_code = ::connect(m_handle, addr, addr.length);
	return err_code >= 0;
}

// Server
bool socket::bind(address const& addr) noexcept {
	int err_code = ::bind(m_handle, addr, addr.length);
	return err_code >= 0;
}
bool socket::listen(unsigned queueLength) noexcept {
	int err_code = ::listen(m_handle, queueLength);
	return err_code >= 0;
}
socket socket::accept(address* addr) noexcept {
	socket result;
	result.m_handle = ::accept(
		m_handle,
		addr ? *addr : (sockaddr*)nullptr,
		addr ? &addr->length : nullptr
	);
	return result;
}

// TCP-ish
int socket::send(const void* data, size_t len, int flags) noexcept {
	return ::send(m_handle, data, len, flags);
}
int socket::recv(void* data,       size_t len, int flags) noexcept {
	return ::recv(m_handle, data, len, flags);
}

// UDP-ish
int socket::recvfrom(void*       buf, size_t len, address* from, int flags) noexcept {
	return ::recvfrom(
		m_handle,
		(char*)buf, len,
		flags,
		from ? *from : (sockaddr*)nullptr,
		from ? &from->length : nullptr
	);
}
int socket::sendto  (void const* buf, size_t len, address const& to,   int flags) noexcept {
	return ::sendto(m_handle, buf, len, flags, to, to.length);
}

// Options
bool socket::option(sockopt_level level, sockopt opt, bool value) noexcept {
	int yes = 1;
	int no  = 0;
	return option(level, opt, value ? &yes : &no, sizeof(yes));
}
bool socket::option(sockopt_level level, sockopt opt, void const* value, size_t size) noexcept {
	int err_code;
	switch (opt) {
		case sockopt::non_blocking:{
			#if defined(STX_WINDOWS_SOCKETS)
				// Not tested, on a linux machine right now
				err_code = ioctlsocket(m_handle, FIONBIO, value);
			#elif defined(STX_LINUX_SOCKETS)
				int flags = fcntl(m_handle, F_GETFL, 0);
				if (flags < 0) {
					err_code = flags;
					break;
				}
				flags = *((int*) value) ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
				err_code = fcntl(m_handle, F_SETFL, flags);
			#else
				#error "I don't know how to set the socket to async for this platform"
			#endif
		} break;
		default: {
			err_code = ::setsockopt(m_handle, (int)level, (int) opt, value, size);
		} break;
	}
	return err_code >= 0;
}

socket::socket(socket&& other) : socket() {
	*this = std::move(other);
}
socket& socket::operator=(socket&& other) {
	close();
	m_handle = std::exchange(other.m_handle, -1);
	return *this;
}

} // namespace stx
