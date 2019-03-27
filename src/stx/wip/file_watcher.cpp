#include "file_watcher.hpp"

#include "../platform.hpp"

#ifdef STX_OS_UNIX

#include <sys/inotify.h>

namespace stx {

int inotify_instance() {
	static int result = inotify_init();
	return result;
}

file_watcher::file_watcher() noexcept {
	m_handle = inotify_add_watch(inotify_instance(), "/", 0);
}
file_watcher::~file_watcher() noexcept {
	inotify_rm_watch(inotify_instance(), m_handle);
}

file_watcher_flags file_watcher::supported_flags() noexcept {
	return (file_watcher_flags) ~0;
}

} // namespace stx

#else
	#warning "stx::file_watcher not supported for this OS"
#endif
