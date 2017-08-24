
namespace stx {

template<typename T, typename TLoadInfo>
cache<T, TLoadInfo>::cache() {}
template<typename T, typename TLoadInfo>
cache<T, TLoadInfo>::~cache() {}

template<typename T, typename TLoadInfo>
shared<T> cache<T, TLoadInfo>::add_entry(LoadInfo const& i, owned<T>&& t) {

}

template<typename T, typename TLoadInfo>
void cache<T, TLoadInfo>::hot_swap(LoadInfo const& i, T&& t) {
	if(auto s = m_entries[i].lock()) {

	}
}

template<typename T, typename TLoadInfo>
shared<T> cache<T, TLoadInfo>::load(LoadInfo const& i) {
	if(auto s = m_entries[i].lock()) {
		return s;
	}
	else {
		s = load_uncached(i);
		m_entries[i] = s;
		return s;
	}
}
template<typename T, typename TLoadInfo>
owned<T> cache<T, TLoadInfo>::load_uncached(LoadInfo const& i) {
	return new_owned<T>(i);
}

} // namespace stx
