#pragma once

#include "injector.hpp"
#include "type.hpp"

#include <iostream>
#include <fstream>
#include <string_view>

namespace stx {

inline void toDotGraph(stx::injector& inj, std::ostream& stream) {
	using namespace std::string_view_literals;

	auto objects = inj.all();

	stream << "digraph dependencies {";

	auto getBlockType = [](stx::shared_block* block) {
		std::string name = demangle(typeid(*block).name());
		if(name.starts_with("stx::injector::dependency_shared_block<")) {
			std::string_view name_view = name;
			name_view.remove_prefix(("stx::injector::dependency_shared_block<"sv).length());
			name_view.remove_suffix(1);
			name = std::string(name_view);
		}
		return name;
	};

	for(auto& object : objects) {
		auto* block = dynamic_cast<injector::dependency_shared_block_base*>(object.get_block());
		if(block) {
			for(auto& dep : block->dependencies) {
				stream << "\t\"" << getBlockType(object.get_block()) << "\" -> \"" << getBlockType(dep.get_block()) << "\";\n";
			}
		}
	}

	stream << "}";
}

inline void toDotGraph(stx::injector& inj, std::string const& path) {
	std::ofstream file(path);
	toDotGraph(inj, file);
}

} // namespace stx
