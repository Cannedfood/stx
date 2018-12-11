#pragma once

#include <cstdio>
#include <vector>
#include <stdexcept>

namespace stx {

inline std::vector<uint8_t> file2vector(const char* path) {
	FILE* file = fopen(path, "rb");
	if(!file) throw std::runtime_error("Failed opening file '" + std::string(path) + "'");
	fseek(file, 0, SEEK_END);
	std::vector<uint8_t> result(ftell(file));
	rewind(file);
	fread(result.data(), 1, result.size(), file);
	fclose(file);
	return result;
}

inline std::string          file2string(const char* path) {
	FILE* file = fopen(path, "rb");
	if(!file) throw std::runtime_error("Failed opening file '" + std::string(path) + "'");
	fseek(file, 0, SEEK_END);
	std::string result(ftell(file), ' ');
	fseek(file, 0, SEEK_SET);
	fread(result.data(), result.size(), 1, file);
	fclose(file);
	return result;
}

} // namespace stx
