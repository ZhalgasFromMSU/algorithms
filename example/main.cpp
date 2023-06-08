#include <iostream>

#include <algo/finite_field/finite_field.hpp>

int main() {
    for (int i = 0; i < 30; ++i) {
        algo::ModuloField<31> a {i};
        std::cerr << a.ToPower(6).value() << std::endl;
    }
    return 0;
}
