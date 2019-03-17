#include "../socket.hpp"

namespace stx {

// -- ipv4 -------------------------------------------------------

ipv4::ipv4() noexcept :
	m_addr {}
{}

ipv4::ipv4(uint32_t ip, uint16_t port) noexcept :
	m_addr {
		.sin_family = (sa_family_t)domain::ipv4,
		.sin_port = htons(port),
		.sin_addr.s_addr = htonl(ip)
	}
{}

ipv4::ipv4(uint16_t port) noexcept :
	ipv4(INADDR_ANY, port)
{}

ipv4::ipv4(uint8_t ip_1, uint8_t ip_2, uint8_t ip_3, uint8_t ip_4, uint16_t port) noexcept :
	ipv4(uint32_t(ip_1) << 24 | uint32_t(ip_2) << 16 | uint32_t(ip_3) << 8 | uint32_t(ip_4) << 0, port)
{}

sockaddr const* ipv4::get()    const noexcept { return reinterpret_cast<sockaddr const*>(&m_addr); }
int             ipv4::length() const noexcept { return sizeof(m_addr); }


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
	}
}

// Client
bool socket::connect(address const& addr) noexcept {
	int err_code = ::connect(m_handle, addr.get(), addr.length());
	return err_code >= 0;
}

// Server
bool socket::bind(address const& addr) noexcept {
	int err_code = ::bind(m_handle, addr.get(), addr.length());
	return err_code >= 0;
}
bool socket::listen(unsigned queueLength) noexcept {
	int err_code = ::listen(m_handle, queueLength);
	return err_code >= 0;
}
socket socket::accept(any_address* addr) noexcept {
	socket result;
	result.m_handle = ::accept(
		m_handle,
		addr ? addr->get() : nullptr,
		addr ? addr->plength() : nullptr
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
int socket::recvfrom(void*       buf, size_t len, any_address* from, int flags) noexcept {
	return ::recvfrom(
		m_handle,
		(char*)buf, len,
		flags,
		from ? from->get() : nullptr,
		from ? from->plength() : nullptr
	);
}
int socket::sendto  (void const* buf, size_t len, address const& to,   int flags) noexcept {
	return ::sendto(m_handle, buf, len, flags, to.get(), to.length());
}

// Options
bool socket::option(sockopt_level level, sockopt opt, bool value) noexcept {
	int yes = 1;
	int no  = 0;
	return option(level, opt, value ? &yes : &no, sizeof(yes));
}
bool socket::option(sockopt_level level, sockopt opt, void const* value, size_t size) noexcept {
	int err_code = ::setsockopt(m_handle, (int)level, (int) opt, value, size);
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