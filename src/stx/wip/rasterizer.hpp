#pragma once

#include <utility>
#include <cassert>
#include <cmath>

#include <type_traits>

#ifdef STX_RASTERIZER_BITMAP_SUPPORT
#include "../bitmap.hpp"
#endif

namespace stx {

// Arithmetic type that does literally nothing
struct arithmetic_void {
	constexpr inline arithmetic_void() noexcept = default;

	constexpr inline arithmetic_void  operator*(float f) const noexcept { return {}; }
	constexpr inline arithmetic_void  operator/(float f) const noexcept { return {}; }
	constexpr inline arithmetic_void& operator*=(float f) noexcept { return *this; }
	constexpr inline arithmetic_void& operator/=(float f) noexcept { return *this; }

	constexpr inline arithmetic_void  operator+(arithmetic_void) const noexcept { return {}; }
	constexpr inline arithmetic_void  operator-(arithmetic_void) const noexcept { return {}; }
	constexpr inline arithmetic_void& operator+=(arithmetic_void) noexcept { return *this; }
	constexpr inline arithmetic_void& operator-=(arithmetic_void) noexcept { return *this; }
};

template<class Z, class Callback>
void rasterize_triangle(
	float ax, float ay, Z az,
	float bx, float by, Z bz,
	float cx, float cy, Z cz,
	Callback&& callback)
{
	static_assert(std::is_invocable_v<Callback, int, int, Z const&>, "Callback should look like: void callback(int, int, Z const&) { ... }");

	// TODO: implement non-conservative rasterization
	// TODO: calculate coverage at edges
	// constexpr bool includeMinY = false;
	// constexpr bool includeMaxY = false;
	// constexpr bool incluceMinX = false;
	// constexpr bool includeMaxX = false;

	// Sort the points so that ay < by < ay
	if(ay > by) {
		std::swap(ax, bx);
		std::swap(ay, by);
		std::swap(az, bz);
	}
	if(by > cy) {
		std::swap(bx, cx);
		std::swap(by, cy);
		std::swap(bz, cz);

		if(ay > by) {
			std::swap(ax, bx);
			std::swap(ay, by);
			std::swap(az, bz);
		}
	}

	// Calculate gradients

	// Naming convention:
	//   Difference:
	//     abDiffX < along the x-axis
	//     ^^From point a to point b
	//   Gradient:
	//     abStepX < gradient of x in relation to y
	//     ^^From point a to point b

	float abDiffY = by - ay;
	float abDiffX = bx - ax;
	Z     abDiffZ = bz - az;
	float abStepX = abDiffX / abDiffY;
	Z     abStepZ = abDiffZ / abDiffY;

	float acDiffY = cy - ay;
	float acDiffX = cx - ax;
	Z     acDiffZ = cz - az;
	float acStepX = acDiffX / acDiffY;
	Z     acStepZ = acDiffZ / acDiffY;

	float bcDiffY = cy - by;
	float bcDiffX = cx - bx;
	Z     bcDiffZ = cz - bz;
	float bcStepX = bcDiffX / bcDiffY;
	Z     bcStepZ = bcDiffZ / bcDiffY;

	// Determine whether the line a-b is to the right of a-c
	// Used later to fill the scanline from the lower x to higher x
	bool bIsToTheRightOfAC = (ax + acStepX * abDiffY) < bx;

	// Utility function for rasterizing one scanline
	auto rasterScanline = [&callback](int y, float xstart, Z zstart, float xend, Z zend) {
		int x = std::ceil(xstart);

		Z stepZ = (zend - zstart) / (xend - xstart);
		Z z     = zstart + stepZ * (x - xstart); // Corrected for std::ceil(xstart)

		while(x < xend) {
			callback(x, y, z);
			x++;
			z += stepZ;
		}
	};

	// Points that we move along the line
	float acX = ax;
	Z     acZ = az;

	float abX = ax;
	Z     abZ = az;

	int y = std::ceil(ay);
	{ // Correct for rounding (std::ceil(ay))
		float roundingError = y - ay;
		acX += acStepX * roundingError;
		acZ += acStepZ * roundingError;
		abX += abStepX * roundingError;
		abZ += abStepZ * roundingError;
	}

	// Rasterize triangle from a.y to b.y
	while(y <= by) {
		if (bIsToTheRightOfAC)
			rasterScanline(y, acX, acZ, abX, abZ);
		else
			rasterScanline(y, abX, abZ, acX, acZ);

		acX += acStepX;
		acZ += acStepZ;
		abX += abStepX;
		abZ += abStepZ;
		y++;
	}

	// Rasterize triangle from b.y to c.y
	float bcX = bx + bcStepX * (y - by); // Again, correct for rounding error
	Z     bcZ = bz + bcStepZ * (y - by);
	while(y <= cy) {
		if (bIsToTheRightOfAC)
			rasterScanline(y, acX, acZ, bcX, bcZ);
		else
			rasterScanline(y, bcX, bcZ, acX, acZ);

		acX += acStepX;
		acZ += acStepZ;
		bcX += bcStepX;
		bcZ += bcStepZ;
		y++;
	}
}

template<class Callback>
void rasterize_triangle(
	float ax, float ay,
	float bx, float by,
	float cx, float cy,
	Callback&& callback)
{
	rasterize_triangle(
		ax, ay, arithmetic_void{},
		bx, by, arithmetic_void{},
		cx, cy, arithmetic_void{},
		[&](int x, int y, arithmetic_void _) {
			callback(x, y);
		}
	);
}

#ifdef STX_RASTERIZER_BITMAP_SUPPORT

template<class Z>
void rasterize_triangle(
	float ax, float ay, Z az,
	float bx, float by, Z bz,
	float cx, float cy, Z cz,
	stx::bitmap<Z> output)
{
	rasterize_triangle(
		ax, ay, az,
		bx, by, bz,
		cx, cy, cz,
		[output](int x, int y, Z const& z) { output(x, y) = z; }
	);
}

#endif

} // namespace stx
