#pragma once

namespace stx {

enum class file_watcher_flags {
	ACCESS        = 1 << 0,
	ATTRIB        = 1 << 1,
	CLOSE_NOWRITE = 1 << 2,
	CLOSE_WRITE   = 1 << 3,
	CREATE        = 1 << 4,
	DELETE        = 1 << 5,
	DELETE_SELF   = 1 << 6,
	IGNORED       = 1 << 7,
	ISDIR         = 1 << 8,
	MODIFY        = 1 << 9,
	MOVE_SELF     = 1 << 10,
	MOVED_FROM    = 1 << 11,
	MOVED_TO      = 1 << 12,
	OPEN          = 1 << 13,
	Q_OVERFLOW    = 1 << 14,
	UNMOUNT       = 1 << 15
};

class file_watcher {
	int m_handle;
public:
	file_watcher() noexcept;
	~file_watcher() noexcept;

	static file_watcher_flags supported_flags() noexcept;
};

} // namespace stx
