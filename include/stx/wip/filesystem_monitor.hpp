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

	template<typename C>
	void set_observer(int watch, C const& c) {
		m_event.subscribe([watch, c](int w, unsigned mask) {
			if(w == watch)
				c(mask);
		});
	}

	template<typename C>
	void set_observer(C&& c) {
		m_event.subscribe(c);
	}

	template<typename C>
	void set_observer(std::string const& path, C const& c) {
		set_observer(add(path, change), c);
	}

	template<typename C>
	void set_observer(std::string const& path, unsigned mask, C const& c) {
		set_observer(add(path, mask), c);
	}

	void poll_events();

	bool initialized() const noexcept;
	inline operator bool() const noexcept { return initialized(); }

	static const char* Stringify(mask m);
};

} // namespace stx
