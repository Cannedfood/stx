// Copyright (c) 2018 Benno Straub, licensed under the MIT license. (A copy can be found at the bottom of this file)

// Utilities for 3d textures/voxel engines

#pragma once

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

	constexpr bitmap(T* data, u32 w, u32 h) noexcept;
	constexpr bitmap(T* data, u32 w, u32 h, u32 elements_per_scanline) noexcept;

	constexpr bitmap<T> subimage(u32 w, u32 h) noexcept;
	constexpr bitmap<T> subimage(i32 x, i32 y, u32 w, u32 h) noexcept;

	template<class Callback>
	void each(Callback&& callback) {
		T* scanline = data;
		for(u32 y = 0; y < h; y++) {
			for(u32 x = 0; x < w; x++) {
				callback(scanline[x]);
			}
			scanline += elements_per_scanline;
		}
	}

	constexpr i32 index(i32 x, i32 y) const noexcept { return x + y * elements_per_scanline; }
	constexpr T&  operator()(i32 x, i32 y) const noexcept { return data[index(x, y)]; }

	constexpr u32 area() const noexcept { return w * h; }
};


// == Blitting =================================================

/// Copies one bitmap to another
// The destination bitmap should not overlap with the source bitmap! (use blit_in_place for that)
template<class Src, class Dst, class Assigner = void(*)(Src&,Dst&)>
void blit(
	bitmap<Src> src, bitmap<Dst> dst,
	Assigner assign = [](auto&a,auto&b){a=std::move(b);}) noexcept;

/// Copies one bitmap to another
// The destination bitmap should not overlap with the source bitmap! (use blit_in_place for that)
template<class Src, class Dst, class Assigner = void(*)(Src&,Dst&)>
void blit_backwards(
	bitmap<Src> src, bitmap<Dst> dst,
	Assigner assign = [](auto&a,auto&b){a=std::move(b);}) noexcept;

/// Copies one bitmap to another
/// The destination bitmap may overlap with the source bitmap
/// If this is not needed use blit, because it has better memory access patterns
template<class Src, class Dst, class Assigner = void(*)(Src&,Dst&)>
void blit_in_place(
	bitmap<Src> src, bitmap<Dst> dst,
	Assigner assign = [](auto&a,auto&b){a=std::move(b);}) noexcept;

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
			assign(dst_scanline[x], src_scanline[x]);
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
			assign(dst_scanline[x], src_scanline[x]);
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

} // namespace stx

/*
 Copyright (c) 2018 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
