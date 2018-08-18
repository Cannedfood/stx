#include "catch.hpp"

#include "../serial.hpp"
#include <sstream>

TEST_CASE("Test serial varlen writing and parsing", "[serial]") {
	std::stringstream stream;

	stx::writeVarlen(stream, uint64_t(1) << 0);
	CHECK(stx::readVarlen(stream) == uint64_t(1) << 0);
	stx::writeVarlen(stream, uint64_t(1) << 8);
	CHECK(stx::readVarlen(stream) == uint64_t(1) << 8);
	stx::writeVarlen(stream, uint64_t(1) << 16);
	CHECK(stx::readVarlen(stream) == uint64_t(1) << 16);
	stx::writeVarlen(stream, uint64_t(1) << 24);
	CHECK(stx::readVarlen(stream) == uint64_t(1) << 24);
	stx::writeVarlen(stream, uint64_t(1) << 32);
	CHECK(stx::readVarlen(stream) == uint64_t(1) << 32);
	stx::writeVarlen(stream, uint64_t(1) << 40);
	CHECK(stx::readVarlen(stream) == uint64_t(1) << 40);
	stx::writeVarlen(stream, uint64_t(1) << 48);
	CHECK(stx::readVarlen(stream) == uint64_t(1) << 48);
	stx::writeVarlen(stream, uint64_t(1) << 56);
	CHECK(stx::readVarlen(stream) == uint64_t(1) << 56);
	stx::writeVarlen(stream, uint64_t(1) << 63);
	CHECK(stx::readVarlen(stream) == uint64_t(1) << 63);
	stx::writeVarlen(stream, ~uint64_t(0));
	CHECK(stx::readVarlen(stream) == ~uint64_t(0));
}

TEST_CASE("Test serial string writing and parsing", "[serial]") {
	std::stringstream stream;

	std::string test_string =
		"Heyho, das wird ein langer string mit äöü und so. Bla bla bla"
		"      w   h  i  t  e  s  p  a  c  e      "
		"And stuff asdasdasdasd";
	stx::writeString(stream, test_string);
	CHECK(stx::readString(stream) == test_string);
}
