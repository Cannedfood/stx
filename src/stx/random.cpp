#include "random.hpp"

namespace stx {

thread_local random<std::mt19937> default_random = { std::mt19937(std::random_device{}()) };

} // namespace stx
