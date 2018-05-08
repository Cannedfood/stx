#include "test.hpp"

#include "../class_registry.hpp"

struct class_registry_testA {};
struct class_registry_testB {};
struct class_registry_testC : class_registry_testA, class_registry_testB {};
template class stx::classes::register_implementation<class_registry_testA, class_registry_testC>;
template class stx::classes::register_implementation<class_registry_testB, class_registry_testC>;

void test_class_registry() {
	stx::classes::print(std::cout);
}
