#pragma once

#include <memory>
#include <cstdint>

#if ( defined(WIN32) || defined(_WIN32) || defined(__WIN32) ) && !defined(__CYGWIN__)
	#define STX_WINDOWS_SOCKETS
	extern "C" {
		#include <winsock.h>
	}
#else
	#define STX_LINUX_SOCKETS
	extern "C" {
		#include <unistd.h> // close()
		#include <sys/types.h>
		#include <sys/socket.h>
		#include <netinet/in.h> // sockaddr_in
		#include <netdb.h> // getservbyname
		#include <fcntl.h> // fcntl
	}
#endif

#ifdef unix
	#undef unix // WTF?
#endif

namespace stx {

enum class domain {
	// unix      = AF_UNIX,
	ipv4      = AF_INET,
	ipv6      = AF_INET6,
	// irda      = AF_IRDA,
	// bluetooth = AF_BLUETOOTH
};

enum class socktype {
	tcp = SOCK_STREAM,
	udp = SOCK_DGRAM,
};

enum class protocol {
	none = 0,
};

enum class sockopt_level {
	socket = SOL_SOCKET
};

enum class sockopt {
	reuse_address = SO_REUSEADDR,
	reuse_port    = SO_REUSEPORT,

	non_blocking = -2062144233
};

/// An interface for an address, you probably want to use the stx::ipv4 or stx::ipv6 address types
class address {
public:
	virtual sockaddr const* get()    const noexcept = 0;
	virtual int             length() const noexcept = 0;
};

/// Ipv4 address helper
class ipv4 : public address {
public:
	ipv4() noexcept {}
	explicit ipv4(address const& addr) noexcept;
	ipv4(uint32_t ip, uint16_t port) noexcept;
	ipv4(uint8_t ip_1, uint8_t ip_2, uint8_t ip_3, uint8_t ip_4, uint16_t port) noexcept;
	ipv4(uint16_t port) noexcept;


	uint32_t ip()   const noexcept { return ntohl(m_addr.sin_addr.s_addr); }
	uint16_t port() const noexcept { return ntohs(m_addr.sin_port); }

	std::string to_string() const noexcept;

	sockaddr const* get()    const noexcept override;
	int             length() const noexcept override;
private:
	sockaddr_in m_addr;
};

/// A address of any socket address family (sa_family), mostly for reading incoming addresses
struct any_address : public address {
public:
	any_address() noexcept {}
	any_address(address const& other) noexcept;

	domain          family() const noexcept { return static_cast<domain>(m_addr.ss_family); }
	sockaddr const* get()    const noexcept override;
	int             length() const noexcept override;

	ipv4 to_ipv4() const noexcept { return ipv4(*this); }

	sockaddr*  get()     noexcept { return reinterpret_cast<sockaddr*>(&m_addr); }
	socklen_t* plength() noexcept { return &m_size; }
private:
	socklen_t        m_size;
	sockaddr_storage m_addr;
};

class socket {
public:
	socket() noexcept : m_handle(-1) {}
	~socket() noexcept { close(); }

	[[nodiscard]] bool open(domain d, socktype t, protocol p = protocol::none) noexcept;
	              void close() noexcept;

	// Client
	[[nodiscard]] bool connect(address const& addr) noexcept;
	// Server
	[[nodiscard]] bool   bind(address const& addr) noexcept;
	[[nodiscard]] bool   listen(unsigned queueLength = 50) noexcept;
	[[nodiscard]] socket accept(any_address* addr = nullptr) noexcept;

	// TCP-ish
	int  send(const void* data, size_t len, int flags = 0) noexcept;
	int  send(std::string_view s, int flags = 0) noexcept { return send(s.data(), s.length(), flags); }
	int  recv(void* data,       size_t len, int flags = 0) noexcept; // TODO: span
	// UDP-ish
	int sendto  (void const* buf, size_t len, address const& to,             int flags = 0) noexcept;
	int sendto  (std::string_view s,          address const& to,             int flags = 0) noexcept { return sendto(s.data(), s.length(), to, flags); }
	int recvfrom(void*       buf, size_t len, any_address*   from = nullptr, int flags = 0) noexcept;


	bool option(sockopt_level level, sockopt opt, bool value) noexcept;
	bool option(sockopt_level level, sockopt opt, void const* value, size_t size) noexcept;

	bool option(sockopt opt, bool value)                     noexcept { return option(sockopt_level::socket, opt, value); }
	bool option(sockopt opt, void const* value, size_t size) noexcept { return option(sockopt_level::socket, opt, value, size); }

	int handle() noexcept { return m_handle; }


	operator bool() const noexcept { return m_handle >= 0; }

	socket(socket&& other);
	socket& operator=(socket&& other);

	socket(socket const&) = delete;
	socket& operator=(socket const&) = delete;
private:
	int m_handle;
};

} // namespace stx