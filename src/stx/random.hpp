#ifndef STX_RANDOM_HPP_INCLUDED
#define STX_RANDOM_HPP_INCLUDED

#pragma once

#include <random>
#include <limits>
#include <cstdint>


namespace stx {

template<class Generator = std::mt19937>
class random {
	Generator mGenerator;
public:
	random() noexcept {}
	random(uint64_t seed) noexcept {
		mGenerator.seed(seed);
	}
	random(Generator&& g) noexcept : mGenerator(g) {}

	void seed(uint64_t value = std::random_device()()) noexcept {
		mGenerator.seed(value);
	}

	template<class T>
	T get(T min, T max) noexcept {
		static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "Can only generate floating point numbers or integers");
		if constexpr(std::is_floating_point_v<T>) {
			return std::uniform_real_distribution<T>(min, max)(mGenerator);
		}
		if constexpr(std::is_integral_v<T>) {
			return std::uniform_int_distribution<T>(min, max)(mGenerator);
		}
	}

	template<class T>
	T get(T range) noexcept { return get<T>(T(0), range); }
	template<class T>
	T get() noexcept { return get<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max()); }
	template<class T>
	auto& get(T* array, size_t size) noexcept { return array[get<size_t>(size - 1)]; }

	Generator& generator() noexcept { return mGenerator; }
};


inline thread_local random<std::mt19937> default_random = { std::mt19937(std::random_device{}()) };

template<class T> inline
T rand(T min, T max) noexcept { return default_random.get<T>(min, max); }
template<class T> inline
T rand(T range) noexcept { return default_random.get<T>(range); }
template<class T> inline
T rand() noexcept { return default_random.get<T>(); }

} // namespace stx

#endif // header guard STX_RANDOM_HPP_INCLUDED
