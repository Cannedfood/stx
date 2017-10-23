
namespace stx {

template<typename T, typename LoadInfo, typename Hash, typename Pred>
cache<T, LoadInfo, Hash, Pred>::cache() {}
template<typename T, typename LoadInfo, typename Hash, typename Pred>
cache<T, LoadInfo, Hash, Pred>::~cache() {}

template<typename T, typename LoadInfo, typename Hash, typename Pred>
shared<T> cache<T, LoadInfo, Hash, Pred>::add_entry(LoadInfo const& i, owned<T>&& t) {

}

template<typename T, typename LoadInfo, typename Hash, typename Pred>
void cache<T, LoadInfo, Hash, Pred>::hot_swap(LoadInfo const& i, T&& t) {
	if(auto s = m_entries[i].lock()) {

	}
}

template<typename T, typename LoadInfo, typename Hash, typename Pred>
shared<T> cache<T, LoadInfo, Hash, Pred>::load(LoadInfo const& i) {
	if(auto s = m_entries[i].lock()) {
		return s;
	}
	else {
		s = load_uncached(i);
		m_entries[i] = s;
		return s;
	}
}
template<typename T, typename LoadInfo, typename Hash, typename Pred>
owned<T> cache<T, LoadInfo, Hash, Pred>::load_uncached(LoadInfo const& i) {
	return new_owned<T>(i);
}

template<typename T, typename LoadInfo, typename Hash, typename Pred>
void cache<T, LoadInfo, Hash, Pred>::clear() {
	m_entries.clear();
}

} // namespace stx
