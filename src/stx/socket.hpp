#pragma once

#include <memory>
#include <cstdint>

extern "C" {
	#include <memory.h> // memcmp
}

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

// #ifdef unix
// 	#undef unix // WTF?
// #endif

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

/// A address of any socket address family (sa_family), mostly for reading incoming addresses
struct address : public sockaddr_storage {
	socklen_t length;

	stx::domain domain() const noexcept { return stx::domain(ss_family); }

	operator sockaddr_in&       ()       noexcept { return reinterpret_cast<sockaddr_in&> ((sockaddr_storage&)*this); }
	operator sockaddr_in  const&() const noexcept { return reinterpret_cast<sockaddr_in const&> ((sockaddr_storage const&)*this); }
	operator sockaddr_in6&      ()       noexcept { return reinterpret_cast<sockaddr_in6&>((sockaddr_storage&)*this); }
	operator sockaddr_in6 const&() const noexcept { return reinterpret_cast<sockaddr_in6 const&>((sockaddr_storage const&)*this); }

	operator sockaddr const*() const noexcept { return reinterpret_cast<sockaddr const*>((sockaddr_storage const*)this); }
	operator sockaddr*()       const noexcept { return reinterpret_cast<sockaddr*>((sockaddr_storage*)this); }

	bool operator< (address const& other) const noexcept { return memcmp(this, &other, sizeof(address)) < 0; }
	bool operator> (address const& other) const noexcept { return memcmp(this, &other, sizeof(address)) > 0; }
	bool operator>=(address const& other) const noexcept { return memcmp(this, &other, sizeof(address)) >= 0; }
	bool operator<=(address const& other) const noexcept { return memcmp(this, &other, sizeof(address)) <= 0; }
	bool operator==(address const& other) const noexcept { return memcmp(this, &other, sizeof(address)) == 0; }
	bool operator!=(address const& other) const noexcept { return memcmp(this, &other, sizeof(address)) != 0; }

};

[[nodiscard]] address ipv4(address const& addr) noexcept;
[[nodiscard]] address ipv4(uint32_t ip, uint16_t port) noexcept;
[[nodiscard]] address ipv4(uint8_t ip_1, uint8_t ip_2, uint8_t ip_3, uint8_t ip_4, uint16_t port) noexcept;
[[nodiscard]] address ipv4(uint16_t port) noexcept;

[[nodiscard]] std::string to_string(address const& addr) noexcept;

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
	[[nodiscard]] socket accept(address* addr = nullptr) noexcept;

	// TCP-ish
	int  send(const void* data, size_t len, int flags = 0) noexcept;
	int  send(std::string_view s, int flags = 0) noexcept { return send(s.data(), s.length(), flags); }
	int  recv(void* data,       size_t len, int flags = 0) noexcept; // TODO: span
	// UDP-ish
	int sendto  (void const* buf, size_t len, address const& to,       int flags = 0) noexcept;
	int sendto  (std::string_view s,          address const& to,       int flags = 0) noexcept { return sendto(s.data(), s.length(), to, flags); }
	int recvfrom(void*       buf, size_t len, address* from = nullptr, int flags = 0) noexcept;


	bool option(sockopt_level level, sockopt opt, bool value) noexcept;
	bool option(sockopt_level level, sockopt opt, void const* value, size_t size) noexcept;

	bool option(sockopt opt, bool value)                     noexcept { return option(sockopt_level::socket, opt, value); }
	bool option(sockopt opt, void const* value, size_t size) noexcept { return option(sockopt_level::socket, opt, value, size); }

	[[nodiscard]] int handle() noexcept { return m_handle; }

	operator bool() const noexcept { return m_handle >= 0; }

	socket(socket&& other);
	socket& operator=(socket&& other);

	socket(socket const&) = delete;
	socket& operator=(socket const&) = delete;
private:
	int m_handle;
};

} // namespace stx
