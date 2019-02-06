#pragma once

#include <unordered_map>
#include <iosfwd>

namespace stx {

/// A lightweight-ish configuration class
class config {
	std::unordered_map<std::string, std::string> m_entries;
public:
	void        set(std::string name, std::string value) noexcept;

	std::string get(std::string const& name);
	bool        get(std::string const& name, std::string* into) noexcept;
	std::string get(std::string const& name, std::string const& fallback) noexcept;
	std::string get(std::string const& name, const char* fallback) noexcept { return get(name, std::string(fallback)); }


	void   set(std::string name, double value) noexcept;
	double getf(std::string const& name);
	bool   get(std::string const& name, double* into) noexcept;
	double get(std::string const& name, double fallback) noexcept;
	double get(std::string const& name, int fallback) noexcept { return get(name, (double) fallback); }

	bool getb(std::string const& name);
	bool get(std::string const& name, bool* into) noexcept;
	bool get(std::string const& name, bool fallback) noexcept;

	void parseIni(std::string const& path);
	void parseIni(std::istream& stream);
};

} // namespace stx
