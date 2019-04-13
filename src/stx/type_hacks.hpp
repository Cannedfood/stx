#pragma once

#include <utility>
#include <typeinfo>
#include <cstddef>

namespace stx {

template <class, class = void>
struct is_defined : std::false_type
{};

template <class T>
struct is_defined<T,
    std::enable_if_t<std::is_object<T>::value &&
                    !std::is_pointer<T>::value &&
                    (sizeof(T) > 0)
        >
    > : std::true_type
{};

template<class T>
constexpr bool is_defined_v = is_defined<T>::value;

template<class T>
constexpr size_t try_sizeof() noexcept {
	constexpr bool has_size = is_defined_v<T> && !std::is_same_v<T, void>;
	if constexpr(has_size) return sizeof(T);
	return 1; // Everything has to be at least one byte
}

template<class T>
constexpr std::type_info const* try_typeid() noexcept {
	if constexpr(is_defined_v<T>) return &typeid(T);
	return nullptr;
}

} // namespace stx
