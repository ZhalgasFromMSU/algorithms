#include <iostream>
#include <vector>

template<class T,
    class Alloc = std::allocator<T>>
class Vector {

};

int main() {
    std::vector<int> a = {1, 2, 3, 4};
    for (std::vector<int>::const_iterator it = a.begin(); it != a.end(); ++it) {
        std::cout << *it << '\n';
    }
    return 0;
}
