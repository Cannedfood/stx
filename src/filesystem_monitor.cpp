#include "../include/stx/wip/filesystem_monitor.hpp"

#include "../include/stx/platform.hpp"

#ifdef STX_OS_LINUX // TODO: does this work on mac? On unix?

extern "C" {
#	include <sys/inotify.h>
#	include <unistd.h>
}

#include <stdexcept>
#include <cstring>
#include <cerrno>

#define CONVERT_FLAG(VFROM, VTO, FROM, TO) if((VFROM) & (FROM)) { (VTO) |= (TO); }

namespace stx {

filesystem_monitor::filesystem_monitor(mode m) :
	m_mode(m),
	m_handle(-1)
{}

filesystem_monitor::~filesystem_monitor() noexcept {
	destroy();
}

bool filesystem_monitor::reset() {
	destroy();

	m_handle = inotify_init1(m_mode == poll ? IN_NONBLOCK : 0);

	if(!initialized())
		return false;

	return true;
}

void filesystem_monitor::destroy() noexcept {
	if(initialized()) {
		close(m_handle);
		m_handle = -1;
	}
}

bool filesystem_monitor::initialized() const noexcept {
	return m_handle >= 0;
}

int filesystem_monitor::add   (std::string const& path, unsigned mask) {
	if(!initialized()) {
		if(!reset()) throw std::runtime_error("Failed creating inotify instance");
	}
	uint32_t real_mask = 0;

	if(mask == 0) {
		mask = ~0u;
	}

	CONVERT_FLAG(mask, real_mask, added,   IN_CREATE   | IN_MOVED_FROM);
	CONVERT_FLAG(mask, real_mask, removed, IN_DELETE   | IN_DELETE_SELF);
	CONVERT_FLAG(mask, real_mask, moved,   IN_MOVED_TO | IN_MOVE_SELF);

	CONVERT_FLAG(mask, real_mask, opened, IN_OPEN);
	CONVERT_FLAG(mask, real_mask, accessed, IN_ACCESS);
	CONVERT_FLAG(mask, real_mask, modified, IN_MODIFY);

	CONVERT_FLAG(mask, real_mask, meta_changed, IN_ATTRIB);

	// CONVERT_FLAG(mask, real_mask, other, TO)

	int watch_descriptor = inotify_add_watch(m_handle, path.c_str(), real_mask);

	if(watch_descriptor < 0) {
		throw std::runtime_error(std::string("Failed creating watch: ") + strerror(errno));
	}

	return watch_descriptor;
}

void filesystem_monitor::remove(int watch) {
	inotify_rm_watch(m_handle, watch);
}

void filesystem_monitor::remove(std::string const& path) {
	inotify_rm_watch(m_handle, inotify_add_watch(m_handle, path.c_str(), 0 | IN_MASK_ADD));
}

void filesystem_monitor::poll_events() {
	if(!initialized()) return;
	if(m_mode == async) throw std::runtime_error("Cannot poll events on a async file watcher");

	inotify_event e;
	int           result;
	while(m_handle >= 0) {
		result = ::read(m_handle, &e, sizeof(e));
		if(result < 0) break;

		unsigned emask = 0;

		CONVERT_FLAG(e.mask, emask, IN_CREATE,     added);
		CONVERT_FLAG(e.mask, emask, IN_MOVED_FROM, added);

		CONVERT_FLAG(e.mask, emask, IN_DELETE,      removed);
		CONVERT_FLAG(e.mask, emask, IN_DELETE_SELF, removed);

		CONVERT_FLAG(e.mask, emask, IN_MOVED_TO,  moved);
		CONVERT_FLAG(e.mask, emask, IN_MOVE_SELF, moved);

		CONVERT_FLAG(e.mask, emask, IN_OPEN,   opened);
		CONVERT_FLAG(e.mask, emask, IN_ACCESS, accessed);
		CONVERT_FLAG(e.mask, emask, IN_MODIFY, modified);

		CONVERT_FLAG(e.mask, emask, IN_ATTRIB, meta_changed);

		if(emask == 0) continue; // TODO: write out everything that'd go here

		m_event.trigger(e.wd, emask);
	}
}

const char* filesystem_monitor::Stringify(mask m) {
	switch (m) {
		case any: return "any";

		case added: return "added";
		case removed: return "removed";
		case moved: return "moved";

		case opened: return "opened";
		case accessed: return "accessed";
		case modified: return "modified";

		case meta_changed: return "meta_changed";

		case other: return "other";

		default: return "unknown";
	}
}

} // namespace stx

#elif defined(STX_OS_WINDOWS)
#	error Windows version of filesystem_monitor not implemented
#elif
#	error Filesystem_monitor not implemented for this OS
#endif
