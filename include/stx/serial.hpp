#pragma once

#include <iostream>
#include <string>

namespace stx {

constexpr inline
uint32_t magic(const char (&string)[5]) {
	uint32_t value = 0;
	for(size_t i = 0; i < 4; i++) {
		value <<= 8;
		value |= string[i];
	}
	return value;
}

constexpr inline
uint64_t magic(const char (&string)[9]) {
	uint32_t value = 0;
	for(size_t i = 0; i < 8; i++) {
		value <<= 8;
		value |= string[i];
	}
	return value;
}

template<class T>
void writeRaw(std::ostream& stream, T const& data) {
	stream.write((char const*)&data, sizeof(T));
}

template<class T>
void writeRaw(std::ostream& stream, T const* data, size_t count) {
	stream.write((char const*)data, sizeof(T) * count);
}

template<class T>
void readRaw(std::istream& stream, T& data) {
	stream.read((char*)&data, sizeof(T));
}

template<class T>
void readRaw(std::istream& stream, T* data, size_t count) {
	stream.read((char*)data, sizeof(T) * count);
}

template<class T>
T readRaw(std::istream& stream) {
	T t;
	readRaw(stream, t);
	return t;
}

template<class T>
bool readCompare(std::istream& stream, T const& comp) {
	return stx::readRaw<T>(stream) == comp;
}

void     writeVarlen(std::ostream& stream, uint64_t var) noexcept;
uint64_t readVarlen(std::istream& stream) noexcept;

void        writeString(std::ostream& stream, std::string_view v);
std::string readString(std::istream& stream);

class file_position {
	std::ostream& m_stream;
	size_t        m_position;
public:
	file_position(std::ostream& stream, size_t seek = 0) :
		m_stream(stream),
		m_position(stream.tellp())
	{
		if(seek) {
			stream.seekp(seek, std::ios::cur);
		}
	}
	~file_position() {}

	template<class T>
	void write(T const& t) {
		size_t end = m_stream.tellp();
		m_stream.seekp(m_position);
		stx::writeRaw<T>(m_stream, t);
		m_stream.seekp(end);
	}

	template<class T>
	void writeWrittenSize(size_t divisor = 1) {
		size_t end = m_stream.tellp();
		m_stream.seekp(m_position);
		stx::writeRaw<T>(m_stream, static_cast<T>((end - m_position) / divisor));
		m_stream.seekp(end);
	}
};

} // namespace stx
