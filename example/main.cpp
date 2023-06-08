#include "algo/finite_field/util.hpp"
#include <iostream>

int main() {
    for (int i = 2; i * i < 1'000'005'001; ++i) {
        if (algo::IsPrime(i * i + 1)) {
            std::cerr << i << std::endl;
        }
    }
    return 0;
}
