#pragma once

#include <functional>

namespace stx {

enum socket_protocol { socket_protocol_invalid, tcp, udp };

enum socket_type {
	socket_type_invalid,
	ipv4,      //<! internet protocol (ip) version 4
	ipv6,      //<! internet protocol (ip) version 6
	irda,      //<! e.g. infrared sensors
	bluetooth, //<! bluetooth
	unix_local //<! unix domain sockets for interprocess communication
};

namespace detail {

	class socket_based {
	protected:
		int             m_socket;
		socket_type     m_type;
		socket_protocol m_protocol;

		bool open(socket_type type, socket_protocol proto);
		void close();

	public:
		socket_based();
		socket_based(socket_based const&) = delete;
		socket_based(socket_based&&);
		~socket_based() noexcept;

		socket_based& operator=(socket_based const&) = delete;
		socket_based& operator=(socket_based&&) noexcept;

		inline socket_type     type() const noexcept { return m_type; }
		inline socket_protocol protocol() const noexcept { return m_protocol; }
		inline int             native_socket_handle() const noexcept { return m_socket; }

		inline bool is_open() const noexcept { return !(m_socket < 0); }
		inline operator bool() const noexcept { return is_open(); }
	};

} // namespace detail

class server : public detail::socket_based {
public:
	server();
	~server() noexcept;

	bool open(socket_type type, socket_protocol proto) noexcept;
	void close() noexcept;
};

class client : public detail::socket_based {
public:
	client();
	~client() noexcept;

	void close() noexcept;
};

} // namespace stx
