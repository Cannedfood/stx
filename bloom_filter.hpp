#pragma once

#include <bitset>
#include <array>

namespace stx {

template<size_t Nbits = 64, bool SizeOptimized = true>
// You can test if things were added to this container. There are false positivies but no false negatives.
// https://en.wikipedia.org/wiki/Bloom_filter
class bloom_filter {
	using filter_t = std::conditional_t<SizeOptimized, std::bitset<Nbits>, std::array<bool, Nbits>>;

	filter_t m_filter;
public:
	constexpr
	bloom_filter() noexcept {}

	constexpr
	bloom_filter(std::initializer_list<size_t> populate) noexcept {
		for(size_t v : populate) add(v);
	}

	template<class T> constexpr
	void  add(std::initializer_list<T> v) noexcept { for(auto vv : v) add(vv); }
	template<class T> constexpr std::enable_if_t<!std::is_same_v<T, size_t>,
	void> add(T const& t) noexcept { m_filter[std::hash<T>()(t) % Nbits] = true; }
	constexpr
	void  add(size_t n) noexcept { m_filter[n % Nbits] = true; }

	constexpr void clear() noexcept;

	template<class T> constexpr std::enable_if_t<!std::is_same_v<T, size_t>,
	bool>           test(T const& t) const noexcept { return m_filter[std::hash<T>()(t) % Nbits]; }
	constexpr bool  test(size_t n)   const noexcept { return m_filter[n % Nbits]; }

	constexpr size_t count() const noexcept;
	constexpr double error() const noexcept;
};

template<size_t Nbits, bool SizeOptimized> constexpr
size_t bloom_filter<Nbits, SizeOptimized>::count() const noexcept {
	if constexpr(SizeOptimized) {
		return m_filter.count(); // std::bitset::count
	}
	else {
		size_t result = 0;
		for(auto b : m_filter) {
			if(b) result++;
		}
		return result;
	}
}
template<size_t Nbits, bool SizeOptimized> constexpr
double bloom_filter<Nbits, SizeOptimized>::error() const noexcept { return count() / (double) Nbits; }

template<size_t Nbits, bool SizeOptimized> constexpr
void bloom_filter<Nbits, SizeOptimized>::clear() noexcept {
	if constexpr(SizeOptimized) {
		m_filter.reset();
	}
	else {
		for(auto& b : m_filter) b = false;
	}
}

} // namespace stx
