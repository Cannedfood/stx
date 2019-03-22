// Copyright (c) 2018 Benno Straub, licensed under the MIT license. (A copy can be found at the bottom of this file)

// Utilities for 3d textures/voxel engines

#pragma once

#include <tuple>
#include <algorithm>

namespace stx {


// == bitmap3d =================================================

/// Represents a subsection of bitmapped 3d array
template<class T>
struct bitmap3d {
	using u32 = unsigned;
	using i32 = int;

	T*  data;
	u32 w, h, d;
	u32 elements_per_scanline;
	u32 elements_per_slice;

	constexpr bitmap3d(T* data, u32 w, u32 h, u32 d) noexcept;
	constexpr bitmap3d(T* data, u32 w, u32 h, u32 d, u32 elements_per_scanline, u32 elements_per_slice) noexcept;

	constexpr bitmap3d<T> subimage(u32 w, u32 h, u32 d) noexcept;
	constexpr bitmap3d<T> subimage(i32 x, i32 y, i32 z, u32 w, u32 h, u32 d) noexcept;

	constexpr std::tuple<u32, u32, u32> indexOf(T const* element) const noexcept;

	template<class Callback>
	void each(Callback&& callback) {
		T* slice = data;
		for(u32 z = 0; z < d; z++) {
			T* scanline = slice;
			for(u32 y = 0; y < h; y++) {
				for(u32 x = 0; x < w; x++) {
					callback(scanline[x]);
				}
				scanline += elements_per_scanline;
			}
			slice += elements_per_slice;
		}
	}

	constexpr i32 index(i32 x, i32 y, i32 z) const noexcept { return x + y * elements_per_scanline + z * elements_per_slice; }
	constexpr T*  get(i32 x, i32 y, i32 z)         noexcept { return data + index(x, y, z); }
	constexpr T&  operator()(i32 x, i32 y, i32 z)  noexcept { return data[index(x, y, z)]; }

	constexpr u32 volume() const noexcept { return w * h * d; }
};


// == Blitting =================================================

/// Copies one bitmap to another
// The destination bitmap should not overlap with the source bitmap! (use blit_in_place for that)
template<class Src, class Dst, class Assigner = void(*)(Src&,Dst&)>
void blit(
	bitmap3d<Src> src, bitmap3d<Dst> dst,
	Assigner assign = [](Dst&a,Src&b){a=b;}) noexcept;

/// Copies one bitmap to another
// The destination bitmap should not overlap with the source bitmap! (use blit_in_place for that)
template<class Src, class Dst, class Assigner = void(*)(Src&,Dst&)>
void blit_backwards(
	bitmap3d<Src> src, bitmap3d<Dst> dst,
	Assigner assign = [](Dst&a,Src&b){a=b;}) noexcept;

/// Copies one bitmap to another
/// The destination bitmap may overlap with the source bitmap
/// If this is not needed use blit, because it has better memory access patterns
template<class Src, class Dst, class Assigner = void(*)(Src&,Dst&)>
void blit_in_place(
	bitmap3d<Src> src, bitmap3d<Dst> dst,
	Assigner assign = [](Dst&a,Src&b){a=b;}) noexcept;


// == Relocating =================================================
template<class Src, class Dst, class Assigner = void(*)(Src&, Dst&), class Finalizer = void(*)(Src&), class Initializer = void(*)(Dst&)>
void relocate(
	stx::bitmap3d<Src> from, int from_x, int from_y, int from_z,
	stx::bitmap3d<Dst> to,   int to_x,   int to_y,   int to_z,
	Assigner&&    assign  = [](Src&a,Dst&b){a=b;},
	Finalizer&&   voider  = [](Src&a){},
	Initializer&& creator = [](Dst&a){}
) noexcept;

} // namespace stx



// =============================================================
// == Inline Implementation =============================================
// =============================================================

namespace stx {

namespace detail {

template<class Src, class Dst, class Assigner>
void invoke_assigner(
	int from_x, int from_y, int from_z,
	int to_x, int to_y, int to_z,
	Src& src, Dst& dst,
	Assigner&& assigner)
{
	if constexpr(std::is_invocable_v<Assigner, int, int, int, int, int, int, Src&, Dst&>) {
		assigner(from_x, from_y, from_z, to_x, to_y, to_z, std::forward<Src>(src), std::forward<Dst>(dst));
	}
	else if constexpr(std::is_invocable_v<Assigner, Src&, Dst&>) {
		assigner(std::forward<Src>(src), std::forward<Dst>(dst));
	}
	else {
		static_assert(
			"Expected assigner to be of the form "
			"void(Src& src, Dst& dst)"
			" or "
			"void(int from_x, int from_y, int from_z, int to_x, int to_y, int to_z, Src& src, Dst& dst)."
		);
	}
}

} // namespace detail

// -- bitmap3d -------------------------------------------------------

template<class T> constexpr
bitmap3d<T>::bitmap3d(T* data, u32 w, u32 h, u32 d) noexcept
	: data(data), w(w), h(h), d(d), elements_per_scanline(w), elements_per_slice(w * h)
{}

template<class T> constexpr
bitmap3d<T>::bitmap3d(
	T* data, u32 w, u32 h, u32 d, u32 elements_per_scanline, u32 elements_per_slice) noexcept
	: data(data), w(w), h(h), d(d), elements_per_scanline(elements_per_scanline), elements_per_slice(elements_per_slice)
{}

template<class T> constexpr
bitmap3d<T> bitmap3d<T>::subimage(u32 w, u32 h, u32 d) noexcept
{
	return bitmap3d<T>(data, w, h, d, elements_per_scanline, elements_per_slice);
}

template<class T> constexpr
bitmap3d<T> bitmap3d<T>::subimage(i32 x, i32 y, i32 z, u32 w, u32 h, u32 d) noexcept
{
	return bitmap3d<T>(data + index(x, y, z), w, h, d, elements_per_scanline, elements_per_slice);
}

template<class T> constexpr
std::tuple<unsigned, unsigned, unsigned> bitmap3d<T>::indexOf(T const* element) const noexcept {
	u32 index = element - data;

	u32 z = index / elements_per_slice;
	u32 y = (index % elements_per_slice) / elements_per_scanline;
	u32 x = ((index % elements_per_slice) % elements_per_scanline) / 1;

	return { x, y, z };
}

// -- Blitting -------------------------------------------------------

template<class Src, class Dst, class Assigner>
void blit(bitmap3d<Src> src, bitmap3d<Dst> dst, Assigner assign) noexcept
{
	using u32 = unsigned;

	Src* src_slice = src.data;
	Dst* dst_slice = dst.data;
	for(u32 z=0;z<src.d;z++) {
		Src* src_scanline = src_slice;
		Dst* dst_scanline = dst_slice;
		for(u32 y=0;y<src.h;y++) {
			for(u32 x=0;x<src.w;x++) {
				assign(dst_scanline[x], src_scanline[x]);
			}
			src_scanline += src.elements_per_scanline;
			dst_scanline += dst.elements_per_scanline;
		}
		src_slice += src.elements_per_slice;
		dst_slice += dst.elements_per_slice;
	}
}

template<class Src, class Dst, class Assigner>
void blit_backwards(bitmap3d<Src> src, bitmap3d<Dst> dst, Assigner assign) noexcept
{
	using i32 = int;

	Src* src_slice = src.data;
	Dst* dst_slice = dst.data;
	for(i32 z=src.d-1;z>=0;z--) {
		Src* src_scanline = src_slice;
		Dst* dst_scanline = dst_slice;
		for(i32 y=src.h-1;y>=0;y--) {
			for(i32 x=src.w-1;x>=0;x--) {
				assign(dst_scanline[x], src_scanline[x]);
			}
			src_scanline += src.elements_per_scanline;
			dst_scanline += dst.elements_per_scanline;
		}
		src_slice += src.elements_per_slice;
		dst_slice += dst.elements_per_slice;
	}
}

template<class Src, class Dst, class Assigner>
void blit_in_place(bitmap3d<Src> src, bitmap3d<Dst> dst, Assigner assign) noexcept
{
	if(src.data >= dst.data) {
		blit<Src, Dst, Assigner>(src, dst, assign);
	}
	else {
		blit_backwards<Src, Dst, Assigner>(src, dst, assign);
	}
}

// -- Relocating ------------------------------------------------------
template<class Src, class Dst, class Assigner, class Finalizer, class Initializer>
void relocate(
	stx::bitmap3d<Src> from, int from_x, int from_y, int from_z,
	stx::bitmap3d<Dst> to,   int to_x,   int to_y,   int to_z,
	Assigner&&    assign,
	Finalizer&&   finalize,
	Initializer&& init) noexcept
{
	int from_max_x = from_x + from.w;
	int from_max_y = from_y + from.h;
	int from_max_z = from_z + from.d;

	int to_max_x = to_x + to.w;
	int to_max_y = to_y + to.h;
	int to_max_z = to_z + to.d;

	int overlap_x = std::max(from_x, to_x);
	int overlap_y = std::max(from_y, to_y);
	int overlap_z = std::max(from_z, to_z);

	int overlap_max_x = std::clamp(to_max_x, overlap_x, from_max_x);
	int overlap_max_y = std::clamp(to_max_y, overlap_y, from_max_y);
	int overlap_max_z = std::clamp(to_max_z, overlap_z, from_max_z);

	stx::bitmap3d<Src> overlap = from.subimage(
		overlap_x - from_x, overlap_y - from_y, overlap_z - from_z,
		overlap_max_x - overlap_x, overlap_max_y - overlap_y, overlap_max_z - overlap_z
	);

	// printf("Overlap from (%i %i %i) (%ux%ux%u)\n",
	// 	overlap_x - from_x, overlap_y - from_y, overlap_z - from_z,
	// 	overlap.w, overlap.h, overlap.d
	// );

	// Apply finalizer
	{
		// Front slab
		// from.subimage(
		// 		0, 0, 0,
		// 		from.w, from.h, overlap_x - from_x)
		//     .each(std::forward<Finalizer>(finalize));

		// Back slab
		// from.subimage(
		// 		0, 0, 0,
		// 		from.w, from.h, overlap_max_x - from_max_x)
		//     .each(std::forward<Finalizer>(finalize));
	}

	// Apply initializer
	{

	}

	// Copy overlap
	{
		bitmap3d<Dst> to_overlap = to.subimage(
			overlap_x - to_x, overlap_y - to_y, overlap_z - to_z,
			overlap.w, overlap.h, overlap.d
		);

		// printf("Overlap to (%i %i %i) (%ux%ux%u)\n",
		// 	overlap_x - to_x, overlap_y - to_y, overlap_z - to_z,
		// 	to_overlap.w, to_overlap.h, to_overlap.d
		// );

		blit_in_place(
			overlap, to_overlap,
			std::forward<Assigner>(assign)
		);
	}
}

} // namespace stx

/*
 Copyright (c) 2018 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
