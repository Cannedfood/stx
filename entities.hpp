// Copyright (c) 2018 Benno Straub, licensed under the MIT license. (A copy can be found at the bottom of this file)

// A entity component system

#pragma once

#include <unordered_map>
#include <vector>
#include <cstdint>

namespace stx {

class entities;

namespace detail {

class type_abstraction {
	size_t m_alignment;
public:
	size_t alignment();

	type_abstraction(size_t align) : m_alignment(align) {}

	virtual void destroy(
		uint8_t* data, size_t count, size_t stride) = 0;

	virtual void move_assign(
		size_t count,
		uint8_t* src_data, size_t src_stride,
		uint8_t* dst_data, size_t dst_stride) = 0;

	virtual void move_construct(
		size_t count,
		uint8_t* src_data, size_t src_stride,
		uint8_t* dst_data, size_t dst_stride) = 0;
};

template<class T>
class type final : public type_abstraction {
public:
	type() : type_abstraction(alignof(T)) {}

	void destroy(uint8_t* data, size_t count, size_t stride) override {
		for(size_t i = 0; i < count; i++) {
			((T*)data + i * stride)->~T();
		}
	}

	void move_assign(
		size_t count,
		uint8_t* src_data, size_t src_stride,
		uint8_t* dst_data, size_t dst_stride) override
	{
		for(size_t i = 0; i < count; i++) {
			auto* src = ((T*) src_data + src_stride * i);
			auto* dst = ((T*) dst_data + dst_stride * i);
			*dst = std::move(*src);
		}
	}

	void move_construct(
		size_t count,
		uint8_t* src_data, size_t src_stride,
		uint8_t* dst_data, size_t dst_stride) override
	{
		for(size_t i = 0; i < count; i++) {
			auto* src = ((T*) src_data + src_stride * i);
			auto* dst = ((T*) dst_data + dst_stride * i);
			dst->T(std::move(*src));
		}
	}
};

class entity_class {
public:
	size_t                             m_stride;
	std::unordered_map<size_t, size_t> m_memory_layout;
	std::vector<uint8_t>               m_memory;

	std::set<entity_class*>  m_generalizations;
	std::set<entity_class*>  m_specializations;
};

template<class... Components>
struct entity_class_binding {
	std::array<size_t, sizeof...(Components)> m_component_layout;

	entity_class_binding(entity_class& data) {
		constexpr size_t hash_codes[] = { typeid(Components).hash_code()... };
		for(size_t i = 0; i < sizeof...(Components); i++) {
			m_component_layout[i] = data.m_memory_layout.at(hash_codes[i]);
		}
	}
};

template<class... Components>
class filter_iterator {
public:
	// std::tuple<Components&..., uint64_t> operator*() {
	//
	// }

	filter_iterator& operator++() noexcept {
		return *this; // TODO
	}

	bool operator!=(filter_iterator const& other) const noexcept {
		return true; // TODO
	}
};

} // namespace detail

template<class... Components>
class filter {
	using entity_class_binding = detail::entity_class_binding<Components...>;

	std::vector<entity_class_binding> m_bindings;
public:
	using iterator = detail::filter_iterator<Components...>;

	filter() {}
	filter(entities& es) {}

	iterator begin() {

	}
	iterator end() {

	}
};

class entities {
public:
};

} // namespace stx

/*
 Copyright (c) 2018 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
