#include <iostream>
#include <ranges>

template<typename T>
void foo(T a) {
    static_assert(std::is_same_v<T, uint32_t>);
}

int main() {
    uint8_t i = 0;
    uint8_t j = 1;
    foo(i * j);
    return 0;
}
