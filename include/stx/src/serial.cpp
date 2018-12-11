#include "../serial.hpp"

namespace stx {

void writeVarlen(std::ostream& stream, uint64_t value) noexcept {
	// TODO
	#define WRITE_BITS(N) \
		if(value > ~((~uint64_t(0)) << (N))) \
			writeRaw(stream, uint8_t((value >> (N)) | 0x80))

	WRITE_BITS(7 * 9);
	WRITE_BITS(7 * 8);
	WRITE_BITS(7 * 7);
	WRITE_BITS(7 * 6);
	WRITE_BITS(7 * 5);
	WRITE_BITS(7 * 4);
	WRITE_BITS(7 * 3);
	WRITE_BITS(7 * 2);
	WRITE_BITS(7 * 1);
	writeRaw(stream, uint8_t(value & 0x7F));
}

uint64_t readVarlen(std::istream& stream) noexcept {
	uint8_t  byte   = readRaw<uint8_t>(stream);
	uint64_t result = byte & 0x7F;
	while(byte & 0x80) {
		byte = readRaw<uint8_t>(stream);
		result <<= 7;
		result |= byte & 0x7F;
	}
	return result;
}

void writeString(std::ostream& stream, std::string_view v) {
	writeVarlen(stream, v.size());
	stream.write(v.data(), v.size());
}

std::string readString(std::istream& stream) {
	std::string result(readVarlen(stream), ' ');
	stream.read(result.data(), result.size());
	return result;
}

} // namespace stx
