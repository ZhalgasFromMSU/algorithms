#include <iostream>
#include <vector>

template<class T>
using vec = std::vector<T>;


class MyClass {
private:
public:
    int val_;
    MyClass()
        :   val_(0)
    {}

//    friend std::istream& operator>>(std::istream& in, const MyClass& self);
    friend std::ostream& operator<<(std::ostream& out, const MyClass& self);
};
/*
std::istream& operator>>(std::istream& in, const MyClass& obj) {
    in >> obj.val_;
    return in;
}*/

std::ostream& operator<<(std::ostream& out, const MyClass& obj) {
    out << obj.val_;
    return out;
}

int main() {
    size_t n;
    std::cin >> n;
    vec<MyClass> inp[n];
    for (size_t i = 0; i < inp.size(); ++i) {
        std::cin >> inp[i].val_;
    }
    for (const auto& i: inp) {
        std::cout << i << '\t';
    }
    return 0;
}
