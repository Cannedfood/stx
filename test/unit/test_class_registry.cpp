#include "catch.hpp"

#include <stx/unmaintained/class_registry.hpp>

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

TEST_CASE("Test class registry", "[class_registry]") {
	CHECK(count(stx::classes::entries()) == 3); // class_registry_testA, class_registry_testB, class_registry_testC

	CHECK(count(stx::classes::get<class_registry_testA>().implementations())   == 1);
	CHECK(count(stx::classes::get<class_registry_testB>().implementations())   == 1);
	CHECK(count(stx::classes::get<class_registry_testC>().implementations()) == 0);
	CHECK(count(stx::classes::get<class_registry_testC>().implements()) == 2); // class_registry_testA, class_registry_testB
}
