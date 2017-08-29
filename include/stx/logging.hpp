#pragma once

#include <iostream>

#ifdef STX_MODULE_NAME
#	define _STX_TO_STRING(X) #X
#	define _STX_MAKRO_TO_STRING(X) _STX_TO_STRING(X)
#	define STX_MODULE_NAME_STRING _STX_MAKRO_TO_STRING(STX_MODULE_NAME)
#endif

namespace stx {

namespace detail {

template<unsigned level>
std::ostream& get_logstream() { return std::cout; }

template<unsigned level>
void release_logstream(std::ostream& s) { s << std::endl; }

inline void write_simple(std::ostream&) {}

template<typename T, typename... ARGS>
void write_simple(std::ostream& to, T&& t, ARGS&&... args) {
	to << t;
	write_simple(to, std::forward<ARGS>(args)...);
}

template<typename T>
const char* write_formatted_next(std::ostream& s, const char* fmt, T&& t) {
	size_t off = 0;
	while(fmt[off]) {
		if(fmt[off] == '%' && fmt[off + 1] == '%') {
			s.write(fmt, off);
			s << std::forward<T>(t);
			return (fmt + off + 2);
		}
		off++;
	}

	throw std::runtime_error("Unexpected end of formatting string!");
}

static
void write_formatted(std::ostream& s, const char* fmt) {
	s << fmt;
}

template<typename T> static
void write_formatted(std::ostream& s, const char* fmt, T&& t) {
	fmt = write_formatted_next(s, fmt, std::forward<T>(t));

	if(*fmt) s << fmt;
}

template<typename First, typename... ARGS> static
void write_formatted(std::ostream& s, const char* fmt, First&& first, ARGS&&... args) {
	write_formatted(s, write_formatted_next(s, fmt, std::forward<First>(first)), std::forward<ARGS>(args)...);
}

} // namespace detail

enum log_target : unsigned {
	log_verbose = 0,
	log_info,
	log_warn,
	log_perfwarn,
	log_error
};

template<unsigned level, typename... ARGS> static
void writelog(ARGS&&... args) {
	auto& stream = detail::get_logstream<level>();
#ifdef STX_MODULE_NAME
	detail::write_simple(stream, "[" STX_MODULE_NAME_STRING "] ");
#endif
	detail::write_simple(stream, std::forward<ARGS>(args)...);
	detail::release_logstream<level>(stream);
}

template<unsigned level, typename... ARGS> static
void writelog(const char* fmt, ARGS&&... args) {
	auto& stream = detail::get_logstream<level>();
#ifdef STX_MODULE_NAME
	detail::write_simple(stream, "[" STX_MODULE_NAME_STRING "] ");
#endif
	detail::write_formatted(stream, fmt, std::forward<ARGS>(args)...);
	detail::release_logstream<level>(stream);
}

template<typename... ARGS> inline static
void verbose(ARGS&&... args) {
#ifdef STX_LOG_VERBOSE
	writelog<log_verbose>(std::forward<ARGS>(args)...);
#endif
}

template<typename... ARGS> inline static
void info(ARGS&&... args) {
	writelog<log_info>(std::forward<ARGS>(args)...);
}

template<typename... ARGS> inline static
void warn(ARGS&&... args) {
	writelog<log_warn>(std::forward<ARGS>(args)...);
}

template<typename... ARGS> inline static
void perfwarn(ARGS&&... args) {
	writelog<log_perfwarn>(std::forward<ARGS>(args)...);
}

template<typename... ARGS> inline static
void error(ARGS&&... args) {
	writelog<log_error>(std::forward<ARGS>(args)...);
}

} // namespace stx


namespace stx {
namespace detail {

#ifdef STX_LOG_COLORS

template<> inline
std::ostream& get_logstream<log_warn>() {
	std::cout << "\033[33m";
	return std::cout;
}

template<> inline
std::ostream& get_logstream<log_perfwarn>() {
	std::cout << "\033[33m";
	return std::cout;
}

template<> inline
std::ostream& get_logstream<log_error>() {
	std::cout << "\033[31m";
	return std::cerr;
}

template<> inline void release_logstream<log_warn>(std::ostream& s)     { s << "\033[0m" << std::endl; }
template<> inline void release_logstream<log_perfwarn>(std::ostream& s) { s << "\033[0m" << std::endl; }
template<> inline void release_logstream<log_error>(std::ostream& s)    { s << "\033[0m" << std::endl; }

#else // STX_LOG_COLORS

template<> inline
std::ostream& get_logstream<log_verbose>() {
	std::cout << "V: (";
	return std::cout;
}
template<> inline
void release_logstream<log_verbose>(std::ostream& s) {
	s << ")" << std::endl;
}

template<> inline
std::ostream& get_logstream<log_info>() {
	std::cout << "I: ";
	return std::cout;
}

template<> inline
std::ostream& get_logstream<log_warn>() {
	std::cout << "W: ";
	return std::cout;
}

template<> inline
std::ostream& get_logstream<log_perfwarn>() {
	std::cout << "PW:";
	return std::cout;
}

template<> inline
std::ostream& get_logstream<log_error>() {
	std::cout << "E: ";
	return std::cerr;
}

#endif // STX_LOG_COLORS

} // namespace detail
} // namespace stx

#ifdef STX_MODULE_NAME
#	undef _STX_TO_STRING
#	undef _STX_MARKO_TO_STRING
#	undef _STX_MODULE_NAME_STRING
#endif

// These are intentionally not defined when the debug tools are off:
// they are only for debugging and should not end up in release builds
#ifdef STX_DEBUG_TOOLS
#define xhere       ::stx::info("%%:%%: ", STX_FUNCTION, __LINE__);
#define xhere_class ::stx::info("%%:%%: (%%)", STX_FUNCTION, __LINE__, this);
#endif
