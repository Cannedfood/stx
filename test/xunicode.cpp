#include <xunicode>

#include "test.hpp"

template<unsigned nbytes, uint32_t utf32, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3> static
void test_good_utf8() {
	try {
		union {
			char    utf8[5]  = {0, 0, 0, 0, 0};
			uint8_t bytes[5];
		};

		union {
			      char* end = nullptr;
			const char* const_end;
		};

		// Test encode
		test(stx::utf32to8(utf32, utf8, &end) == nbytes);
		test(end - utf8 == nbytes);

		test(bytes[0] == b0); bytes[0] = b0;
		test(bytes[1] == b1); bytes[1] = b1;
		test(bytes[2] == b2); bytes[2] = b2;
		test(bytes[3] == b3); bytes[3] = b3;

		// printf("%X -> %X %X %X %X (needs %X %X %X %X)\n",
		// 	utf32,
		// 	bytes[0], bytes[1], bytes[2], bytes[3],
		// 	b0, b1, b2, b3
		// );

		// Test decode
		test(stx::utf8to32(utf8, &const_end) == utf32);
		test(end - utf8 == nbytes);
	}
	catch(std::runtime_error e) {
		test(!"utf32to8 or utf8to32 threw an exception");
	}
}

template<unsigned nbytes, uint32_t utf32, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3> static
void test_bad_utf8() {
	// TODO: test_bad_utf8
}

void test_xunicode() {
	test_good_utf8<1, 0x00079, 0x79, 0x00, 0x00, 0x00>(); // 'y'
	test_good_utf8<2, 0x000E4, 0xC3, 0xA4, 0x00, 0x00>(); // 'ä'
	test_good_utf8<2, 0x000AE, 0xC2, 0xAE, 0x00, 0x00>(); // '®'
	test_good_utf8<3, 0x020AC, 0xE2, 0x82, 0xAC, 0x00>(); // '€'
	test_good_utf8<4, 0x1D11E, 0xF0, 0x9D, 0x84, 0x9E>(); // '𝄞'

	// TODO: Test for invalid characters
}
