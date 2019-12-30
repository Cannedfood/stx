#pragma once

#include <functional>
#include <type_traits>
#include <typeindex>

#include "../shared.hpp"

namespace stx {

class injector2;

class injector2 {
public:
	template<class T>
	injector2& singleton() noexcept {
		factories[typeid(T)] = stx::make_shared<SingletonFactory<T>>();
		return *this;
	}

	template<class T>
	injector2& ephemeral() noexcept {
		factories[typeid(T)] = stx::make_shared<EphemeralFactory<T>>();
		return *this;
	}

	template<class T>
	stx::shared<T> get() noexcept {
		auto iter = factories.find(typeid(T));
		if(iter == factories.end()) return nullptr;
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
private:
	struct Factory {
		virtual stx::shared<void> get(injector2& context) noexcept = 0;
	};

	template<class T>
	struct SingletonFactory final : public Factory {
		stx::shared<void> instance;
		stx::shared<void> get(injector2& context) noexcept {
			if(!instance) {
				instance = context.invoke_constructor<T, stx::shared<T>>([]<class... Args>(Args... args) { return stx::make_shared<T>(std::forward<Args>(args)...); });
			}
			return instance;
		}
	};

	template<class T>
	struct EphemeralFactory final : public Factory {
		stx::shared<void> get(injector2& context) noexcept {
			return context.invoke_constructor<T, stx::shared<T>>([]<class... Args>(Args... args) { return stx::make_shared<T>(std::forward<Args>(args)...); });
		}
	};

	template<class DoNotBindTo = void>
	struct autobinder {
		injector2* m_injector;

		template<class T>
		using basic_type_of = std::remove_reference_t<std::remove_const_t<T>>;

		template<class T>
		const static bool can_bind_to = !std::is_same_v<basic_type_of<T>, DoNotBindTo> && !std::is_same_v<basic_type_of<T>, stx::shared<DoNotBindTo>>;

		template<class T, class = std::enable_if_t<can_bind_to<T>>>
		operator stx::shared<T>() { return m_injector->get<T>(); }

		template<typename T, typename = std::enable_if_t<can_bind_to<T>>>
		operator T&() { return *m_injector->get<T>(); }
	};

	template<class T, class ExpectedResult, class Callback>
	auto invoke_constructor(Callback&& construct) -> ExpectedResult {
		using binder = autobinder<T>;
		auto binding = binder{this};
		if constexpr(std::is_constructible_v<T, binder&, binder&, binder&, binder&, binder&, binder&>) { return construct(binding, binding, binding, binding, binding, binding); }
		else if constexpr(std::is_constructible_v<T, binder&, binder&, binder&, binder&, binder&>) {  return construct(binding, binding, binding, binding, binding); }
		else if constexpr(std::is_constructible_v<T, binder&, binder&, binder&, binder&>) {  return construct(binding, binding, binding, binding); }
		else if constexpr(std::is_constructible_v<T, binder&, binder&, binder&>) {  return construct(binding, binding, binding); }
		else if constexpr(std::is_constructible_v<T, binder&, binder&>) {  return construct(binding, binding); }
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
