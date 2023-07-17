#pragma once

#include <iostream>
#include <source_location>

#define ASSERT(expr, ...) if (!(expr)) [[unlikely]] { std::cerr << "Assert failed: " __VA_ARGS__ << '\n' \
                                                                << std::source_location::current().file_name() << ':' << std::source_location::current().line() << ": " \
                                                                << #expr << std::endl; std::terminate(); }
