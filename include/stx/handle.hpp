#pragma once

namespace stx {

struct handle_socket {
	virtual void on_handle_destroyed() noexcept = 0;
};

class handle {
	handle_socket* m_socket;

public:
	handle() : m_socket(nullptr) {}
	handle(handle_socket* s) : m_socket(s) {}
	handle(handle&& h) : m_socket(h.m_socket) { h.m_socket = nullptr; }
	~handle() {
		if(m_socket) { m_socket->on_handle_destroyed(); }
	}

	handle& reset(handle_socket* new_sock = nullptr) {
		if(m_socket) { m_socket->on_handle_destroyed(); }
		m_socket = new_sock;
		return *this;
	}

	handle& operator=(handle_socket* s) { return reset(s); }

	handle& operator=(handle&& s) {
		reset(s.m_socket);
		s.m_socket = nullptr;
		return *this;
	}
};

class handles {
	std::vector<handle> m_handles;

public:
	handles& operator<<(handle&& h) {
		m_handles.emplace_back(std::move(h));
		return *this;
	}
};

} // namespace stx
