#include "catch.hpp"

#include "test_helpers/counted.hpp"

#include <stx/random.hpp>

#include <stx/unmaintained/gc.hpp>

#include <fstream>
#include <iostream>
#include <vector>
#include <map>

using namespace stx;

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
		// puts("========== Move");
		gc<counted> a = make_gc<counted>(count);
		gc<counted> b = std::move(a);

		CHECK(!a);
		CHECK(b);

		CHECK(count == 1);
		CHECK(garbage_collector::total_ref_count() == 1);
		CHECK(garbage_collector::refcount(b.get()) == 1);

		// garbage_collector::printDotGraph(std::cout);
	}
	garbage_collector::mark_and_sweep();
	REQUIRE(count == 0);

	// Copy
	{
		// puts("========== Copy");
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

	CHECK(count == (int)things.size());
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
	CHECK(count == (int)things.size());

	CHECK(count == (int)things.size());
	garbage_collector::mark_and_sweep();
	CHECK(count == (int)things.size());

	things.clear();
	garbage_collector::mark_and_sweep();
	CHECK(count == 0);
	CHECK(garbage_collector::total_obj_count() == 0);
	CHECK(garbage_collector::total_ref_count() == 0);
}

struct recursive_ref {
	std::vector<gc<recursive_ref>, gc_alloc<gc<recursive_ref>>> refs;
	recursive_ref() : refs(gc_alloc<gc<recursive_ref>>(this)) {}
};

TEST_CASE("gc_alloc works", "[gc]") {
	stx::random rnd;

	SECTION("With std::map") {
		garbage_collector::LEAK_ALL();

		auto owner = stx::make_gc<int>();

		using gc_map = std::map<int, int, std::less<>, gc_alloc<std::pair<const int, int>>>;

		gc_map map(gc_alloc<int>{owner.get()});
		for (size_t i = 0; i < 100; i++) {
			map.emplace(rnd.get<int>(), rnd.get<int>());
		}
		CHECK(garbage_collector::total_obj_count() > 0);
		CHECK(garbage_collector::total_ref_count() > 0);
		CHECK(garbage_collector::total_ref_count() == 1+garbage_collector::outrefcount(owner.get()));
	}

	SECTION("With circular references") {
		garbage_collector::LEAK_ALL();

		auto ref = stx::make_gc<recursive_ref>();
		ref->refs.push_back(ref);
		ref->refs.push_back(ref);
		garbage_collector::mark_and_sweep();
		CHECK(garbage_collector::total_obj_count() == 2);
		ref = nullptr;
		CHECK(garbage_collector::total_obj_count() == 0);

		garbage_collector::writeDotFile("test.gv");
	}

	CHECK(garbage_collector::total_ref_count() == 0);
	CHECK(garbage_collector::total_obj_count() >  0);
	garbage_collector::mark_and_sweep();
	CHECK(garbage_collector::total_ref_count() == 0);
	CHECK(garbage_collector::total_obj_count() == 0);
}
