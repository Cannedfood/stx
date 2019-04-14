#include "catch.hpp"

#include "test_helpers/counted.hpp"

#include <stx/gc.hpp>
#include <stx/random.hpp>

#include <fstream>
#include <iostream>

using stx::gc;
using stx::make_gc;
using stx::garbage_collector;

TEST_CASE("Single reference doesn't leak", "[gc]") {
	int count = 0;

	{
		gc<counted> a = make_gc<counted>(count);
		garbage_collector::mark_and_sweep();
		REQUIRE(count == 1);
		REQUIRE(garbage_collector::refcount(a.get()) == 1);
	}

	garbage_collector::mark_and_sweep();
	REQUIRE(count == 0);
}

TEST_CASE("Multiple references don't leak", "[gc]") {
	int count = 0;

	garbage_collector::LEAK_ALL();

	gc<counted> a = make_gc<counted>(count);
	gc<counted> b = a;

	REQUIRE(garbage_collector::refcount(a.get()) == 2);

	garbage_collector::mark_and_sweep();
	REQUIRE(count == 1);
	a.reset();

	garbage_collector::mark_and_sweep();
	REQUIRE(count == 1);
	REQUIRE(garbage_collector::refcount(b.get()) == 1);

	b.reset();
	garbage_collector::mark_and_sweep();
	REQUIRE(count == 0);
	REQUIRE(garbage_collector::total_ref_count() == 0);
	REQUIRE(garbage_collector::total_obj_count() == 0);
}

TEST_CASE("Doesn't leak with multiple objects", "[gc]") {
	int count = 0;

	garbage_collector::LEAK_ALL();

	{
		gc<counted> a = make_gc<counted>(count), b = make_gc<counted>(count), c = make_gc<counted>(count), d = make_gc<counted>(count);
		REQUIRE(count == 4);
		REQUIRE(garbage_collector::total_ref_count() == 4);
		REQUIRE(garbage_collector::total_obj_count() == 4);
	}
	garbage_collector::mark_and_sweep();
	REQUIRE(count == 0);
	REQUIRE(garbage_collector::total_ref_count() == 0);
	REQUIRE(garbage_collector::total_obj_count() == 0);
}

TEST_CASE("Indirect references don't leak", "[gc]") {
	int count = 0;

	garbage_collector::LEAK_ALL();

	auto a = make_gc<counted>(count);

	struct thing { gc<counted> value; };
	auto b = make_gc<thing>(thing{a});

	REQUIRE(garbage_collector::outrefcount(b.get()) == 1);

	a.reset();

	garbage_collector::mark_and_sweep();
	REQUIRE(count == 1);

	b.reset();
	garbage_collector::mark_and_sweep();
	REQUIRE(count == 0);
}

TEST_CASE("Circular references don't leak", "[gc]") {
	int count = 0;

	{
		struct thing {
			gc<thing> other;
			counted count;
		};

		gc<thing> a = make_gc<thing>(thing { .count = counted(count) });
		gc<thing> b = make_gc<thing>(thing { .count = counted(count) });
		REQUIRE(count == 2);

		garbage_collector::mark_and_sweep();
		REQUIRE(count == 2);

		a->other = b;
		b->other = a;

		garbage_collector::mark_and_sweep();
		REQUIRE(count == 2);
	}

	garbage_collector::mark_and_sweep();
	REQUIRE(count == 0);
}

TEST_CASE("Assignment tests", "[gc]") {
	int count = 0;

	garbage_collector::LEAK_ALL();

	// Move
	{
		puts("========== Move");
		gc<counted> a = make_gc<counted>(count);
		gc<counted> b = std::move(a);

		CHECK(!a);
		CHECK(b);

		CHECK(count == 1);
		CHECK(garbage_collector::total_ref_count() == 1);
		CHECK(garbage_collector::refcount(b.get()) == 1);

		garbage_collector::printDotGraph(std::cout);
	}
	garbage_collector::mark_and_sweep();
	REQUIRE(count == 0);

	// Copy
	{
		puts("========== Copy");
		gc<counted> a = make_gc<counted>(count);
		gc<counted> b = a;

		CHECK(a);
		CHECK(b);

		CHECK(count == 1);
		CHECK(garbage_collector::total_ref_count() == 2);
	}
	garbage_collector::mark_and_sweep();
	REQUIRE(count == 0);

	// Swap
	{
		// puts("========== Swap");

		gc<counted> a = make_gc<counted>(count);
		gc<counted> b = make_gc<counted>(count);

		CHECK(count == 2);
		CHECK(garbage_collector::total_ref_count() == 2);

		std::swap(a, b);
		CHECK(count == 2);
		CHECK(garbage_collector::total_ref_count() == 2);

		a.reset();
		b.reset();
		garbage_collector::mark_and_sweep();
		CHECK(count == 0);
		CHECK(garbage_collector::total_ref_count() == 0);
	}
	garbage_collector::mark_and_sweep();
	REQUIRE(count == 0);
}

TEST_CASE("GC stress test", "[gc]") {
	// puts("========== Stress test");
	struct thing {
		gc<thing> other;
		counted count;
	};

	constexpr size_t num_things = 30;

	int count = 0;

	garbage_collector::LEAK_ALL();

	std::vector<gc<thing>> things(num_things);
	for(auto& e : things) {
		e = make_gc<thing>(thing { .count = { count } });
	}

	CHECK(count == things.size());
	CHECK(garbage_collector::total_obj_count() == things.size());
	CHECK(garbage_collector::total_ref_count() == things.size());

	// Generate random object graph
	stx::random rnd;
	rnd.seed();
	for(auto& e : things) {
		e->other = rnd.get(things.data(), things.size());
	}
	std::shuffle(things.begin(), things.end(), rnd.generator());

	// Print for debugging
	// {
	// 	std::ofstream out("gc-graph.dot");
	// 	garbage_collector::printDotGraph(out);
	// }

	garbage_collector::mark_and_sweep();
	CHECK(garbage_collector::total_obj_count() == things.size());
	CHECK(garbage_collector::total_ref_count() == things.size() * 2);
	CHECK(count == things.size());

	CHECK(count == things.size());
	garbage_collector::mark_and_sweep();
	CHECK(count == things.size());

	things.clear();
	garbage_collector::mark_and_sweep();
	CHECK(count == 0);
	CHECK(garbage_collector::total_obj_count() == 0);
	CHECK(garbage_collector::total_ref_count() == 0);
}
