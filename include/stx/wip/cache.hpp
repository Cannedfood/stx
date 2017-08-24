#pragma once

#include "../hash.hpp"
#include "../memory.hpp"

#include <map>

namespace stx {

template<typename T, typename TLoadInfo = symstring>
class cache {
public:
	using LoadInfo = TLoadInfo;

	static_assert(std::is_copy_constructible<LoadInfo>::value,
	              "LoadInfo has to be copy constructable");
	static_assert(std::is_move_constructible<LoadInfo>::value,
	              "LoadInfo has to be move constructable");

private:
	std::map<LoadInfo, weak<T>> m_entries;

public:
	cache();
	~cache();

	shared<T> add_entry(LoadInfo const& i, owned<T>&& t);

	void      hot_swap(LoadInfo const& i, T&& t);

	shared<T> load(LoadInfo const& i);
	owned<T>  load_uncached(LoadInfo const& i);
};

} // namespace stx

#include "cache.inl"
