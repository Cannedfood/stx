#pragma once

#include "entities.hpp"

namespace stx {

// ====================================================================
// == Component Filtering =============================================
// ====================================================================

class filter_id_t {
	entities&      m_entities;
	component_mask m_mask;
public:
	filter_id_t(entities& entities, component_mask mask) :
		m_entities(entities),
		m_mask(mask)
	{}

	struct iterator {
	protected:
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

		stx::entity operator*() const noexcept {
			return m_entity;
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

template<class... Components>
class filter_t {
	entities&      m_entities;
	component_mask m_mask;
public:
	filter_t(entities& ecs) noexcept :
		m_entities(ecs),
		m_mask(make_component_mask<Components...>())
	{}

	struct iterator {
		mutable entities* m_entities;
		entity            m_entity;
		component_mask    m_mask;

		template<class T>
		using ResultT =
			std::conditional_t<
				std::is_same_v<T, stx::entity>, stx::entity, std::conditional_t<
				std::is_pointer_v<T>, T,
				T&
			>>;

		template<class T>
		auto getValue(stx::entity e) const noexcept -> ResultT<T> {
			if constexpr(std::is_same_v<T, stx::entity>) { return e; }
			else if constexpr(std::is_pointer_v<T>) { return m_entities->get<std::remove_pointer_t<T>>(e); }
			else return m_entities->getUnchecked<T>(e);
		}
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

		std::tuple<ResultT<Components>...> operator*() const noexcept {
			return { getValue<Components>(m_entity)... };
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
filter_t<Types...> entities::filter() noexcept {
	return {*this};
}

template<class... Types>
filter_id_t entities::filter_id() noexcept {
	return {*this, make_component_mask<Types...>()};
}

} // namespace stx
