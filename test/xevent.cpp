#define STX_WIP

#include <xwip/event>

#include "test.hpp"

using namespace stx;


void test_xevent() {
	//return; // TODO

	int called = 0;

	stx::event<void()> e;

	// Test if one is calle
	e.add_observer([&called]() {
		called++;
	});

	test(called == 0);
	e.trigger();
	test(called == 1);


	// Test if multiples are called
	called = 0;

	e.add_observer([&called]() {
		called += 2;
	});
	e.add_observer([&called]() {
		called += 7;
	});

	e.trigger();
	test(called == 10);
}
