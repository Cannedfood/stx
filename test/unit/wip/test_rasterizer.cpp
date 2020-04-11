#include "../catch.hpp"

#include <stx/bitmap.hpp>
#include <stx/wip/rasterizer.hpp>

TEST_CASE("rasterizeLine", "[rasterizer]") {
	stx::static_bitmap<bool, 4, 4> bmp;

	SECTION("tl -> br") {
		bmp.data_storage.fill(false);
		stx::rasterizeLine(
			0, 0,
			3, 3,
			[&](float x, float y) { bmp(x, y) = true; }
		);
		CHECK(bmp(0, 0));
		CHECK(bmp(1, 1));
		CHECK(bmp(2, 2));
		CHECK(bmp(3, 3));
	}
	SECTION("br -> tl") {
		bmp.data_storage.fill(false);
		stx::rasterizeLine(
			3, 3,
			0, 0,
			[&](float x, float y) { bmp(x, y) = true; }
		);
		CHECK(bmp(0, 0));
		CHECK(bmp(1, 1));
		CHECK(bmp(2, 2));
		CHECK(bmp(3, 3));
	}
	SECTION("tr -> bl") {
		bmp.data_storage.fill(false);
		stx::rasterizeLine(
			0, 3,
			3, 0,
			[&](float x, float y) { bmp(x, y) = true; }
		);
		CHECK(bmp(3, 0));
		CHECK(bmp(2, 1));
		CHECK(bmp(1, 2));
		CHECK(bmp(0, 3));
	}
	SECTION("bl -> tr") {
		bmp.data_storage.fill(false);
		stx::rasterizeLine(
			3, 0,
			0, 3,
			[&](float x, float y) { bmp(x, y) = true; }
		);
		CHECK(bmp(3, 0));
		CHECK(bmp(2, 1));
		CHECK(bmp(1, 2));
		CHECK(bmp(0, 3));
	}
}

// TODO
// TEST_CASE("rasterizeTriangle", "[rasterizer]") {
// 	stx::static_bitmap<bool, 4, 4> bmp;

// 	bmp.data_storage.fill(false);
// 	stx::rasterizeTriangle(1, 1, 1, 2, 2, 2, [&](int x, int y) { bmp(x, y) = true; });
// }
