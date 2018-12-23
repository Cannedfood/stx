#include "../injector.hpp"

#ifndef NDEBUG
#include "../type.hpp"
#endif

namespace stx {

static size_t get_id(std::type_info const& info, size_t quirk) {
	return (info.hash_code() + quirk) ^ quirk;
}

injector::entry_t injector::get(std::type_info const& info, size_t quirk) {
	auto id = get_id(info, quirk);

	auto iter = m_entries.find(id);
	if(iter != m_entries.end()) return iter->second;

	auto factory_iter = m_factories.find(id);
	if(factory_iter == m_factories.end())
		throw std::runtime_error("Couldn't find " + std::string(info.name()));

	#ifndef NDEBUG
		static int depth = 0;
		printf(
			"[stx:injector](info) %.*s%sCreating %s\n",
			std::clamp(depth * 2 - 3, 0, 32), "                                  ",
			depth > 0 ? u8" ⮡ " : "",
			stx::demangle(info.name()).c_str());
		depth++;
	#endif
	auto result = m_entries.emplace_hint(iter, id, factory_iter->second(*this))->second;
	#ifndef NDEBUG
		depth--;
	#endif

	return result;
}
void  injector::entry(entry_t   e, std::type_info const& info, size_t quirk) {
	m_entries.try_emplace(get_id(info, quirk), e);
}
void  injector::factory(factory_t f, std::type_info const& info, size_t quirk) {
	m_factories.try_emplace(get_id(info, quirk), f);
}

} // namespace stx
