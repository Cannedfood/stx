// Copyright (c) 2018 Benno Straub, licensed under the MIT license. (A copy can be found at the bottom of this file)

// Utilities for 3d textures/voxel engines

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>

namespace stx {

// == bitmap =================================================

/// Represents a subsection of bitmapped 2d array
template<class T>
struct bitmap {
	using u32 = unsigned;
	using i32 = int;

	T*  data;
	u32 w, h;
	u32 elements_per_scanline;

	constexpr bitmap(T* data = nullptr, u32 w = 0, u32 h = 0) noexcept;
	constexpr bitmap(T* data, u32 w, u32 h, u32 elements_per_scanline) noexcept;

	constexpr bitmap<T> subimage(u32 w, u32 h) noexcept;
	constexpr bitmap<T> subimage(i32 x, i32 y, u32 w, u32 h) noexcept;

	template<class Callback>
	void each(Callback&& callback) {
		T* scanline = data;
		for(u32 y = 0; y < h; y++) {
			for(u32 x = 0; x < w; x++) {
				if constexpr(std::is_invocable_v<Callback, T&, u32, u32>)
					callback(scanline[x], x, y);
				else
					callback(scanline[x]);
			}
			scanline += elements_per_scanline;
		}
	}

	constexpr u32 num_pixels() const noexcept { return w * h; }

	constexpr i32 index(i32 x, i32 y) const noexcept { return x + y * elements_per_scanline; }
	constexpr T&  operator()(i32 x, i32 y) const noexcept { return data[index(x, y)]; }

	constexpr T&  sample_wrapped(i32 x, i32 y) {
		x = x % w;
		if(x < 0) x += w;
		y = y % h;
		if(y < 0) y += h;
		return data[index(x, y)];
	}

	constexpr u32 area() const noexcept { return w * h; }
};

/// A bitmap<T> with static storage
template<class T, unsigned W, unsigned H>
struct static_bitmap : public bitmap<T> {
	constexpr static_bitmap() noexcept {
		this->data = data_storage.data();
		this->w = W;
		this->h = H;
	}

	std::array<T, W*H> data_storage;
};

/// A bitmap<T> with storage on the heap
template<class T, class Deleter = std::default_delete<T[]>>
struct heap_bitmap : public bitmap<T> {
	using u32 = typename bitmap<T>::u32;
	using i32 = typename bitmap<T>::i32;

	Deleter deleter;

	constexpr heap_bitmap(T* data, u32 w, u32 h, Deleter del = {}) noexcept : bitmap<T>(data, w, h), deleter(std::move(del)) {}
	heap_bitmap(u32 w, u32 h) noexcept : heap_bitmap(new T[w*h], w, h, std::default_delete<T[]>()) {}

	constexpr heap_bitmap(heap_bitmap&& other) noexcept :
		bitmap<T>(std::exchange(other.data, nullptr), std::exchange(other.w, 0), std::exchange(other.h, 0), std::exchange(other.deleter, {}))
	{}

	constexpr heap_bitmap& operator=(heap_bitmap&& other) noexcept(Deleter()((T*)nullptr)) {
		std::swap(this->data,    other.data);
		std::swap(this->w,       other.w);
		std::swap(this->h,       other.h);
		std::swap(this->deleter, other.deleter);
		other.reset();
	}

	constexpr heap_bitmap(heap_bitmap const& other) noexcept            = delete;
	constexpr heap_bitmap& operator=(heap_bitmap const& other) noexcept = delete;

	void reset(T* t, u32 w, u32 h, Deleter del = {}) noexcept {
		if(this->data) {
			deleter(std::exchange(this->data, nullptr));
		}
		deleter = std::move(del);
		this->data = std::move(t);
		this->w = w;
		this->h = h;
	}

	void reset() { reset(nullptr, 0, 0, {}); }
};

// == Blitting =================================================

/// Copies one bitmap to another
// The destination bitmap should not overlap with the source bitmap! (use blit_in_place for that)
template<class Src, class Dst, class Assigner = void(*)(Src const&,Dst&)>
void blit(
	bitmap<Src> src, bitmap<Dst> dst,
	Assigner assign = [](Src const& a,Dst&b){b=a;}) noexcept;

/// Copies one bitmap to another
// The destination bitmap should not overlap with the source bitmap! (use blit_in_place for that)
template<class Src, class Dst, class Assigner = void(*)(Src const&,Dst&)>
void blit_backwards(
	bitmap<Src> src, bitmap<Dst> dst,
	Assigner assign = [](Src const& a,Dst&b){b=a;}) noexcept;

/// Copies one bitmap to another
/// The destination bitmap may overlap with the source bitmap
/// If this is not needed use blit, because it has better memory access patterns
template<class Src, class Dst, class Assigner = void(*)(Src const&,Dst&)>
void blit_in_place(
	bitmap<Src> src, bitmap<Dst> dst,
	Assigner assign = [](Src const& a,Dst&b){b=std::move(a);}) noexcept;


// == Sampling ==================================================

template<class T>
T& sample_nearest(stx::bitmap<T> src, float x, float y) noexcept {
	unsigned ux = std::clamp<float>(std::round(x * (src.w - 1)), 0, src.w - 1);
	unsigned uy = std::clamp<float>(std::round(y * (src.h - 1)), 0, src.h - 1);
	return src(ux, uy);
}

template<class T, class MixFn = T(*)(T const& a, float aW, T const& b, float bW)>
T sample_linear(
	stx::bitmap<T> src, float x, float y,
	MixFn mix = [](T const& a, float aW, T const& b, float bW) -> T { return a * aW + b * bW; }) noexcept
{
	x = std::clamp(x, 0.f, 1.f);
	y = std::clamp(y, 0.f, 1.f);

	unsigned floor_x = std::min(src.w-2, x*(src.w-1));
	unsigned floor_y = std::min(src.h-2, y*(src.h-1));

	float fract_x = x - floor_x;
	float fract_y = y - floor_y;

	return mix(
		mix(src(floor_x + 0, floor_y + 0), 1 - fract_x, src(floor_x + 1, floor_y + 0), fract_x), 1 - fract_y,
		mix(src(floor_x + 0, floor_y + 0), 1 - fract_x, src(floor_x + 1, floor_y + 0), fract_x), fract_y
	);
}

// == Filtering/Signal processing ================================================

template<class Src, class Dst, class Kernel, class ZeroFn = Src(*)(), class DefaultValueFn = Src(*)(size_t x, size_t y)>
void convolve(
	bitmap<Src> src, bitmap<Dst> dst, bitmap<Kernel> kernel,
	ZeroFn zero = []() -> Src { return {}; },
	DefaultValueFn defaultValue = [](size_t x, size_t y) -> Src { return {}; }) noexcept;

} // namespace stx



// =============================================================
// == Inline Implementation =============================================
// =============================================================

namespace stx {

// -- bitmap -------------------------------------------------------

template<class T> constexpr
bitmap<T>::bitmap(T* data, u32 w, u32 h) noexcept
	: data(data), w(w), h(h), elements_per_scanline(w)
{}

template<class T> constexpr
bitmap<T>::bitmap(
	T* data, u32 w, u32 h, u32 elements_per_scanline) noexcept
	: data(data), w(w), h(h), elements_per_scanline(elements_per_scanline)
{}

template<class T> constexpr
bitmap<T> bitmap<T>::subimage(u32 w, u32 h) noexcept
{
	return bitmap<T>(data, w, h, elements_per_scanline);
}

template<class T> constexpr
bitmap<T> bitmap<T>::subimage(i32 x, i32 y, u32 w, u32 h) noexcept
{
	return bitmap<T>(data + index(x, y), w, h, elements_per_scanline);
}

// -- Blitting -------------------------------------------------------

template<class Src, class Dst, class Assigner>
void blit(bitmap<Src> src, bitmap<Dst> dst, Assigner assign) noexcept
{
	using u32 = unsigned;

	Src* src_scanline = src.data;
	Dst* dst_scanline = dst.data;
	for(u32 y=0;y<src.h;y++) {
		for(u32 x=0;x<src.w;x++) {
			assign(const_cast<Src const&>(src_scanline[x]), dst_scanline[x]);
		}
		src_scanline += src.elements_per_scanline;
		dst_scanline += dst.elements_per_scanline;
	}
}

template<class Src, class Dst, class Assigner>
void blit_backwards(bitmap<Src> src, bitmap<Dst> dst, Assigner assign) noexcept
{
	using i32 = int;

	Src* src_scanline = src.data;
	Dst* dst_scanline = dst.data;
	for(i32 y=src.h-1;y>=0;y--) {
		for(i32 x=src.w-1;x>=0;x--) {
			assign(const_cast<Src const&>(src_scanline[x]), dst_scanline[x]);
		}
		src_scanline += src.elements_per_scanline;
		dst_scanline += dst.elements_per_scanline;
	}
}

template<class Src, class Dst, class Assigner>
void blit_in_place(bitmap<Src> src, bitmap<Dst> dst, Assigner assign) noexcept
{
	if(src.data >= dst.data) {
		blit<Src, Dst, Assigner>(src, dst, assign);
	}
	else {
		blit_backwards<Src, Dst, Assigner>(src, dst, assign);
	}
}

template<class Src, class Dst, class Kernel, class ZeroFn, class DefaultValueFn>
void convolve(bitmap<Src> src, bitmap<Dst> dst, bitmap<Kernel> kernel, ZeroFn zero, DefaultValueFn defaultValue) noexcept
{
	if(src.w == dst.w + kernel.w - 1 && src.h == dst.w + kernel.h - 1) { // Convolution without padding
		for(size_t y = 0; y < dst.w; y++)
		for(size_t x = 0; x < dst.h; x++)
		{
			dst(x, y) = zero();
			for(size_t kernel_y = 0; kernel_y < kernel.h; kernel_y++)
			for(size_t kernel_x = 0; kernel_x < kernel.w; kernel_x++)
			{
				dst(x, y) += src(x + kernel_x, y + kernel_y) * kernel(kernel_x, kernel_y);
			}
		}
	}
	else if(src.w == dst.w && src.h == dst.h) { // Convolution with padding
		size_t offx = kernel.w / 2;
		size_t offy = kernel.h / 2;

		for(size_t y = 0; y < dst.w; y++)
		for(size_t x = 0; x < dst.h; x++)
		{
			dst(x, y) = zero();
			for(size_t kernel_y = 0; kernel_y < kernel.h; kernel_y++)
			for(size_t kernel_x = 0; kernel_x < kernel.w; kernel_x++)
			{
				int src_x = x + kernel_x - offx;
				int src_y = y + kernel_y - offy;

				// TODO: optimize? Or is the compiler smart enough to invert the if statements?
				if(src_x < 0 || src_x >= (int)src.w || src_y < 0 || src_y >= (int)src.h)
					dst(x, y) += defaultValue((const decltype(src_y))src_x, (const decltype(src_y))src_y);
				else
					dst(x, y) += src(x + kernel_x, y + kernel_y) * kernel(kernel_x, kernel_y);
			}
		}
	}
	else {
		std::terminate();
	}
}

} // namespace stx

/*
 Copyright (c) 2018 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
