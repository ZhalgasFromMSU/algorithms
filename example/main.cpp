#include <iostream>
#include <ranges>
#include <vector>

void foo(std::ranges::random_access_range auto&& range) {
    auto data = std::ranges::begin(range);
    for (size_t i = 0; i < std::ranges::size(range); ++i) {
        std::cerr << data[i] << std::endl;
    }
}

class A {
    std::vector<int> a {1, 2, 3};
public:
    auto begin() {
        return a.begin();
    }

    auto end() {
        return a.end();
    }
};


int main() {
    A a;

    foo(a);
    return 0;
}
