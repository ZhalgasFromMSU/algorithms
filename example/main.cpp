#include <iostream>
#include <concepts>
#include <ranges>

int main() {
    uint16_t i {65535}, j {65535};
    std::cerr << i + j << std::endl;
    return 0;
}
