#include "../include/stx/logging.hpp"
#include "../include/stx/platform.hpp"

namespace stx {

namespace detail {

bool allow_log_colors = false;

template<>
std::ostream& get_logstream<log_info>() { return std::cout; }
template<>
std::ostream& get_logstream<log_warn>() { return std::cout; }
template<>
std::ostream& get_logstream<log_perfwarn>() { return std::cout; }
template<>
std::ostream& get_logstream<log_error>() { return std::cerr; }

template<>
void release_logstream<log_info>() {}
template<>
void release_logstream<log_warn>() {}
template<>
void release_logstream<log_perfwarn>() {}
template<>
void release_logstream<log_error>() {}

} // namespace detail

} // namespace stx
