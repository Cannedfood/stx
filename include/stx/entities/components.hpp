#pragma once

#include "entity_options.hpp"

#include <bitset>

namespace stx {

namespace ecs {

size_t new_component_id(std::type_info const& t) noexcept;

template<class T>
struct component_id_holder {
	static_assert(!std::is_pointer_v<T>, "Pointers are not valid components");
	static_assert(std::is_move_assignable_v<T>, "Components have to be move assignable");
	static_assert(std::is_move_constructible_v<T>, "Components have to be move constructable");
	static const size_t value;
};
template<class T>
const size_t component_id_holder<T>::value = new_component_id(typeid(T));

} // namespace ecs

using component_mask = std::bitset<options::MaxNumComponents>;

template<class T>
inline           unsigned component_id         = ecs::component_id_holder<T>::value;
constexpr inline unsigned invalid_component_id = ~unsigned(0);

template<class... Components>
component_mask make_component_mask() noexcept {
	using namespace std;

	component_mask mask;
	if constexpr(sizeof...(Components) > 0) {
		for(unsigned id : { component_id<remove_cv_t<remove_reference_t<Components>>>... }) {
			mask.set(id);
		}
	}
	return mask;
}

} // namespace stx