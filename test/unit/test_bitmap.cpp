#include "catch.hpp"
#include <stx/bitmap.hpp>
#include <stx/random.hpp>

#include <array>

TEST_CASE("Test bitmap", "[bitmap]") {
	unsigned w = 5, h = 7;

	std::vector<int> data(w*h);

	std::generate(data.begin(), data.end(), []() { return stx::rand<int>(-1e7, -1); });

	stx::bitmap<int> a = { data.data(), w, h };

	SECTION("bitmap::each goes over each voxel") {
		a.each([](auto& i) { i = 42; });

		REQUIRE(
			std::all_of(data.begin(), data.end(), [](int i) { return
				i == 42;
			})
		);
	}

	SECTION("bitmap::subimage(w, h, d) works") {
		auto b = a.subimage(4, 4);
		b.each([](int& i) { i = 1337; });
			REQUIRE(16 == std::count(data.begin(), data.end(), 1337));
	}

	SECTION("bitmap::subimage(x,y,z,w,h,d) works") {
		auto b = a.subimage(w-4,h-4, 4,4); // 4x4x4 Corner cube

		b.each([](int& i) { i = 80085; });

		REQUIRE(16 == std::count(data.begin(), data.end(), 80085));

		bool no_80085_outside_of_cube = true;
		{
			auto also_not_80085 = [&](int& i) {
				no_80085_outside_of_cube = no_80085_outside_of_cube && (i != 80085);
			};

			a.subimage(w, h-4).each(also_not_80085);
			a.subimage(w-4, h).each(also_not_80085);
		}

		REQUIRE(no_80085_outside_of_cube);
	}

	SECTION("blit works") {
		std::vector<int> data2(data.size());
		stx::bitmap<int> b = { data2.data(), w, h };
		stx::blit(a, b);
		REQUIRE(data == data2);
	}

	SECTION("blit_backwards works like bitmap::blit") {
		std::vector<int> data2(data.size());
		stx::bitmap<int> b = { data2.data(), w, h };
		stx::blit_backwards(a, b);
		REQUIRE(data == data2);
	}

	SECTION("bitmap::blit_in_place handles blitting overlapping sections") {
		stx::bitmap<int> b = a.subimage(0, 0, a.w-1, a.h);
		stx::bitmap<int> c = a.subimage(1, 0, a.w-1, a.h);

		int overwritten_voxels;

		// Test l.data < r.data case
		overwritten_voxels = 0;
		std::generate(data.begin(), data.end(), []() { return stx::rand<int>(-1e7, -1); });
		stx::blit_in_place(b, c, [&](int const& src, int& dst) {
			if(src == 0) overwritten_voxels++;
			dst = 0;
		});
		REQUIRE(overwritten_voxels == 0);

		// Test l.data > r.data case
		overwritten_voxels = 0;
		std::generate(data.begin(), data.end(), []() { return stx::rand<int>(-1e7, -1); });
		stx::blit_in_place(c, b, [&](int const& src, int& dst) {
			if(src == 0) overwritten_voxels++;
			dst = 0;
		});
		REQUIRE(overwritten_voxels == 0);
	}

	std::array<float, 16> source = {
		 1,-1, 1,-1,
		-1, 1,-1, 1,
		 1,-1, 1,-1,
		-1, 1,-1, 1
	};
	std::array<float, 9> dst = {
		0.f, 0.f, 0.f,
		0.f, 0.f, 0.f,
		0.f, 0.f, 0.f
	};
	std::array<float, 4> kernel = {
		1.f, 1.f,
		1.f, 1.f,
	};

	SECTION("bitmap::convolve without border") {
		stx::convolve(stx::bitmap<float>(source.data(), 4, 4), stx::bitmap<float>(dst.data(), 3, 3), stx::bitmap<float>(kernel.data(), 2, 2));

		CHECK(std::all_of(dst.begin(), dst.end(), [](float f) { return std::abs(f) < 1e-8f; }));
	}

	// TODO: test for bitmap::convole with border
}
