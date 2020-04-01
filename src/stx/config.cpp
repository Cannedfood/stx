#include "config.hpp"

#include <cassert>
#include <fstream>
#include <algorithm>
#include <vector>

#if __has_include(<filesystem>)
	#include <filesystem>
	namespace filesystem = std::filesystem;
#elif __has_include(<experimental/filesystem>)
	#include <experimental/filesystem>
	namespace filesystem = std::experimental::filesystem;
#else
	#define STX_NO_FILESYSTEM
	#pragma message("No <filesystem> or <experimental/filesystem>, config will not support directories.")
#endif

namespace stx {

void config::set(std::string name, std::string value) noexcept {
	m_entries[std::move(name)] = std::move(value);
}

std::string config::get(std::string const& name) {
	std::string result;
	if(!get(name, &result)) throw std::runtime_error("Couldn't find config entry '" + name + "'");
	return result;
}
bool config::get(std::string const& name, std::string* into) noexcept {
	assert(into);

	auto iter = m_entries.find(name);

	if(iter == m_entries.end()) return false; // Not found

	*into = iter->second;
	return true;
}
std::string config::get(std::string const& name, std::string const& fallback) noexcept {
	std::string result;
	return get(name, &result) ? result : fallback;
}

double config::getf(std::string const& name) {
	double result;
	if(!get(name, &result)) throw std::runtime_error("Couldn't find config entry '" + name + "'");
	return result;
}
bool config::get(std::string const& name, double* into) noexcept {
	auto iter = m_entries.find(name);

	if(iter == m_entries.end()) return false; // Not found

	*into = std::stod(iter->second);
	return true;
}
double config::get(std::string const& name, double fallback) noexcept {
	double result;
	return get(name, &result) ? result : fallback;
}

bool config::getb(std::string const& name) {
	bool result;
	if(!get(name, &result)) throw std::runtime_error("Couldn't find config entry '" + name + "'");
	return result;
}
bool config::get(std::string const& name, bool* into) noexcept {
	auto iter = m_entries.find(name);

	if(iter == m_entries.end()) return false; // Not found

	*into = iter->second == "true" || iter->second == "1";
	return true;
}
bool config::get(std::string const& name, bool fallback) noexcept {
	bool result;
	return get(name, &result) ? result : fallback;
}

void config::parseIni(std::string const& path) {
	auto status = filesystem::status(path);

	#ifndef STX_NO_FILESYSTEM
	// Is path a directory?
	if(status.type() == filesystem::file_type::directory) {
		// A directory

		// Read directory contents IN ALPHABETICAL ORDER
		std::vector<std::string> directory_contents;
		for(auto& entry : filesystem::recursive_directory_iterator(path)) {
			directory_contents.push_back(entry.path().string());
		}
		std::sort(directory_contents.begin(), directory_contents.end());
		for(auto& content : directory_contents) {
			parseIni(content);
		}

		return;
	}
	#endif // !defined(STX_NO_FILESYSTEM)

	// A file
	std::ifstream file(path);
	parseIni(file);
}

void config::parseIni(std::istream& stream) {
	constexpr auto npos = std::string_view::npos;

	std::string section;

	std::string raw_line;
	while(std::getline(stream, raw_line)) {
		std::string_view line = raw_line;

		// Remove comment
		if(auto commentBegin = line.find_first_of(';'); commentBegin != npos) {
			line = line.substr(0, commentBegin);
		}

		// Trim leading whitespace
		while(!line.empty() && std::isspace(line.front())) line.remove_prefix(1);

		// Trim trailing whitespace
		while(!line.empty() && std::isspace(line.back())) line.remove_suffix(1);

		if(line.empty()) continue; // Line is empty

		// New Section?
		if(line.front() == '[' && line.back() == ']') {
			line.remove_prefix(1);
			line.remove_suffix(1);
			section = std::string(line) + ".";
			continue;
		}

		std::string_view key, value;
		value = line;
		while(value.size() && value.front() != '=')
			value.remove_prefix(1);
		if(value.empty()) throw std::runtime_error("Missing =");
		value.remove_prefix(1);

		key = line.substr(0, line.size() - value.size() - 1);

		set(section + std::string(key), std::string(value));
	}
}

void config::parseCmd(int argc, const char** argv) noexcept {
	for(int i = 0; i < argc; i++) {
		const char* arg = argv[i];
		if(arg[0] == '-' && arg[1] == '-') {
			std::string_view option(arg + 2);

			size_t equals_pos = option.find('=');
			if(equals_pos == std::string_view::npos) {
				set(std::string(option), "true");
			}
			else {
				std::string_view option_name = option.substr(0, equals_pos);
				std::string_view option_value = option.substr(equals_pos + 1);
				set(std::string(option_name), std::string(option_value));
			}
		}
	}

}

extern "C" char** environ;

void config::importEnv() noexcept {
	for(char** envar = environ; *envar; envar++) {
		std::string_view option(*envar);

		size_t equals_pos = option.find('=');
		if(equals_pos == std::string_view::npos) {
			set(std::string(option), "true");
		}
		else {
			std::string_view option_name = option.substr(0, equals_pos);
			std::string_view option_value = option.substr(equals_pos + 1);
			set(std::string(option_name), std::string(option_value));
		}
	}
}

} // namespace stx
