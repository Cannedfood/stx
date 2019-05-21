#include "injector.hpp"

#ifndef NDEBUG
#include "type.hpp"
#endif

#include <algorithm>

namespace stx {

static size_t get_id(std::type_info const& info, size_t quirk) {
	return (info.hash_code() + quirk) ^ quirk;
}

injector::injector() :
	m_entries({ this })
{}

#ifndef NDEBUG
static int depth = 0;
#endif

injector::entry_t injector::get(std::type_info const& info, size_t quirk) {
	auto id = get_id(info, quirk);

	entry_t& entry = m_entries[id];
	if(entry) return entry;

	auto factory_iter = m_factories.find(id);
	if(factory_iter == m_factories.end()) {
		std::string msg = "Couldn't find " + std::string(info.name());
		throw std::runtime_error(msg);
	}

	#ifndef NDEBUG
		depth++;
	#endif
	entry = factory_iter->second(*this);
	#ifndef NDEBUG
		depth--;
		printf(
			"[stx::injector](info) %.*s%sCreating %s\n",
			std::clamp((depth - 1) * 3, 0, 32), "                                  ",
			depth > 0 ? u8"  ↙" : "",
			stx::demangle(info.name()).c_str());
	#endif

	return entry;
}
void  injector::entry(entry_t   e, std::type_info const& info, size_t quirk) {
	m_entries.try_emplace(get_id(info, quirk), e);
}
void  injector::factory(factory_t f, std::type_info const& info, size_t quirk) {
	m_factories.try_emplace(get_id(info, quirk), f);

	#ifndef NDEBUG
		printf("[stx::injector](info) Factory %s\n", stx::demangle(info.name()).c_str());
	#endif
}

} // namespace stx
