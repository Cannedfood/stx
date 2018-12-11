#include "catch.hpp"

#include <stx/bloom_filter.hpp>

TEST_CASE("Test bloom filter (size optimized)", "[bloom_filter]") {
	constexpr bool SizeOptimized = true;

	stx::bloom_filter<64, SizeOptimized> filter;

	for(size_t i = 0; i < 128; i++) {
		filter.clear();
		filter.add(i);
		CHECK(filter.test(i));
	}

	filter.clear();
	filter.add<std::string>({"Hey", "Dude"});
	CHECK(filter.count() == 2); // May spuriously fail, when "Hey" and "Dude" have the same hash
	CHECK(filter.error() == 2.0 / 64.0);
	CHECK(filter.test<std::string>("Hey"));
	CHECK(filter.test<std::string>("Dude"));

	filter.clear();
}
