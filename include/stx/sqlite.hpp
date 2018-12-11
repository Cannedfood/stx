#pragma once

#include <sqlite3.h>

#include <string>
#include <string_view>

namespace stx {

namespace sqlite {

class stmt {
	sqlite3_stmt* m_stmt;
public:
	constexpr inline
	stmt(sqlite3_stmt*&& stmt = nullptr) : m_stmt(stmt) {}

	inline
	~stmt() noexcept {
		if(m_stmt)
			sqlite3_finalize(m_stmt);
	}

	constexpr inline
	stmt(stmt&& other) noexcept :
		m_stmt(other.m_stmt)
	{
		other.m_stmt = nullptr;
	}
	constexpr inline
	stmt& operator=(stmt&& other) noexcept {
		m_stmt = other.m_stmt;
		other.m_stmt = nullptr;
	}

	void reset() {
		sqlite3_reset(m_stmt);
	}

	bool step() {
		return sqlite3_step(m_stmt) == SQLITE_ROW;
	}

	stmt(stmt const& other) noexcept            = delete;
	stmt& operator=(stmt const& other) noexcept = delete;
};

// TODO: check for errors
class database {
	sqlite3* m_handle;
public:
	database(std::string const& path, bool readonly, uint32_t options) {
		sqlite3_open_v2(path.c_str(), &m_handle, options | SQLITE_OPEN_URI, NULL);
	}

	~database() noexcept {
		sqlite3_close_v2(m_handle);
	}

	stmt query(std::string_view sql) {
		sqlite3_stmt* q;
		sqlite3_prepare_v2(m_handle, sql.data(), sql.size(), &q, NULL);
		return {std::move(q)};
	}
};

} // namespace sqlite

} // namespace stx