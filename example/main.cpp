#include <iostream>
#include <ranges>
#include <array>


int main() {
    size_t count = 0;
    for (int i = 0; i < 1'000'000'0; ++i) {
        std::array<size_t, 1'000'000> a {};
        count += a[599'000];
    }
    std::cerr << count << std::endl;
    return 0;
}
