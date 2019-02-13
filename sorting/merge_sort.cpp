#include <iostream>
#include <vector>
#include <functional>
#include <iterator>

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

// it1            it2             end2
//   |             |               |
//   v1 v2 v3 v4 v5 v6 v7 v8 v9 v10
template<typename Iterator, typename Cmp>
void merge(Iterator it1, Iterator it2, Iterator end2,
         Iterator buff, Cmp cmp) {
    Iterator it, end1(it2);
    size_t size = std::distance(it1, end2);
    for (size_t i = 0; i < size && it1 != end1 && it2 != end2; ++i) {
        if (cmp(*it1, *it2)) {
            *it = *it2;
            ++it2;
        } else {
            *it = *it1;
            ++it1;
        }
        ++it;
    }
    if (it1 == end1) {
        std::copy(it2, end2, it);
    } else if (it2 == end2) {
        std::copy(it1, end1, it);
    }
}

template<typename T, typename Cmp>
void merge_sort(std::vector<T>& inp, Cmp cmp) {
    std::vector<T> buffer(inp.size());
}

template<typename T>
void merge_sort(std::vector<T>& inp) {
    merge_sort(inp, std::greater<T>());
}

int main() {
    size_t n;
    std::cin >> n;
    std::vector<MyClass> inp(n);
    for (auto& i: inp) {
        std::cin >> i;
    }
//    merge_sort(inp);
    std::vector<MyClass> buf(n);
    merge(inp.begin(), inp.begin() + 3, inp.end(), buf.begin(), std::greater<MyClass>());
    for (auto& i: buf) {
        std::cout << i << '\t';
    }
    return 0;
}
