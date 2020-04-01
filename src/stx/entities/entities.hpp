#pragma once

#include "entity.hpp"
#include "entity_ids.hpp"
#include "component_storage.hpp"
#include "components.hpp"
#include "entity_options.hpp"

#include <cassert>
#include <array>

namespace stx {

template<class... Components>
class filter_t;
class filter_id_t;

class entities {
public:
	entities() noexcept;
	~entities() noexcept;

	// -- Create/Destroy entities ------------------------------------------
	entity create() noexcept;
	entity create(component_mask hint) noexcept;
	bool   destroy(entity e) noexcept;
	bool   valid(entity e) const noexcept { return m_ids.valid(e); }

	template<class... Components>
	entity create(Components&&... components) noexcept {
		entity e = create(make_component_mask<Components...>());
		(attach(e, std::forward<Components>(components)), ...);
		return e;
	}

	// -- Attach/Remove components (For C API) -----------------------------
	void*          getUnchecked(entity e, size_t component_id) noexcept;
	void*          get(entity e, size_t component_id) noexcept;
	void           remove(entity e, size_t component_id) noexcept;
	component_mask mask(entity e) const noexcept {
		return valid(e) ? m_component_masks[e.index()] : component_mask{};
	}

	// -- Attach/Remove components (Template based) ------------------------
	template<class T, class... Args>
	T& attachNew(entity e, Args&&... args) {
		assert(m_ids.valid(e));

		using storage_t = ecs::component_storage<T>;

		size_t componentId = component_id<T>;
		auto* storage = static_cast<storage_t*>(m_component_storage[componentId].get());
		if(!storage) {
			auto storagePtr = std::make_unique<storage_t>();
			storage = storagePtr.get();
			m_component_storage[componentId] = std::move(storagePtr);
		}

		auto& componentMask = m_component_masks[e.index()];
		if(componentMask.test(componentId)) {
			storage->destroy(e.index());
		}
		componentMask.set(componentId);

		// std::cout << componentMask << " " << e.index()<<":"<<e.id() << " attach " << typeid(T).name() << std::endl;
		return *storage->create(e.index(), std::forward<Args>(args)...);
	}
	template<class T>
	auto attach(entity e, T&& t) -> std::remove_cv_t<std::remove_reference_t<T>>& {
		return attachNew<std::remove_cv_t<std::remove_reference_t<T>>>(e, std::forward<T>(t));
	}
	template<class T, class... Args>
	T& getOrAttach(entity e, Args&&... args) {
		if(!m_component_masks[e.index()].test(component_id<T>))
			return attachNew<T>(e, std::forward<Args>(args)...);
		else
			return getUnchecked<T>(e);
	}
	template<class T>
	T* get(entity e) noexcept {
		if(!m_component_masks[e.index()].test(component_id<T>)) return nullptr;
		return &getUnchecked<T>(e);
	}
	template<class T>
	T& getUnchecked(entity e) noexcept {
		auto* storage = static_cast<ecs::component_storage<T>*>(
			m_component_storage[component_id<T>].get()
		);
		return *storage->get(e.index());
	}
	template<class T>
	void remove(entity e) { remove(e, component_id<T>); }

	// -- Filter entities by component -------------------------------------
	entity first(component_mask m) noexcept;
	entity next(entity e, component_mask m) noexcept;

	template<class... Types>
	filter_t<Types...> filter() noexcept;
	template<class... Types>
	filter_id_t        filter_id() noexcept;

	using statistics_t = ecs::component_storage_interface::statistics_t;
	statistics_t statistics() noexcept {
		statistics_t stats;
		for(auto& v : m_component_storage) {
			if(v) {
				auto s = v->statistics();
				stats.total_memory += s.total_memory;
				stats.used_memory  += s.used_memory;
				stats.total_slots  += s.total_slots;
				stats.used_slots   += s.used_slots;
			}
		}
		return stats;
	}
private:
	ecs::entity_ids m_ids;
	std::array<std::unique_ptr<ecs::component_storage_interface>, options::MaxNumComponents> m_component_storage;
	std::vector<component_mask> m_component_masks;
};

} // namespace stx

#include "filter.hpp"
