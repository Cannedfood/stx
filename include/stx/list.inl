// Copyright (c) 2017 Benno Straub, licensed under the MIT license. (A copy can be found at the end of this file)

namespace stx {

template<class T> constexpr
list_element<T>::list_element() :
	m_next(nullptr),
	m_to_this(nullptr)
{}

template<class T> constexpr
list_element<T>::list_element(list_element_t const& other) :
	list_element()
{ *this = other; }
template<class T> constexpr
list_element<T>::list_element(list_element_t&& other) :
	list_element()
{ *this = (list_element_t&&)other; }

template<class T> constexpr
list_element<T>& list_element<T>::operator=(list_element_t&& other) {
	remove();
	if((m_next = other.m_next)) {
		m_next->m_to_this = &m_next;
	}
	if((m_to_this = other.m_to_this)) {
		*m_to_this = (T*)this;
	}
	other.m_next    = nullptr;
	other.m_to_this = nullptr;
	return *this;
}
template<class T> constexpr
list_element<T>& list_element<T>::operator=(list_element_t const& other) {}

template<class T> constexpr
bool list_element<T>::remove() {
	bool result = m_next || m_to_this;
	if(m_next)
		m_next->m_to_this = m_to_this;
	if(m_to_this)
		*m_to_this = m_next;
	m_next = nullptr;
	m_to_this = nullptr;
	return result;
}

template<class T> constexpr
void list_element<T>::add_to(T*& t) {
	remove();
	if((m_next = t)) {
		m_next->m_to_this = &m_next;
	}
	m_to_this  = &t;
	*m_to_this = (T*) this;
}
template<class T> constexpr
void list_element<T>::add_to(list_element<T>& t) {
	add_to(t.m_next);
}

template<class T> constexpr
T* list_element<T>::snip() {
	if(m_to_this)
		*m_to_this = nullptr;
	return (T*) this;
}

template<class T> constexpr
T* list_element<T>::back(T* end) {
	T* result = (T*)this;
	while(result->next() != end) result = result->next();
	return result;
}
template<class T> constexpr
T* list_element<T>::center(T* end) {
	T* result = (T*)this;
	T* tmp    = (T*)this;
	while(true) {
		tmp    = tmp->next();
		if(tmp == end) break;
		tmp    = tmp->next();
		if(tmp == end) break;
		result = result->next();
	}
	return result;
}

template<class T>
class list_element<T>::iterator {
	mutable list_element<T>* m_value;
public:
	iterator() : m_value(nullptr) {}
	iterator(T* t) : m_value(t) {}

	iterator& operator++(void) const noexcept { m_value = m_value->next(); }
	iterator operator++(int) const noexcept { return iterator(m_value->next()); }
	T* operator->() { return (T*) m_value; }
	T& operator*() { return *((T*) m_value); }
	T const* operator->() const { return (T*) m_value; }
	T const& operator*() const { return *((T*) m_value); }
	bool operator==(iterator const& other) const noexcept { return m_value == other.m_value; }

	operator T*() noexcept { return m_value; }
	operator T const*() const noexcept { return m_value; }
};

} // namespace stx

/*
 Copyright (c) 2017 Benno Straub

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
