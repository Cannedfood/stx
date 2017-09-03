#pragma once

#include <string>
#include <thread>

#include "../event.hpp"

namespace stx {

class filesystem_monitor {
public:
	enum mode {
		async,
		sync,
		poll
	};

	enum mask {
		any      = 0,

		added    = 1<<0,
		removed  = 1<<1,
		moved    = 1<<2,

		opened   = 1<<3,
		accessed = 1<<4,
		modified = 1<<5,

		meta_changed = 1 << 6,

		other    = 1 << 7
	};

	enum composite_mask {
		folder_change = added | removed | moved,
		data_change   = added | removed | modified,
		change        = folder_change | data_change
	};

private:
	mode         m_mode;
	int          m_handle;
	std::thread* m_thread;

	event<void(int handle, unsigned mask)> m_event;
	// std::map<std::string, int> m_watches;

public:
	// -- Construct --------------------------------------------------------
	filesystem_monitor(mode m = poll);
	~filesystem_monitor() noexcept;

	// -- Copy --------------------------------------------------------------
	filesystem_monitor(filesystem_monitor const&) = delete;
	filesystem_monitor& operator=(filesystem_monitor const&) = delete;

	// -- Move --------------------------------------------------------------
	filesystem_monitor(filesystem_monitor&& other);
	filesystem_monitor& operator=(filesystem_monitor&& other);

	// -- Functionality -----------------------------------------------------
	bool reset();
	void destroy() noexcept;

	int  add(std::string const& path, unsigned mask);
	void remove(std::string const& path);
	void remove(int handle);

	template<typename Fn>
	void set_observer(int watch, Fn const& callback);

	template<typename Fn>
	void set_observer(Fn&& callback);

	template<typename Fn>
	void set_observer(std::string const& path, Fn const& callback);

	template<typename Fn>
	void set_observer(std::string const& path, unsigned mask, Fn const& callback);

	void poll_events();

	bool initialized() const noexcept;
	inline operator bool() const noexcept { return initialized(); }

	static const char* to_string(mask m);
};

template<typename Fn>
void filesystem_monitor::set_observer(int watch, Fn const& c) {
	m_event.subscribe([watch, c](int w, unsigned mask) {
		if(w == watch)
			c(mask);
	});
}

template<typename Fn>
void filesystem_monitor::set_observer(Fn&& callback) {
	m_event.subscribe(callback);
}

template<typename Fn>
void filesystem_monitor::set_observer(std::string const& path, Fn const& callback) {
	set_observer(add(path, change), callback);
}

template<typename Fn>
void filesystem_monitor::set_observer(std::string const& path, unsigned mask, Fn const& callback) {
	set_observer(add(path, mask), callback);
}

} // namespace stx
