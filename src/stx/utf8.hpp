#pragma once

#include <cstdint>


namespace stx::utf8 {

unsigned encode(char* utf8_str, uint32_t codepoint) noexcept;
unsigned decode(char const* utf8_str, uint32_t* codepoint) noexcept;

} // namespace stx::utf8


// =============================================================
// == Inline implementation =============================================
// =============================================================

namespace stx::utf8 {

inline
unsigned encode(char* utf8_str, uint32_t codepoint) noexcept {
	if(codepoint < 0x80) {
		utf8_str[0] = ((codepoint >>  0) & 0b01111111) | 0b00000000;
		return 1;
	}
	else if(codepoint < 0x800) {
		utf8_str[0] = ((codepoint >>  6) & 0b00011111) | 0b11000000;
		utf8_str[1] = ((codepoint >>  0) & 0b00111111) | 0b10000000;
		return 2;
	}
	else if(codepoint < 0x10000) {
		utf8_str[0] = ((codepoint >> 12) & 0b00001111) | 0b11100000;
		utf8_str[1] = ((codepoint >>  6) & 0b00111111) | 0b10000000;
		utf8_str[2] = ((codepoint >>  0) & 0b00111111) | 0b10000000;
		return 3;
	}
	else if(codepoint < 0x11000) {
		utf8_str[0] = ((codepoint >> 18) & 0b00000111) | 0b11110000;
		utf8_str[1] = ((codepoint >> 12) & 0b00111111) | 0b10000000;
		utf8_str[2] = ((codepoint >>  6) & 0b00111111) | 0b10000000;
		utf8_str[3] = ((codepoint >>  0) & 0b00111111) | 0b10000000;
		return 4;
	}
	else {
		// Unicode REPLACEMENT CHARACTER U+FFFD
		utf8_str[0] = 0xEF;
		utf8_str[1] = 0xBF;
		utf8_str[2] = 0xBD;
		return 3;
	}
}

inline
unsigned decode(char const* utf8_str, uint32_t* codepoint) noexcept {
	if((utf8_str[0] & 0b10000000) == 0b00000000) {
		*codepoint = utf8_str[0];
		return 1;
	}
	if((utf8_str[0] & 0b11100000) == 0b11000000) {
		*codepoint =
			(utf8_str[0] & 0b00111111) << 6 |
			(utf8_str[1] & 0b00111111);
		return 2;
	}
	if((utf8_str[0] & 0b11100000) == 0b11000000) {
		*codepoint =
			(utf8_str[0] & 0b00011111) << 12 |
			(utf8_str[1] & 0b00111111) <<  6 |
			(utf8_str[2] & 0b00111111); // TODO: check if really continuation bytes??
		return 3;
	}
	if((utf8_str[0] & 0b11110000) == 0b11100000) {
		*codepoint =
			(utf8_str[0] & 0b00001111) << 18 |
			(utf8_str[1] & 0b00111111) << 12 |
			(utf8_str[2] & 0b00111111) <<  6 |
			(utf8_str[3] & 0b00111111);
		return 4;
	}
	*codepoint = 0xFFFD;
	return -1;
}

} // namespace stx::utf8
