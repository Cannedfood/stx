#include "test.hpp"

#include "../class_registry.hpp"

#include <algorithm>

struct class_registry_testA {};
struct class_registry_testB {};
struct class_registry_testC : class_registry_testA, class_registry_testB {};
template class stx::classes::register_implementation<class_registry_testA, class_registry_testC>;
template class stx::classes::register_implementation<class_registry_testB, class_registry_testC>;

template<class Container>
size_t count(Container&& c) {
	return std::distance(c.begin(), c.end());
}

void test_class_registry() {
	test_eq(
		count(stx::classes::entries()),
		3 // class_registry_testA, class_registry_testB, class_registry_testC
	);

	test_eq(
		count(stx::classes::get<class_registry_testA>().implementations()),
		1
	);
	test_eq(
		count(stx::classes::get<class_registry_testB>().implementations()),
		1
	);
	test_eq(
		count(stx::classes::get<class_registry_testC>().implementations()),
		0
	);
	test_eq(
		count(stx::classes::get<class_registry_testC>().implements()),
		2 // class_registry_testA, class_registry_testB
	);
}
