#include <iostream>
#include <format>



int main() {
    std::cerr << std::format("1{}", 2) << std::endl;
    return 0;
}
