

namespace stx {

// == list_element<T> =========================================================

template<typename T>
void list_element<T>::update_next_last() noexcept {
	if(m_next)
		m_next->m_last = this;
	if(m_last)
		m_last->m_next = this;
}

template<typename T> constexpr
list_element<T>::list_element() :
	m_next(nullptr),
	m_last(nullptr)
{}

template<typename T> constexpr
list_element<T>::list_element(Tself&& e) :
	m_next(e.m_next),
	m_last(e.m_last)
{
	e.m_next = e.m_last = nullptr;
	update_next_last();
}

template<typename T>
list_element<T>::~list_element() noexcept {
	remove();
}

template<typename T> constexpr
list_element<T>& list_element<T>::operator=(list_element<T>&& e) noexcept {
	remove();
	m_next = e.m_next;
	m_last = e.m_last;
	e.m_next = e.m_last = nullptr;
	update_next_last();
}

template<typename T>
void list_element<T>::remove() noexcept {
	if(m_next) {
		m_next->m_last = m_last;
	}
	if(m_last) {
		m_last->m_next = m_next;
	}
	m_next = nullptr;
	m_last = nullptr;
}

template<typename T>
void list_element<T>::push_front(T* p) noexcept {
	p->remove();
	T* h = head();
	h->m_last = p;
	p->m_next = h;
}

template<typename T>
void list_element<T>::push_back(T* p) noexcept {
	p->remove();
	T* t = tail();
	t->m_next = p;
	p->m_last = t;
}

template<typename T>
void list_element<T>::insert_as_last(T* p) noexcept {
	p->remove();
	if(m_last) {
		m_last->m_next = p;
	}
	p->m_last = m_last;
	p->m_next = (T*) this;
	m_last = p;
}

template<typename T>
void list_element<T>::insert_as_next(T* p) noexcept {
	p->remove();
	if(m_next) {
		m_next->m_last = p;
	}
	p->m_next = m_next;
	p->m_last = (T*) this;
	m_next = p;
}

template<typename T>
T* list_element<T>::tail() noexcept {
	return const_cast<T*>(((const T*) this)->tail());
}

template<typename T>
const T* list_element<T>::tail() const noexcept {
	T* t = (T*) this;
	do {
		if(!t->m_next) {
			return t;
		}
		else {
#ifdef STX_DEBUG
			xassertmsg(t != t->m_next, "Cycle detected!");
#endif
			t = t->m_next;
		}
	} while(t);
	return nullptr;
}

template<typename T>
T* list_element<T>::head() noexcept {
	return const_cast<T*>(((const T*) this)->head());
}

template<typename T>
const T* list_element<T>::head() const noexcept {
	T* t = (T*) this;
	do {
		if(!t->m_last)
			return t;
		else
			t = t->m_last;
	} while(t);
	return nullptr;
}

// == parent_element<T, TChild> ===============================================

// -- Constructor ------------------------------------------------------------

template<typename T, typename TChild>
parent_element<T, TChild>::parent_element() :
	m_children(nullptr)
{}

template<typename T, typename TChild>
parent_element<T, TChild>::~parent_element() noexcept {
	// Remove all children
	while(children())
		children()->remove();
}


// -- Move --------------------------------------------------------------
template<typename T, typename TChild>
parent_element<T, TChild>::parent_element(Tself&& e) :
	m_children(e.m_children)
{
	e.m_children = nullptr;
}

template<typename T, typename TChild>
parent_element<T, TChild>& parent_element<T, TChild>::operator=(parent_element<T, TChild>&& other) noexcept {
	std::swap(m_children, other.m_children);
}

template<typename T, typename TChild>
void parent_element<T, TChild>::push_back(TChild* c) noexcept {
	if(!m_children) {
		m_children = c;
		c->m_parent = (T*) this;
	}
	else {
		m_children->push_back(c);
	}
}

template<typename T, typename TChild>
void parent_element<T, TChild>::push_front(TChild* c) noexcept {
	if(!m_children) {
		m_children = c;
		c->m_parent = (T*) this;
	}
	else {
		m_children->push_front(c);
	}
}

// == child_element<T, TParent> ==============================================

template<typename T, typename TParent> constexpr
child_element<T, TParent>::child_element() {}

template<typename T, typename TParent>
child_element<T, TParent>::~child_element() noexcept {
	remove();
}

template<typename T, typename TParent> constexpr
child_element<T, TParent>::child_element(child_element<T, TParent>&& other) :
	list_element<T>(std::move(other))
{
	m_parent = other.m_parent;
}

template<typename T, typename TParent> constexpr
child_element<T, TParent>& child_element<T, TParent>::operator=(child_element<T, TParent>&& other) noexcept {
	m_parent = other.m_parent;
	list_element<T>::operator=(std::move(other));
}

template<typename T, typename TParent>
void child_element<T, TParent>::push_front(T* t) noexcept {
	list_element<T>::push_front(t);
	t->m_parent = m_parent;
	m_parent->m_children = t;
}

template<typename T, typename TParent>
void child_element<T, TParent>::push_back(T* t) noexcept {
	list_element<T>::push_back(t);
	t->m_parent = m_parent;
}

template<typename T, typename TParent>
void child_element<T, TParent>::remove() noexcept {
	if(m_parent) {
		if(m_parent->children() == this) {
			m_parent->m_children = list_element<T>::next();
		}
		m_parent = nullptr;
	}
	list_element<T>::remove();
}

} // namespace stx
