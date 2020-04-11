#pragma once

#include "entity_options.hpp"

#include "entity.hpp"

#include <type_traits>
#include <bitset>

namespace stx {

namespace ecs {

int new_component_id(std::type_info const& t) noexcept;

template<class T>
struct component_id_holder {
	static_assert(!std::is_pointer_v<T>, "Pointers are not valid components");
	static_assert(std::is_move_assignable_v<T>, "Components have to be move assignable");
	static_assert(std::is_move_constructible_v<T>, "Components have to be move constructable");
	static const int value;
};
template<class T>
const int component_id_holder<T>::value = new_component_id(typeid(T));

template<class T>
struct component_id_holder<T*> { constexpr static int value = -1; };
template<>
struct component_id_holder<stx::entity> { constexpr static int value = -1; };

} // namespace ecs

using component_mask = std::bitset<options::MaxNumComponents>;

template<class T>
inline           int component_id         = ecs::component_id_holder<std::remove_cvref_t<T>>::value;
constexpr inline int invalid_component_id = -1;

template<class... Components>
component_mask make_component_mask() noexcept {
	using namespace std;

	component_mask mask;
	if constexpr(sizeof...(Components) > 0) {
		for(int id : { component_id<remove_cv_t<remove_reference_t<Components>>>... }) {
			if(id != invalid_component_id) {
				mask.set(id);
			}
		}
	}
	return mask;
}

} // namespace stx
