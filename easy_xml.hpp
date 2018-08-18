#pragma once

#include "xml.hpp"

#include <map>
#include <unordered_map>
#include <functional>

namespace stx {

class easy_xml {
	std::map<std::string, easy_xml>                             m_child_node;

	std::unordered_map<std::string, std::function<void(xml::attribute&)>> m_attribute_callbacks;

	std::function<void(xml::node&)>                                       m_start;
	std::function<void(std::string_view const&)>                          m_content;
	std::function<void(xml::node&)>                                       m_finish;

	std::vector<std::string> m_look_first_for;
public:
	easy_xml& get(std::string_view const& path) {
		size_t separator = path.find('.', 0);
		if(separator == std::string_view::npos)
			return m_child_node[std::string(path)];
		else
			return m_child_node[std::string(path.substr(0, separator - 1))].get(path.substr(separator));
	}

	template<class Callback>
	easy_xml& start(Callback&& c) {
		m_content = std::forward<Callback>(c);
		return *this;
	}

	template<class Callback>
	easy_xml& attribute(std::string name, Callback&& c) {
		m_attribute_callbacks[std::move(name)] = std::forward<Callback>(c);
		return *this;
	}

	easy_xml& attribute(std::string name, std::string& value) {
		m_attribute_callbacks[std::move(name)] = [&value](auto& v) { value = v; };
		return *this;
	}

	template<class Callback>
	easy_xml& content(Callback&& c) {
		m_content = std::forward<Callback>(c);
		return *this;
	}

	template<class Callback>
	easy_xml& finish(Callback&& c) {
		m_content = std::forward<Callback>(c);
		return *this;
	}
};

} // namespace stx