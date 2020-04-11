#pragma once

#include <algorithm>
#include <cmath>

namespace stx {

template<class Callback>
void rasterizeLine(
	float xfrom, float yfrom,
	float xto,   float yto,
	Callback&& callback) noexcept;

template<class Callback>
void rasterizeTriangle(
	float x0, float y0,
	float x1, float y1,
	float x2, float y2,
	Callback&& callback) noexcept;

} // namespace stx

// =============================================================
// == Inline implementation =============================================
// =============================================================

template<class Callback>
void stx::rasterizeLine(
	float xfrom, float yfrom,
	float xto,   float yto,
	Callback&& callback) noexcept
{
	float dx = xto - xfrom;
	float dy = yto - yfrom;

	if(std::abs(dx) <= std::abs(dy)) {
		if(xfrom > xto) {
			std::swap(xfrom, xto);
			std::swap(yfrom, yto);
			dx = -dx;
			dy = -dy;
		}

		int   x     = std::ceil(xfrom);
		int   xend  = std::floor(xto) + 1;
		float ystep = dy / dx;
		float y     = yfrom + ystep * (xfrom - x);
		while(x < xend) {
			callback(x, std::ceil(y));
			y += ystep;
			x++;
		}
	}
	else {
		if(yfrom > yto) {
			std::swap(xfrom, xto);
			std::swap(yfrom, yto);
			dx = -dx;
			dy = -dy;
		}

		int   y     = std::ceil(yfrom);
		int   yend  = std::floor(yto) + 1;
		float xstep = dx / dy;
		float x     = xfrom + xstep * (yfrom - y);
		while(y < yend) {
			callback(y, std::ceil(x));
			x += xstep;
			y++;
		}
	}
}

template<class Callback>
void stx::rasterizeTriangle(
	float x0, float y0,
	float x1, float y1,
	float x2, float y2,
	Callback&& callback) noexcept
{
	// Sort points by y-coordinate
	if(y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
	if(y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
	if(y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }

	//              x0/y0, ystart
	//              |\
	//              | \ slope dx0to1
	// slope dx0to2 |  \ x1/y1, ymiddle
	//              |  /
	//              | / slope dx1to2
	//              |/
	//              x2/y2, yend

	const int ystart  = std::ceil(y0);
	const int ymiddle = std::ceil(y1);
	const int yend    = std::ceil(y2);

	const float dx0to1 = (x1-x0) / (y1-y0);
	const float dx1to2 = (x2-x1) / (y2-y1);
	const float dx0to2 = (x2-x0) / (y2-y0);

	const bool isLeft = dx0to1 < dx0to2; // Whether x1 is to the left of the line from p0 to p2

	float xLeftStep  = isLeft? dx0to1:dx0to2;
	float xRightStep = isLeft? dx0to2:dx0to1;
	float xLeft      = x0 + (y0 - ystart) * xLeftStep;
	float xRight     = x0 + (y0 - ystart) * xRightStep;

	int y = ystart;
	while(y < ymiddle) {
		for(int x = std::ceil(xLeft), xend = int(std::floor(xRight)) + 1; x < xend; x++) {
			callback(x, y);
		}
		xLeft  += xLeftStep;
		xRight += xRightStep;
		y++;
	}

	(isLeft?xLeftStep:xRightStep) = dx1to2;
	(isLeft?xLeft:xRight)         = x1 + dx1to2 * (y1 - ymiddle);

	while(y < yend) {
		for(int x = std::ceil(xLeft), xend = int(std::floor(xRight)) + 1; x < xend; x++) {
			callback(x, y);
		}
		xLeft  += xLeftStep;
		xRight += xRightStep;
		y++;
	}
}
