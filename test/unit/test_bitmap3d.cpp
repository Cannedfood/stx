#include "catch.hpp"
#include <stx/bitmap3d.hpp>
#include <stx/random.hpp>

TEST_CASE("Test bitmap3d", "[bitmap3d]") {
	unsigned w = 5, h = 7, d = 11;

	std::vector<int> data(w*h*d);
	std::generate(data.begin(), data.end(), []() { return stx::rand<int>(-1e7, -1); });

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
		std::generate(data.begin(), data.end(), []() { return stx::rand<int>(-1e7, -1); });
		stx::blit_in_place(b, c, [&](int& a, int& b) {
			if(b == 0) overwritten_voxels++;
			a = 0;
		});
		REQUIRE(overwritten_voxels == 0);

		// Test l.data > r.data case
		overwritten_voxels = 0;
		std::generate(data.begin(), data.end(), []() { return stx::rand<int>(-1e7, -1); });
		stx::blit_in_place(c, b, [&](int& a, int& b) {
			if(b == 0) overwritten_voxels++;
			a = 0;
		});
		REQUIRE(overwritten_voxels == 0);
	}

	// TODO:
	SECTION("bitmap::relocate works") {
		unsigned border = 1;
		unsigned w1 = 2, h1 = 2, d1 = 2;
		unsigned w2 = w1+border, h2 = h1+border, d2 = d1+border;

		std::vector<int> from_data(w1*h1*d1, -1);
		std::vector<int> to_data  (w2*h2*d2, -1);

		stx::bitmap3d<int> from {from_data.data(), w1, h1, d1};
		stx::bitmap3d<int> to   {to_data.data(),   w2, h2, d2};

		SECTION("initializer, finalizer and assigner are applied correctly") {
			enum PixelState {
				Assigned,
				Initialized,
				Finalized,
			};

			stx::relocate(
				from, 1, 1, 1,
				to,   0, 0, 0,
				[](auto&a,auto&b) { a = Assigned;    }, // Assigner (a = b)
				[](auto&a)        { a = Finalized;   }, // Finalizer
				[](auto&a)        { a = Initialized; }  // Initializer
			);

			// Copied area should have the assigner run over it.
			to.subimage(border, border, border, w1, h1, d1).each([](auto& v) {
				CHECK(v == Assigned);
			});

			// Everything should be initialized
			to.each([](auto& v) {
				bool initialized_or_assigned = (v == Initialized) || (v == Assigned);
				CHECK(initialized_or_assigned);
			});
		}

		// Just tests for the copying
		SECTION("overlap is copied correctly") {
			for(unsigned i = 0; i < w1; i++)
				from(i, i, i) = i;

			stx::relocate(
				from, border, border, border,
				to,   0, 0, 0
			);

			for(unsigned i = 0; i < w1; i++)
				CHECK(to(i + border, i + border, i + border) == i);
		}
	}
}
