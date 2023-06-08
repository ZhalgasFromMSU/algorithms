#include <iostream>

#include <algo/finite_field/finite_field.hpp>

int main() {
    std::cerr << algo::ModuloField<131>::primitive() << std::endl;
    return 0;
}
