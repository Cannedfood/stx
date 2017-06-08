#pragma once

#if __cplusplus < 201103L
#error This file requires compiler and library support \
for the ISO C++ 2011 standard. This support must be enabled \
with the -std=c++11 or -std=gnu++11 compiler options.
#endif

#include <stdexcept>

#include <cstdint>

#include <chrono>

namespace stx {

enum sockdomain : unsigned {
	sockdomain_invalid,
	ip4,
	ip6,
	irda,
	bluetooth,
	unix,
	_num_sockdomains
};

enum sockprotocol : unsigned {
	sockprotocol_invalid,
	udp, //<! User Datagram Protocol: Fire and forget packages, faster
	tcp, //<! Transmission Control Protocol: Packet stream, reliable
	_num_sockprotocols
};

class connection;
class address;

class socket {
	int  m_handle;
	int* m_refcount;

	socket(socket const&);
	socket& operator=(socket const&);

	bool         blocking() const noexcept;
	socket&      blocking(bool b);
public:
	struct error : public std::runtime_error {
		error(const std::string& s) : std::runtime_error(s) {}
	};

	struct invalid_operation : public error {
		invalid_operation(const std::string& s) : error(s) {}
	};

	struct failed_operation : public error {
		failed_operation(const std::string& s) : error(s) {}
	};

	socket();
	socket(socket&&);
	~socket();

	socket& operator=(socket&&);

	// == General ====================================================

	socket& open(sockdomain d, sockprotocol p);
	socket& open(sockdomain d, sockprotocol p, uint16_t port);
	socket& close();

	// == Server =====================================================

	/// Bind socket to a port (for servers, socket has to be )
	socket& bind(uint16_t port);

	/// Accept incoming connections (for servers, have to be bound to a port
	/// first @see bind)
	connection accept(
	    size_t                    max_queued = 1,
	    std::chrono::milliseconds timeout    = std::chrono::milliseconds(-1));

	/// Accept incoming connects (for servers)
	connection accept(
	    sockdomain                d,
	    sockprotocol              p,
	    uint16_t                  port,
	    size_t                    max_queued = 1,
	    std::chrono::milliseconds timeout    = std::chrono::milliseconds(-1));

	// == Client =====================================================

	// TODO: socketopt & recv and sendv options
	// TODO: connect

	// == Getters & Setters ==========================================

	sockprotocol protocol() const noexcept;
	sockdomain   domain() const noexcept;
	size_t       references() const noexcept;
	bool         listening() const noexcept;

	inline bool is_open() const { return m_handle >= 0; }
	inline operator bool() const { return is_open(); }
};

class connection {
	socket m_socket;
	int    m_handle;

	friend class socket;

public:
	connection();
	~connection();

	// == Movable =========================================================
	connection(connection&&);
	connection& operator=(connection&&);

	// == Not copiable ====================================================
	connection(connection const&) = delete;
	connection& operator=(connection const&) = delete;

	// == Methods =========================================================
	connection& close();

	void write_s(const char* s);

	// == Getters =========================================================
	socket const& socket() const noexcept { return m_socket; }
	inline operator bool() const noexcept { return m_handle >= 0; }
};

} // namespace stx
