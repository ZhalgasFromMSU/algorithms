#include <iostream>
#include <vector>
#include <functional>

template<class T>
using vec = std::vector<T>;

class MyClass {
private:
public:
    int val_;
    MyClass()
        :   val_(0)
    {}

    bool operator>(const MyClass& other) const {
        return val_ > other.val_;
    }

    friend std::istream& operator>>(std::istream& in, MyClass& self);
    friend std::ostream& operator<<(std::ostream& out, const MyClass& self);
};

std::istream& operator>>(std::istream& in, MyClass& obj) {
    in >> obj.val_;
    return in;
}

std::ostream& operator<<(std::ostream& out, const MyClass& obj) {
    out << obj.val_;
    return out;
}

template<typename T, typename Cmp>
void merge_sort(vec<T>& inp, Cmp cmp) {
    std::cout << cmp(inp[0], inp[1]) << '\n';
}

template<typename T>
void merge_sort(vec<T>& inp) {
    merge_sort(inp, std::greater<T>());
}

int main() {
    size_t n;
    std::cin >> n;
    vec<MyClass> inp(n);
    for (auto& i: inp) {
        std::cin >> i;
    }
    merge_sort(inp);
    return 0;
}
