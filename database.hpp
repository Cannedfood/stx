#pragma once

#include <memory>
#include <vector>

namespace stx {

class database;
class database_result;

namespace detail {

class database_impl {
protected:
	uint32_t mFlags;
public:
	virtual ~database_impl() {}

	virtual bool open(std::string const& path, uint32_t flags) = 0;
	virtual void close() = 0;
	virtual bool is_open() const = 0;

	virtual database_result query(std::string const& sql) = 0;

	uint32_t flags() const noexcept { return mFlags; }
};

class database_result_impl {
public:
	virtual ~database_result_impl() {}

	virtual size_t columns() = 0;

	virtual bool next() = 0;
};

} // namespace detail

enum db_flags {
	db_readonly           = 1,
	db_no_synchronization = 2, //<! Don't use synchronization when accessing the database (faster)
	db_dont_create        = 4, //<! Don't create a database if it doesn't exist
	db_exceptions         = 8 // Throw an exception if something goes wrong (e.g. in database::open)
	// TODO: shared cache?
};

// class database_binding {
// public:
// 	struct entry {
// 		size_t offset;
// 		size_t type;
// 		const char* name;
// 	};
//
// 	database_binding(std::vector<entry> entries) :
// 		m_entries(std::move(entries))
// 	{}
//
// private:
// 	std::vector<entry> m_entries;
// };

class database_result : private std::shared_ptr<detail::database_result_impl> {
public:
	database_result() {}

	database_result(shared_ptr<detail::database_result_impl>&& p) :
		shared_ptr<detail::database_result_impl>(std::move(p))
	{}

	bool next() { return get()->next(); }
};

class database : private std::unique_ptr<detail::database_impl> {
public:
	static database sqlite();

	constexpr
	database() {}

	explicit
	database(std::unique_ptr<detail::database_impl>&& impl) :
		std::unique_ptr<element_type>(std::move(impl))
	{}

	bool open(std::string const& path, uint32_t flags = 0) { return get()->open(path, flags); }
	void close() { get()->close(); }

	database_result query(std::string const& sql) { return get()->query(sql); }

	bool     is_open() const noexcept { return get() ? get()->is_open() : false; }
	uint32_t flags() const noexcept { return get() ? get()->flags() : 0; }

	operator bool() const noexcept { return is_open(); }
};

using db = database;
using db_result = database_result;

} // namespace stx
