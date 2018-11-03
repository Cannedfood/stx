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

constexpr inline size_t MaxNumComponents = 64;

// =============================================================
// == Component id =============================================
// =============================================================

namespace detail {

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

} // namespace detail

template<class T>
inline           unsigned component_id = detail::component_id_holder<T>::value;
constexpr inline unsigned invalid_component_id = ~unsigned(0);

// =====================================================================
// == Component allocation =============================================
// =====================================================================

class type_operations {
public:
	type_operations() {}

	template<class T>
	type_operations() :
		type_operations()
	{
		static_assert(std::is_move_constructible_v<T>, "T has to be move constructable");
		static_assert(std::is_move_assignable_v<T>,    "T has to be move assignable");
		m_size = sizeof(T);
		m_alignment = alignof(T);

		m_pfnDestroy = [](void* a, void* b) { *static_cast<T*>(a)->~T(); };

		m_pfnMoveAssign = [](void* a, void* b) { *static_cast<T*>(a) = std::move(*static_cast<T*>(b)); };
		m_pfnMoveConstruct = [](void* at, void* b) { new(at) T(std::move(*static_cast<T*>(b))); };

		if constexpr(std::is_copy_constructible_v<T>) {
			m_pfnCopyConstruct = [](void* a, void* b) { *static_cast<T*>(a) = *static_cast<T*>(b); };
		}

		if constexpr(std::is_default_constructible_v<T>) {
			m_pfnDefaultConstruct =  [](void* at) { new(at) T(); };
		}
	}

	size_t size() const noexcept { return m_size; }
	size_t alignment() const noexcept { return m_alignment; }

	void moveAssign      (void* from, void* to) { m_pfnMoveAssign(from, to); }
	void moveConstruct   (void* from, void* to) { m_pfnMoveConstruct(from, to); }
	void copyConstruct   (void* from, void* to) { m_pfnCopyConstruct(from, to); }
	void copyAssign      (void* from, void* to) { m_pfnCopyAssign(from, to); }
	void destroy         (void* at) { m_pfnDestroy(at); }
	void defaultConstruct(void* at) { m_pfnDefaultConstruct(at); }

private:
	size_t m_size;
	size_t m_alignment;

	void (*m_pfnMoveAssign)       (void* from, void* to) = nullptr;
	void (*m_pfnMoveConstruct)    (void* from, void* to) = nullptr;
	void (*m_pfnCopyAssign)       (void* from, void* to) = nullptr;
	void (*m_pfnCopyConstruct)    (void* from, void* to) = nullptr;
	void (*m_pfnDestroy)          (void* at) = nullptr;
	void (*m_pfnDefaultConstruct) (void* at) = nullptr;
};

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

namespace detail {

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

} // namespace detail

// =====================================================================
// == Component Allocation =============================================
// =====================================================================

class sparse_vector_interface {
public:
	virtual void* getTypeErased    (size_t index) = 0;
	virtual void  destroyTypeErased(size_t index) = 0;
	virtual ~sparse_vector_interface() {}
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
	size_t total_slots() {
		size_t result = 0;
		for(auto& b : m_blocks) {
			if(b) result += ElementsPerBlock;
		}
		return result;
	}
	size_t used_slots() {
		size_t result = 0;
		for(auto u : m_usages) result += u;
		return result;
	}
	size_t unused_slots() { return total_slots() - used_slots(); }

	size_t total_memory()  { return total_slots() * aligned_size(); }
	size_t used_memory()   { return used_slots() * aligned_size(); }
	size_t unused_memory() { return unused_slots() * aligned_size(); }

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

// ==============================================================
// == Entity System =============================================
// ==============================================================

using component_mask = std::bitset<MaxNumComponents>;

class entities {
public:
	entities();
	~entities();

	// Create/Destroy entities
	entity create();
	void   destroy(entity e);
	bool   valid(entity e);

	// Attach/Remove components
	void* getUnchecked(entity e, size_t component_id);
	void* get(entity e, size_t component_id);
	void  remove(entity e, size_t component_id);

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
	T* attach(entity e, T&& t) { return attachNew<T>(e, std::forward<T>(t)); }

	template<class T>
	T* get(entity e) {
		if(!m_component_masks[e.id()].test(component_id<T>)) return nullptr;
		return getUnchecked<T>(e);
	}
	template<class T>
	T* getUnchecked(entity e) {
		auto* storage = static_cast<sparse_vector<T>*>(
			m_component_storage[component_id<T>].get()
		);
		return storage->get(e.index());
	}

	// Filter entities by component
	entity first(component_mask m);
	entity next(entity e, component_mask m);
private:
	detail::id_manager          m_ids;
	std::array<std::unique_ptr<sparse_vector_interface>, MaxNumComponents> m_component_storage;
	std::vector<component_mask> m_component_masks;
};

// ====================================================================
// == Component Filtering =============================================
// ====================================================================

template<class... Components>
class filter {
	component_mask m_mask;
	entities&      m_entities;
public:
	filter(entities& entities) noexcept :
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
		constexpr iterator() noexcept : m_entities(nullptr), m_entity(), m_mask() {}

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

/*
 Copyright (c) 2018 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
