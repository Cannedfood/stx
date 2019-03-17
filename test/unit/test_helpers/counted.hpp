#pragma once

#include <atomic>

template<class T>
struct basic_counted {
	struct Decrementer { void operator()(int* c) const noexcept { --*c; } };

	std::unique_ptr<T, Decrementer> c;
	basic_counted(T& c) : c(&(++c)) {}
};

using counted = basic_counted<int>;
using atomic_counted = basic_counted<std::atomic<int>>;