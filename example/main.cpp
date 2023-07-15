#include <iostream>
#include <ranges>
#include <array>


void foo(std::ranges::range auto&& x) {
    x.abc();
    std::cerr << sizeof(x) << std::endl;
}


int main() {
    std::array<size_t, 100> a;
    // foo(a);
    foo(std::ranges::take_view(a, 100));
    return 0;
}
