#include "../database.hpp"

#include "sqlite3.h"

namespace stx {

class sqlite_database_result final : public detail::database_result_impl {
	sqlite3_stmt* m_stmt;
public:
	sqlite_database_result(sqlite3_stmt* stmt) :
		m_stmt(stmt)
	{}

	~sqlite_database_result() {
		if(m_stmt)
			sqlite3_finalize(m_stmt);
	}

	size_t columns() override {
		return sqlite3_column_count(m_stmt);
	}

	bool next() override {
		return sqlite3_step(m_stmt) == SQLITE_ROW;
	}
};

class sqlite_database final : public detail::database_impl {
	sqlite3* m_db = nullptr;

	inline
	bool check(int err) {
		if(err == SQLITE_OK) return true;

		if(mFlags & db_exceptions)
			throw std::runtime_error(std::string("SQLITE3: ") + sqlite3_errstr(err));

		return false;
	}
public:
	~sqlite_database() override {
		close();
	}

	bool open(std::string const& path, uint32_t flags) override {
		close();

		mFlags = flags;

		int openFlags;
		if(flags & db_readonly)
			openFlags = SQLITE_OPEN_READONLY;
		else if(flags & db_dont_create)
			openFlags = SQLITE_OPEN_READWRITE;
		else
			openFlags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

		if(flags & db_no_synchronization)
			openFlags |= SQLITE_OPEN_NOMUTEX;

		openFlags |= SQLITE_OPEN_URI;

		bool success = check(sqlite3_open_v2(
			path.c_str(),
			&m_db,
			openFlags,
			NULL
		));
		if(!success) return false;

		return is_open();
	}

	void close() override {
		if(m_db) {
			sqlite3_close(m_db);
		}
	}

	bool is_open() const override {
		return m_db;
	}

	database_result query(std::string const& sql) override {
		sqlite3_stmt* stmt;
		if(!check(
			sqlite3_prepare_v2(m_db, sql.c_str(), sql.size() + 1, &stmt, NULL)))
		{
			return database_result();
		}

		if(!check(sqlite3_step(stmt))) return database_result();

		return std::shared_ptr<detail::database_result_impl>(
			std::make_shared<sqlite_database_result>(stmt)
		);
	}
};

database database::sqlite() {
	return database(std::make_unique<sqlite_database>());
}

} // namespace stx
