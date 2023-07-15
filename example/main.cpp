#include <iostream>
#include <ranges>
#include <array>


struct A {
    std::array<int, 100> a {};

    auto to_view(size_t s) const {
        return std::views::counted(a.begin(), s);
    }

    void foo(const std::ranges::range auto& range) {
        auto lhs = to_view(30);
        auto rhs = std::views::counted(std::ranges::begin(range), 40);
        lhs.asd();
        rhs.asd();

        std::cerr << sizeof(lhs) << '\t' << std::ranges::size(lhs) << '\t' << sizeof(rhs) << '\t' << std::ranges::size(rhs) << std::endl;
        std::swap(lhs, rhs);
        std::cerr << sizeof(lhs) << '\t' << std::ranges::size(lhs) << '\t' << sizeof(rhs) << '\t' << std::ranges::size(rhs) << std::endl;
    }
};

int main() {
    A a;
    a.foo(A{}.to_view(50));

    return 0;
}
