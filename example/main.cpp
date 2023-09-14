#include <iostream>
#include <concepts>
#include <ranges>


void foo(std::ranges::range auto range) {
    for (auto i : range) {
        std::cerr << i << std::endl;
    }
}

int main() {
    auto l = {1, 2, 3};
    foo({1, 2, 3});
    return 0;
}
