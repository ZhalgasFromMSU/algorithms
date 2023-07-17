#include <iostream>
#include <ranges>
#include <array>


void foo(const std::ranges::random_access_range auto& x) {
    auto data = std::ranges::begin(x);
    std::cerr << data[0] << std::endl;
}

int main() {
    foo(std::ranges::single_view(1));
    return 0;
}
