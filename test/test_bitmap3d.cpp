#include "catch.hpp"
#include "../bitmap3d.hpp"
#include "../random.hpp"

TEST_CASE("Test bitmap3d", "[bitmap3d]") {
	unsigned w = 5, h = 7, d = 11;

	std::vector<int> data(w*h*d);
	std::generate(data.begin(), data.end(), []() { return stx::rand<int>(-1, -1e7); });

	stx::bitmap3d<int> a = { data.data(), w, h, d };

	SECTION("bitmap3d::each goes over each voxel") {
		a.each([](auto& i) { i = 42; });

		REQUIRE(
			std::all_of(data.begin(), data.end(), [](int i) { return
				i == 42;
			})
		);
	}

	SECTION("bitmap3d::subimage(w, h, d) works") {
		auto b = a.subimage(4, 4, 4);
		b.each([](int& i) { i = 1337; });
		REQUIRE(64 == std::count(data.begin(), data.end(), 1337));
	}

	SECTION("bitmap::subimage(x,y,z,w,h,d) works") {
		auto b = a.subimage(w-4,h-4,d-4, 4,4,4); // 4x4x4 Corner cube

		b.each([](int& i) { i = 80085; });

		REQUIRE(64 == std::count(data.begin(), data.end(), 80085));

		bool no_80085_outside_of_cube = true;
		{
			auto also_not_80085 = [&](int& i) {
				no_80085_outside_of_cube = no_80085_outside_of_cube && (i != 80085);
			};

			a.subimage(w, h, d-4).each(also_not_80085);
			a.subimage(w, h-4, d).each(also_not_80085);
			a.subimage(w-4, h, d).each(also_not_80085);
		}

		REQUIRE(no_80085_outside_of_cube);
	}

	SECTION("blit works") {
		std::vector<int> data2(data.size());
		stx::bitmap3d<int> b = { data2.data(), w, h, d };
		stx::blit(a, b);
		REQUIRE(data == data2);
	}

	SECTION("blit_backwards works like bitmap::blit") {
		std::vector<int> data2(data.size());
		stx::bitmap3d<int> b = { data2.data(), w, h, d };
		stx::blit_backwards(a, b);
		REQUIRE(data == data2);
	}

	SECTION("bitmap::blit_in_place handles blitting overlapping sections") {
		stx::bitmap3d<int> b = a.subimage(0, 0, 0, a.w-1, a.h, a.d);
		stx::bitmap3d<int> c = a.subimage(1, 0, 0, a.w-1, a.h, a.d);

		int overwritten_voxels;

		// Test l.data < r.data case
		overwritten_voxels = 0;
		std::generate(data.begin(), data.end(), []() { return stx::rand<int>(-1, -1e7); });
		stx::blit_in_place(b, c, [&](int& a, int& b) {
			if(b == 0) overwritten_voxels++;
			a = 0;
		});
		REQUIRE(overwritten_voxels == 0);

		// Test l.data > r.data case
		overwritten_voxels = 0;
		std::generate(data.begin(), data.end(), []() { return stx::rand<int>(-1, -1e7); });
		stx::blit_in_place(c, b, [&](int& a, int& b) {
			if(b == 0) overwritten_voxels++;
			a = 0;
		});
		REQUIRE(overwritten_voxels == 0);
	}
}
