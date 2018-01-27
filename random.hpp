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
	random(Generator&& g) : mGenerator(g) {}

	template<class T> std::enable_if_t<std::is_integral_v<T>,
	T> get(T min, T max) { return std::uniform_int_distribution<T>(min, max)(mGenerator); }
	template<class T> std::enable_if_t<std::is_floating_point_v<T>,
	T> get(T min, T max) { return std::uniform_int_distribution<T>(min, max)(mGenerator); }

	template<class T>
	T get(T range) { return get<T>(0, range); }
	template<class T>
	T get() { return get<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max()); }
};

inline
random<std::random_device> default_random() {
	return random<std::random_device>();
}

template<class T>
T rand(T min, T max) { return default_random().get<T>(min, max); }
template<class T>
T rand(T range) { return default_random().get<T>(range); }
template<class T>
T rand() { return default_random().get<T>(); }

} // namespace stx

#endif // header guard STX_RANDOM_HPP_INCLUDED
