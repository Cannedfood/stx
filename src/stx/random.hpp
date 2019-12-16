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
	random() {}
	random(uint64_t seed) {
		mGenerator.seed(seed);
	}
	random(Generator&& g) : mGenerator(g) {}

	void seed(uint64_t value = std::random_device()()) {
		mGenerator.seed(value);
	}

	template<class T>
	T get(T min, T max) {
		static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "Can only generate floating point numbers or integers");
		if constexpr(std::is_floating_point_v<T>) {
			return std::uniform_real_distribution<T>(min, max)(mGenerator);
		}
		if constexpr(std::is_integral_v<T>) {
			return std::uniform_int_distribution<T>(min, max)(mGenerator);
		}
	}

	template<class T>
	T get(T range) { return get<T>(T(0), range); }
	template<class T>
	T get() { return get<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max()); }
	template<class T>
	auto& get(T* array, size_t size) { return array[get<size_t>(size - 1)]; }

	Generator& generator() noexcept { return mGenerator; }
};


extern thread_local random<std::mt19937> default_random;

template<class T>
T rand(T min, T max) { return default_random.get<T>(min, max); }
template<class T>
T rand(T range) { return default_random.get<T>(range); }
template<class T>
T rand() { return default_random.get<T>(); }

} // namespace stx

#endif // header guard STX_RANDOM_HPP_INCLUDED
