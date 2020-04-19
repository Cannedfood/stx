#pragma once

#include "shared.hpp"

#include <functional>
#include <type_traits>
#include <typeindex>
#include <vector>
#include <unordered_map>
#include <string>

namespace stx {

class injector {
public:
	injector(injector* fallback = nullptr) noexcept : fallback(fallback) {}
	injector(injector& fallback) noexcept : injector(&fallback) {}

	// Create once, destroy after injector is destroyed
	template<class T, class Implementation = T>
	injector& singleton() noexcept { factories[typeid(T)] = stx::make_shared<SingletonFactory<T, Implementation>>(); return *this; }
	// Do not recreate until all references are gone
	template<class T, class Implementation = T>
	injector& cached()    noexcept { factories[typeid(T)] = stx::make_shared<CachedFactory<T, Implementation>>(); return *this; }
	// Recreate every time
	template<class T, class Implementation = T>
	injector& ephemeral() noexcept {
		factories[typeid(T)] = stx::make_shared<EphemeralFactory<T, Implementation>>();
		return *this;
	}

	template<class T, class Implementation = T>
	stx::shared<T> create_singleton() {
		auto& factory = factories[typeid(T)];
		if(!factory) factory = stx::make_shared<SingletonFactory<T, Implementation>>();
		return factory->get(*this).cast_static<T>();
	}

	template<class T>
	stx::shared<T> get() noexcept {
		auto iter = factories.find(typeid(T));
		if(iter == factories.end()) {
			if(!fallback) return nullptr;
			return fallback->get<T>();
		}
		return iter->second->get(*this).cast_static<T>();
	}

	template<class T>
	stx::shared<T> require() {
		auto result = get<T>();
		if(!result) {
			throw std::runtime_error(std::string("Could not find or construct instance of type") + typeid(T).name());
		}
		return result;
	}

	template<class... Tn> // Call operator injects all arguments
	void operator()(Tn&... t) {
		std::tie(t...) =
			std::tuple<std::remove_reference_t<Tn>...>(
				(std::is_same_v<Tn, Tn> ? *this : *this)... // Hack to repeat *this for Tn...
			);
	}

	template<class T>
	operator stx::shared<T>() noexcept { return get<T>(); }
private:
	injector* fallback = nullptr;

	struct Factory {
		virtual stx::shared<void> get(injector& context) noexcept = 0;
	};

	template<class T>
	class dependency_shared_block final : public shared_block {
		using Tptr = std::remove_all_extents_t<T>*;
	
		alignas(T) unsigned char m_data[sizeof(T)];
	public:
		std::vector<stx::shared<void>> dependencies;

		template<class... Args>
		dependency_shared_block(Args&&... args) noexcept {
			T* tmp = new(m_data) T(std::forward<Args>(args)...);
			if(!((unsigned char*)tmp == m_data)) std::terminate();
			detail::handle_enable_shared_from_this<T, Tptr>(tmp, this);
		}
	
		T* value() { return (T*) m_data; }

		void shared_block_destroy() noexcept override {
			value()->~T();
			dependencies.clear();
		}
		void shared_block_free() noexcept override { delete this; }
	};

	template<class T, class Implementation>
	struct SingletonFactory final : public Factory {
		stx::shared<void> instance;
		stx::shared<void> get(injector& context) noexcept {
			if(!instance) {
				instance = context.create<Implementation>().template cast_dynamic<T>();
			}
			return instance;
		}
	};

	template<class T, class Implementation>
	struct CachedFactory final : public Factory {
		stx::weak<void> instance;
		stx::shared<void> get(injector& context) noexcept {
			auto result = instance.lock();
			if(!result) {
				instance = result = context.create<Implementation>().template cast_dynamic<T>();
			}
			return result;
		}
	};

	template<class T, class Implementation>
	struct EphemeralFactory final : public Factory {
		stx::shared<void> get(injector& context) noexcept {
			return context.create<Implementation>().template cast_dynamic<T>();
		}
	};

	template<class DoNotBindTo = void>
	struct autobinder {
		injector* m_injector;

		std::vector<stx::shared<void>> dependencies = {};

		template<class T>
		using basic_type_of = std::remove_reference_t<std::remove_const_t<T>>;

		template<class T>
		const static bool can_bind_to = !std::is_same_v<basic_type_of<T>, DoNotBindTo> && !std::is_same_v<basic_type_of<T>, stx::shared<DoNotBindTo>>;

		template<class T, class = std::enable_if_t<can_bind_to<T>>>
		operator stx::shared<T>() {
			auto result = m_injector->require<T>();
			dependencies.push_back(result);
			return result;
		}

		template<typename T, typename = std::enable_if_t<can_bind_to<T>>>
		operator T&() {
			if constexpr(std::is_same_v<T, stx::injector>) {
				return *m_injector;
			}
			else {
				auto result = m_injector->require<T>();
				dependencies.push_back(result);
				return *result;
			}
		}

		template<typename T, typename = std::enable_if_t<can_bind_to<T>>>
		operator T*() {
			if constexpr(std::is_same_v<T, stx::injector>) {
				return m_injector;
			}
			else {
				auto result = m_injector->get<T>();
				if(result)
					dependencies.push_back(result);
				return result.get();
			}
		}

		autobinder(injector* injector) noexcept : m_injector(injector) {}
		autobinder(autobinder&& other) noexcept                 = delete;
		autobinder& operator=(autobinder&& other) noexcept      = delete;
		autobinder(autobinder const& other) noexcept            = delete;
		autobinder& operator=(autobinder const& other) noexcept = delete;
	};

	template<class T>
	stx::shared<T> create() {
		using binder = autobinder<T>;
		binder binding{this};

		auto construct = [&]<class... Args>(Args&&... args) {
			stx::shared<T> result;
			auto* block = new dependency_shared_block<T>(std::forward<Args>(args)...);
			block->dependencies = std::move(binding.dependencies);
			result._move_reset(block->value(), block);
			return result;
		};

		if constexpr(std::is_constructible_v<T, binder&, binder&, binder&, binder&, binder&, binder&>) { return construct(binding, binding, binding, binding, binding, binding); }
		else if constexpr(std::is_constructible_v<T, binder&, binder&, binder&, binder&, binder&>) { return construct(binding, binding, binding, binding, binding); }
		else if constexpr(std::is_constructible_v<T, binder&, binder&, binder&, binder&>) { return construct(binding, binding, binding, binding); }
		else if constexpr(std::is_constructible_v<T, binder&, binder&, binder&>) { return construct(binding, binding, binding); }
		else if constexpr(std::is_constructible_v<T, binder&, binder&>) { return construct(binding, binding); }
		else if constexpr(std::is_constructible_v<T, binder&>) { return construct(binding); }
		else if constexpr(std::is_default_constructible_v<T>) { return construct(); }
		else {
			printf("No viable constructor for %s\n", typeid(T).name());
			std::terminate();
		}
	}

	std::unordered_map<std::type_index, stx::shared<Factory>> factories;
};

} // namespace stx
