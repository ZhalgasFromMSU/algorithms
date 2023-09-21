#include <iostream>
#include <limits>

int main() {
    static_assert(~0u == std::numeric_limits<unsigned>::max());
    return 0;
}
