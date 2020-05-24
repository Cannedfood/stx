#include "catch.hpp"

#define STX_RASTERIZER_BITMAP_SUPPORT
#include <stx/wip/rasterizer.hpp>
#include <stx/random.hpp>

bool isInsideOfTriangle(float ptx, float pty, float v1x, float v1y, float v2x, float v2y, float v3x, float v3y) {
	auto sign = [](float p1x, float p1y, float p2x, float p2y, float p3x, float p3y) {
		return (p1x - p3x) * (p2y - p3y) - (p2x - p3x) * (p1y - p3y);
	};

    float d1 = sign(ptx, pty, v1x, v1y, v2x, v1y);
    float d2 = sign(ptx, pty, v2x, v2y, v3x, v2y);
    float d3 = sign(ptx, pty, v3x, v3y, v1x, v3y);

    bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

TEST_CASE("Test rasterizer", "[rasterizer]") {
	// float x1 = GENERATE(random(0.f, 100.f)), y1 = GENERATE(random(0.f, 100.f));
	// float x2 = GENERATE(random(0.f, 100.f)), y2 = GENERATE(random(0.f, 100.f));
	// float x3 = GENERATE(random(0.f, 100.f)), y3 = GENERATE(random(0.f, 100.f));

	float x1 = stx::rand(0.f, 20.f), y1 = stx::rand(0.f, 20.f);
	float x2 = stx::rand(0.f, 20.f), y2 = stx::rand(0.f, 20.f);
	float x3 = stx::rand(0.f, 20.f), y3 = stx::rand(0.f, 20.f);

	SECTION("Interpolating x should be the same as x") {
		stx::rasterize_triangle(
			x1, y1, x1,
			x2, y2, x2,
			x3, y3, x3,
			[](int x, int y, float interpolatedX) {
				CHECK(x == Approx(interpolatedX));
			}
		);
	}

	SECTION("Interpolating y should be the same as y") {
		stx::rasterize_triangle(
			x1, y1, y1,
			x2, y2, y2,
			x3, y3, y3,
			[](int x, int y, float interpolatedY) {
				CHECK(y == Approx(interpolatedY));
			}
		);
	}

	// SECTION("Check that rasterization is correct") {
	// 	stx::heap_bitmap<bool> image(10, 10);
	// 	image.each([](bool& b) { b = false; });

	// 	float x1 = GENERATE(random(0.f, 20.f)), y1 = GENERATE(random(0.f, 20.f));
	// 	float x2 = GENERATE(random(0.f, 20.f)), y2 = GENERATE(random(0.f, 20.f));
	// 	float x3 = GENERATE(random(0.f, 20.f)), y3 = GENERATE(random(0.f, 20.f));

	// 	stx::rasterize_triangle(
	// 		x1, y1,
	// 		x2, y2,
	// 		x3, y3,
	// 		[&](int x, int y) {
	// 			image(x, y) = true;
	// 			CHECK(isInsideOfTriangle(x, y, x1, y1, x2, y2, x3, y3));
	// 		}
	// 	);

	// 	image.each([&](bool value, unsigned x, unsigned y) {
	// 		CHECK(value == isInsideOfTriangle(x, y, x1, y1, x2, y2, x3, y3));
	// 	});
	// }
}
