#pragma once

#include "../hash.hpp"
#include "../memory.hpp"

#include <unordered_map>
#include <functional>

namespace stx {

template<typename T, typename LoadInfo = symstring, typename Hash = std::hash<LoadInfo>, typename Pred = std::equal_to<LoadInfo>>
class cache {
public:
	using load_info = LoadInfo;
	using hash      = Hash;
	using pred      = Pred;

	static_assert(std::is_copy_constructible<LoadInfo>::value,
	              "LoadInfo has to be copy constructable");
	static_assert(std::is_move_constructible<LoadInfo>::value,
	              "LoadInfo has to be move constructable");

private:
	std::unordered_map<LoadInfo, weak<T>, hash, pred> m_entries;

public:
	cache();
	~cache();

	shared<T> add_entry(LoadInfo const& i, owned<T>&& t);

	void      hot_swap(LoadInfo const& i, T&& t);

	shared<T> load(LoadInfo const& i);
	owned<T>  load_uncached(LoadInfo const& i);

	void      clear();
};

} // namespace stx

#include "cache.inl"
