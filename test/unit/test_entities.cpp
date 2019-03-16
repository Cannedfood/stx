#include "catch.hpp"

#include "test_helpers/counted.hpp"

#include <stx/entities.hpp>

#include <random>

std::vector<size_t> random_indices(size_t count, std::pair<size_t, size_t> minmax) {
	std::vector<size_t> result;

	size_t range = minmax.second - minmax.first;

	result.reserve(range);
	for(size_t i = minmax.first; i < minmax.second; i++) result.push_back(i);
	std::shuffle(result.begin(), result.end(), std::mt19937(rand()));
	result.resize(std::min(result.size(), count));
	result.shrink_to_fit();
	return result;
}

std::vector<size_t> random_indices(size_t count) {
	return random_indices(count, {0, count});
}

TEST_CASE("entity id works", "[entities]") { ///////////////////////////////////
	using namespace stx;

	SECTION("Entity is properly constructed from id and version") {
		CHECK(entity(0xFFFFFFFF, 0).index() == 0xFFFFFFFF);
		CHECK(entity(0, 0xFFFFFFFF).version() == 0xFFFFFFFF);
	}

	SECTION("Entity default-constructs to invalid") {
		CHECK(!entity());
	}

	SECTION("Entity behaves properly when copying them around") {
		entity prototype = entity(0xAAAA5555, 0xFFFFFFFF);
		std::vector<entity> entities(100, prototype);
		std::shuffle(entities.begin(), entities.end(), std::mt19937(rand()));
		for(auto e : entities)
			REQUIRE(e == prototype);
	}
}

TEST_CASE("id_manager works", "[entities]") { //////////////////////////////////
	using namespace stx;

	detail::ecs::id_manager manager;
	int count = 5;

	std::mt19937 rgen {(unsigned long)rand()};

	std::vector<entity> entities;

	for(size_t i = 0; i < 2; i++) {
		entities.resize(count + i*count);

		for(auto& e : entities) {
			e = manager.create();
			REQUIRE((bool)e);
			REQUIRE(manager.valid(e));
		}

		std::sort(entities.begin(), entities.end());
		REQUIRE(std::unique(entities.begin(), entities.end()) == entities.end());

		std::shuffle(entities.begin(), entities.end(), rgen);

		for(auto& e : entities) {
			manager.free(e);
			REQUIRE(!manager.valid(e));
		}
	}
}

TEST_CASE("Test sparse_memory", "[entities]") { ////////////////////////////////
	stx::detail::ecs::sparse_vector<counted, 16> vec;

	int count = 0;

	SECTION("sparse_vector .get and .create return the same pointer") {
		auto indices = random_indices(1000, {0, 128});

		for(auto idx : indices) {
			auto created = vec.create(idx, count);
			auto gotten  = vec.get(idx);
			REQUIRE(created == gotten);
		}

		for(auto idx : indices)
			vec.destroy(idx);
	}

	SECTION("sparse_vector doesn't leak") {
		auto indices = random_indices(1000, {0, 4096});

		for(auto idx : indices) vec.create(idx, count);

		REQUIRE(count == indices.size());
		REQUIRE(vec.statistics().used_slots == indices.size());
		std::shuffle(indices.begin(), indices.end(), std::mt19937(rand()));

		for(auto idx : indices) vec.destroy(idx);
		REQUIRE(count == 0);
		REQUIRE(vec.statistics().used_slots == 0);
		REQUIRE(vec.statistics().used_memory == 0);
	}
}

TEST_CASE("Test stx::entities", "[entities]") { ////////////////////////////////
	SECTION("Attaching and removing components works") {
		constexpr size_t N = 1000;

		int           count = 0;
		stx::entities entities;

		for(size_t i = 0; i < N; i++) {
			stx::entity e = entities.create();
			entities.attach(e, counted{count});
		}

		REQUIRE(count == N);

		for(auto e : entities.filter_id<counted>())
		{
			entities.remove<counted>(e);
		}

		REQUIRE(count == 0);
	}

	SECTION("Getting components works") {
		stx::entities entities;
		for(size_t i = 0; i < 1000; i++) {
			stx::entity e = entities.create();
			entities.attach(e, int());
			REQUIRE(entities.get<int>(e));
			REQUIRE(entities.get<float>(e) == nullptr);
		}
	}

	SECTION("doesn't leak memory after destruction") {
		int count = 0;
		{
			stx::entities entities;

			for(size_t i = 0; i < 1000; i++) {
				stx::entity e = entities.create();
				entities.attach(e, counted{count});
			}

			REQUIRE(count == 1000);
		}
		REQUIRE(count == 0);
	}

	SECTION("doesn't leak memory when overwriting components") {
		int count = 0;
		{
			stx::entities entities;

			stx::entity e = entities.create();
			for(size_t i = 0; i < 5; i++) {
				entities.attach(e, counted{count});
			}

			REQUIRE(count == 1);
		}
		REQUIRE(count == 0);
	}

	SECTION("Manual iteration works") {
		constexpr size_t Count = 100;

		stx::entities entities;
		for(size_t i = 0; i < Count; i++) {
			entities.attach(entities.create(), int());
		}
		stx::component_mask mask;
		mask.set(stx::component_id<int>);

		size_t count = 0;
		for(stx::entity e = entities.first(mask);
		    e != stx::entity();
		    e = entities.next(e, mask))
		{
			++count;
		}
		REQUIRE(count == Count);
	}
}

TEST_CASE("Test stx::filter(stx::entities)", "[entities]") { ///////////////////
	constexpr size_t Count = 1000;

	using Foo = float;
	using Bar = int;

	stx::entities entities;
	for(size_t i = 0; i < Count; i++) {
		stx::entity e = entities.create();
		entities.attach(e, Foo());
		entities.attach(e, Bar());
	}

	SECTION("Filtering by Foo works") {
		size_t count = 0;
		for(auto [foo] : entities.filter<Foo>()) {
			++count;
		}
		REQUIRE(count == Count);
	}

	SECTION("Filtering by Bar works") {
		size_t count = 0;
		for(auto [bar] : entities.filter<Foo>()) {
			++count;
		}
		REQUIRE(count == Count);
	}

	SECTION("Filtering by Foo and Bar works") {
		size_t count = 0;
		for(auto [foo, bar] : entities.filter<Foo, Bar>()) {
			++count;
		}
		REQUIRE(count == Count);
	}
}
