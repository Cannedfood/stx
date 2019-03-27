#pragma once

#include <exception>
#include <string>
#include <string_view>
#include <array>

namespace stx::parsing {

struct cursor_location {
	unsigned         line;
	unsigned         column;
	std::string_view line_content;

	cursor_location(unsigned line, unsigned column);
	cursor_location(const char* start, const char* location);
};

template<class Pred> constexpr
std::array<bool, 256> make_lookup(Pred pred) {
	std::array<bool, 256> values { false };
	for(unsigned char c = 0; c < 255; c++) {
		values[c] = pred((char) c);
	}
	return values;
}

namespace errors {
	class error : public std::exception {
		std::string m_message;
		const char* m_location;
		std::string_view m_location2;
	public:
		error(std::string message, const char* location = nullptr, std::string_view location2 = {}) :
			m_message(std::move(message)),
			m_location(location), m_location2(location2)
		{}

		const char* what() const noexcept override { return m_message.c_str(); }
		const char* location() const noexcept { return m_location; }
		std::string_view location2() const noexcept { return m_location2; }

		void diagnose(
			const char* sourcefile,
			const char* source,
			std::ostream& printTo);
		void diagnose(
			const char* sourcefile,
			const char* source);
	};

	class parsing_error : public error {
	public: using error::error;
	};

	class attribute_not_found : public error {
	public: using error::error;
	};

	class node_not_found : public error {
	public: using error::error;
	};
} // namespace errors

} // namespace stx::parsing