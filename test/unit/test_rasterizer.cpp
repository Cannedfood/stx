#include "catch.hpp"

#define STX_RASTERIZER_BITMAP_SUPPORT
#include <stx/wip/rasterizer.hpp>
#include <stx/random.hpp>

#include <cmath>

struct Triangle {
    Triangle(float x1, float y1, float x2, float y2, float x3, float y3) {
        this->x3 = x3;
        this->y3 = y3;
        y23 = y2 - y3;
        x32 = x3 - x2;
        y31 = y3 - y1;
        x13 = x1 - x3;
        det = y23 * x13 - x32 * y31;
        minD = std::min(det, 0.f);
        maxD = std::max(det, 0.f);
    }

    bool contains(float x, float y) {
        float dx = x - x3;
        float dy = y - y3;
        float a = y23 * dx + x32 * dy;
        if (a < minD || a > maxD)
            return false;
        float b = y31 * dx + x13 * dy;
        if (b < minD || b > maxD)
            return false;
        float c = det - a - b;
        if (c < minD || c > maxD)
            return false;
        return true;
    }

    float x3, y3;
    float y23, x32, y31, x13;
    float det, minD, maxD;
};

TEST_CASE("Test rasterizer", "[rasterizer]") {
	// TODO: switch to Catch2 GENERATE (currently crashes)
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

	SECTION("Check that rasterization is correct") {
		stx::heap_bitmap<bool> image(32, 32);
		image.each([](bool& b) { b = false; });

		// float x1 = GENERATE(random(0.f, 20.f)), y1 = GENERATE(random(0.f, 20.f));
		// float x2 = GENERATE(random(0.f, 20.f)), y2 = GENERATE(random(0.f, 20.f));
		// float x3 = GENERATE(random(0.f, 20.f)), y3 = GENERATE(random(0.f, 20.f));

		Triangle triangle(x1, y1, x2, y2, x3, y3);

		stx::rasterize_triangle(
			x1, y1,
			x2, y2,
			x3, y3,
			[&](int x, int y) { image(x, y) = true; }
		);


		image.each([&](bool value, unsigned x, unsigned y) {
			CHECK(value == triangle.contains(x, y));
		});

		// puts("------------------");
		// for(size_t y = 0; y < image.h; y++) {
		// 	for(size_t x = 0; x < image.w; x++) {
		// 		printf(image(x, y)?"X":" ");
		// 	}
		// 	printf("\n");
		// }

		// Triangle triangle(x1, y1, x2, y2, x3, y3);
		// puts("------------------");
		// for(size_t y = 0; y < image.h; y++) {
		// 	for(size_t x = 0; x < image.w; x++) {
		// 		printf(triangle.contains(x, y)?"X":" ");
		// 	}
		// 	printf("\n");
		// }
		// puts("------------------");
		// for(size_t y = 0; y < image.h; y++) {
		// 	for(size_t x = 0; x < image.w; x++) {
		// 		printf(triangle.contains(x, y) != image(x, y)?"X":" ");
		// 	}
		// 	printf("\n");
		// }
		// puts("------------------");
	}
}
