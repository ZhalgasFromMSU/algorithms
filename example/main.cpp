#include <iostream>
#include <ranges>
#include <vector>


void goo(std::ranges::range auto&& s) {
    std::cerr << "range\t" << std::ranges::size(s) << '\n';
    for (auto it = std::ranges::begin(s); it != std::ranges::end(s); ++it) {
        std::cerr << *it << '\t';
    }
    std::cerr << std::endl;
}

void foo(std::ranges::range auto&& s) {
    std::cerr << "fo range\t" << std::ranges::size(s) << '\n';
    goo(std::ranges::subrange(std::ranges::begin(s), std::ranges::begin(s) + 2));
    std::cerr << std::endl;
}

int main() {
    std::array<int, 13> ar = {1, 2, 3, 4};
    foo(ar);
    goo(std::ranges::subrange(ar.begin(), ar.begin() + 2));
    std::cerr << std::endl;
    
    return 0;
}
