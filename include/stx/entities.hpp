// Copyright (c) 2018 Benno Straub, licensed under the MIT license. (A copy can be found at the bottom of this file)

// A entity component system

// TODO:
// - consolidate entities
// - sort entities by components (and maybe by supplied function?)

#pragma once

#include <memory>
#include <vector>
#include <bitset>
#include <cassert>

namespace stx {

// =============================================================
// == Settings =================================================
// =============================================================

namespace options {

constexpr inline size_t MaxNumComponents = 64;

} // namespace options


// =============================================================
// == Component id =============================================
// =============================================================

namespace detail::ecs {

size_t new_component_id() noexcept;

template<class T>
constexpr inline bool is_valid_component_v =
	!std::is_pointer_v<T> &&
	std::is_move_assignable_v<T> &&
	std::is_move_constructible_v<T>;

template<class T>
constexpr inline bool is_nice_component_v =
	is_valid_component_v<T> &&
	std::is_trivially_destructible_v<T>;

template<class T>
struct component_id_holder {
	static_assert(!std::is_pointer_v<T>, "Pointers are not valid components");
	static_assert(std::is_move_assignable_v<T>, "Components have to be move assignable");
	static_assert(std::is_move_constructible_v<T>, "Components have to be move constructable");
	static size_t value;
};
template<class T> size_t component_id_holder<T>::value = new_component_id();

} // namespace detail::ecs

template<class T>
inline           unsigned component_id         = detail::ecs::component_id_holder<T>::value;
constexpr inline unsigned invalid_component_id = ~unsigned(0);

// ================================================================
// == Entity/EntityID =============================================
// ================================================================

/// A handle to an entity
class entity {
	uint64_t m_id;

public:
	constexpr entity() : m_id(~0ull) {}
	constexpr entity(uint64_t id) : m_id(id) {}
	constexpr entity(uint32_t index, uint32_t version) :
		m_id(uint64_t(index) | uint64_t(version) << 32ull)
	{}

	constexpr entity(entity const&) = default;
	constexpr entity& operator=(entity const&) = default;

	constexpr uint64_t id() const { return m_id; }
	constexpr uint32_t index()   const { return m_id & 0xFFFFFFFF; }
	constexpr uint32_t version() const { return m_id >> 32; }

	constexpr operator bool() const noexcept { return *this != entity(); }

	constexpr bool operator==(entity const& other) const noexcept { return id() == other.id(); }
	constexpr bool operator!=(entity const& other) const noexcept { return id() != other.id(); }
	constexpr bool operator<=(entity const& other) const noexcept { return id() <= other.id(); }
	constexpr bool operator>=(entity const& other) const noexcept { return id() >= other.id(); }
	constexpr bool operator< (entity const& other) const noexcept { return id() <  other.id(); }
	constexpr bool operator> (entity const& other) const noexcept { return id() >  other.id(); }
};

namespace detail::ecs {

class id_manager {
public:
	entity create() noexcept;
	void   free(entity) noexcept;
	bool   valid(entity) const noexcept;

	entity entityAt(uint32_t index) const noexcept;
private:
	std::vector<uint32_t> m_versions;
	std::vector<uint32_t> m_free;
};

// =====================================================================
// == Component Allocation =============================================
// =====================================================================

class sparse_vector_interface {
public:
	virtual ~sparse_vector_interface() {}


	virtual void*        getTypeErased    (size_t index) = 0;
	virtual void         destroyTypeErased(size_t index) = 0;

	struct statistics_t {
		size_t total_slots  = 0;
		size_t used_slots   = 0;
		size_t total_memory = 0;
		size_t used_memory  = 0;
	};
	virtual statistics_t statistics() = 0;
};

template<class T, size_t ElementsPerBlock = 524288 / sizeof(T)>
class sparse_vector final : public sparse_vector_interface {
	constexpr size_t aligned_size() const noexcept {
		return sizeof(T); // TODO: make sure multiples of the size are still aligned
	}

	void* getTypeErased(size_t index)     override { return get(index); }
	void  destroyTypeErased(size_t index) override { return destroy(index); }
public:
	sparse_vector() {}
	~sparse_vector() {}

	T*   get  (size_t index) {
		size_t block   = index / ElementsPerBlock; // Optimizes to right shift (hopefully)
		size_t element = index % ElementsPerBlock; // Optimizes to bitwise-and
		return reinterpret_cast<T*>(m_blocks[block].get() + element * aligned_size());
	}

	template<class... Args>
	T*   create(size_t index, Args&&... args) {
		_increment_usage(index);
		return new(get(index)) T(std::forward<Args>(args)...);;
	}
	void destroy(size_t index) {
		get(index)->~T();
		_decrement_usage(index);
	}
	void moveToEmpty(size_t from, size_t to) {
		T* pTo   = get(to);
		T* pFrom = get(from);
		new(pTo) T(std::move(*pFrom));
		pFrom->~T();
	}
	void moveAndOverride(size_t from, size_t to) {
		destroy(to);
		moveToEmpty(from, to);
	}

	// Statistics
	statistics_t statistics() override {
		statistics_t result {};

		for(auto& b : m_blocks) {
			if(b) result.total_slots += ElementsPerBlock;
		}
		for(auto u : m_usages) result.used_slots += u;

		result.total_memory = result.total_slots * aligned_size();
		result.used_memory  = result.used_slots  * aligned_size();

		return result;
	}

private:
	uint8_t* _increment_usage(size_t index) {
		size_t block = index / ElementsPerBlock;

		if(m_usages.size() <= block) {
			m_usages.resize(block + 1);
			m_blocks.resize(block + 1);
		}

		uint32_t usages = ++m_usages[block];
		if(usages == 1) {
			m_blocks[block].reset(new uint8_t[ElementsPerBlock * aligned_size()]);
		}
		return m_blocks[block].get();
	}

	void _decrement_usage(size_t index) {
		size_t block = index / ElementsPerBlock;

		if(--m_usages[block] == 0) {
			m_blocks[block].reset();
		}
	}

	std::vector<std::unique_ptr<uint8_t[]>> m_blocks;
	std::vector<uint32_t>                   m_usages;
};

} // namespace detail::ecs

// ==============================================================
// == Entity System =============================================
// ==============================================================

using component_mask = std::bitset<options::MaxNumComponents>;

template<class... Components> class filter_t;

class entities {
	using sparse_vector_interface = detail::ecs::sparse_vector_interface;
	template<class T>
	using sparse_vector = detail::ecs::sparse_vector<T>;
	using id_manager    = detail::ecs::id_manager;
public:
	entities();
	~entities();

	// -- Create/Destroy entities ------------------------------------------
	entity create() noexcept;
	entity create(component_mask hint) noexcept;
	void   destroy(entity e);
	bool   valid(entity e) const noexcept;

	template<class... Components>
	entity create(Components&&... components) noexcept {
		entity e = create(make_mask<Components...>());
		(attach(e, std::forward<Components>(components)), ...);
		return e;
	}

	// -- Attach/Remove components (For C API) -----------------------------
	void* getUnchecked(entity e, size_t component_id);
	void* get(entity e, size_t component_id);
	void  remove(entity e, size_t component_id);

	// -- Attach/Remove components (Template based) ------------------------
	template<class T, class... Args>
	T* attachNew(entity e, Args&&... args) {
		assert(m_ids.valid(e));

		using storage_t = sparse_vector<std::remove_reference_t<T>>;

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
		return storage->create(e.index(), std::forward<Args>(args)...);
	}
	template<class T>
	std::remove_reference_t<T>* attach(entity e, T&& t) {
		return attachNew<std::remove_reference_t<T>>(e, std::forward<T>(t));
	}
	template<class T, class... Args>
	T* getOrAttach(entity e, Args&&... args) {
		if(!m_component_masks[e.index()].test(component_id<T>))
			return attachNew(e, std::forward<Args>(args)...);
		else
			return getUnchecked<T>(e);
	}
	template<class T>
	T* get(entity e) {
		if(!m_component_masks[e.index()].test(component_id<T>)) return nullptr;
		return getUnchecked<T>(e);
	}
	template<class T>
	T* getUnchecked(entity e) {
		auto* storage = static_cast<sparse_vector<T>*>(
			m_component_storage[component_id<T>].get()
		);
		return storage->get(e.index());
	}

	// -- Helpers -------------------------------------------------------
	template<class... Components>
	static component_mask make_mask() {
		using namespace std;

		component_mask mask;
		for(unsigned id : std::array{ component_id<remove_cv_t<remove_reference_t<Components>>>... }) { // I <3 C++17
			mask.set(id);
		}
		return mask;
	}

	// -- Filter entities by component -------------------------------------
	entity first(component_mask m);
	entity next(entity e, component_mask m);

	template<class... Types>
	filter_t<Types...> filter();

	using statistics_t = detail::ecs::sparse_vector_interface::statistics_t;
	statistics_t statistics() {
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
	id_manager          m_ids;
	std::array<std::unique_ptr<sparse_vector_interface>, options::MaxNumComponents> m_component_storage;
	std::vector<component_mask> m_component_masks;
};

// ====================================================================
// == Component Filtering =============================================
// ====================================================================

template<class... Components>
class filter_t {
	component_mask m_mask;
	entities&      m_entities;
public:
	filter_t(entities& entities) noexcept :
		m_entities(entities)
	{
		for(unsigned id : std::array{ component_id<Components>... }) { // I <3 C++17
			m_mask.set(id);
		}
	}

	struct iterator {
		mutable entities* m_entities;
		entity            m_entity;
		component_mask    m_mask;
	public:
		constexpr iterator() noexcept
			: m_entities(nullptr), m_entity(), m_mask()
		{}

		constexpr iterator(entities* ecs, entity e, component_mask mask) noexcept
			: m_entities(ecs), m_entity(e), m_mask(mask)
		{}

		iterator& operator++(void) noexcept {
			m_entity = m_entities->next(m_entity, m_mask);
			return *this;
		}

		std::tuple<Components&...> operator*() const noexcept {
			return { *m_entities->getUnchecked<Components>(m_entity)... };
		}

		constexpr bool operator==(iterator const& other) const noexcept {
			return m_entity == other.m_entity;
		}
		constexpr bool operator!=(iterator const& other) const noexcept {
			return m_entity != other.m_entity;
		}
	};

	iterator begin() const noexcept {
		return iterator(&m_entities, m_entities.first(m_mask), m_mask);
	}
	constexpr iterator end() const noexcept {
		return iterator();
	}
};

} // namespace stx

// =============================================================
// == Inline implementation =============================================
// =============================================================

namespace stx {

template<class... Types>
filter_t<Types...> entities::filter() {
	return {*this};
}

} // namespace stx

/*
 Copyright (c) 2018 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
