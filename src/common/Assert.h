#pragma once

#include <cstdlib>
#include <print>
#include <source_location>

namespace dibeast::detail {

[[noreturn]] inline void assertFailed(const char* expression, std::source_location where) {
  std::println(stderr, "assertion failed: {} at {}:{} in {}", expression, where.file_name(),
               where.line(), where.function_name());
  std::abort();
}

}  // namespace dibeast::detail

#ifdef NDEBUG
#define DIBEAST_ASSERT(expr) static_cast<void>(0)
#else
#define DIBEAST_ASSERT(expr)     \
  ((expr) ? static_cast<void>(0) \
          : ::dibeast::detail::assertFailed(#expr, std::source_location::current()))
#endif
