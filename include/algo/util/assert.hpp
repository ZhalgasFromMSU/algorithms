#pragma once

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#define ERROR_INFO(expr_str) __FILE__ ":" STRINGIZE(__LINE__) ": " expr_str

#define ASSERT(expr) if (!(expr)) { std::cerr << "Assert failed: " << ERROR_INFO(#expr) << std::endl; std::terminate(); }
