#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <iterator>
#include <iterator>

class MyClass {
private:
    int val_;
public:
    MyClass()
        :   val_(0)
    {}

    MyClass(int a)
        :   val_(a)
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

// it1         it2 = end1          end2
//   |             |               |
//   v1 v2 v3 v4 v5 v6 v7 v8 v9 v10
template<typename Iter, typename Cmp>
void merge(Iter it1, Iter end1, Iter it2, Iter end2, Iter buff, Cmp cmp) {
    size_t size = std::distance(it1, end1) + std::distance(it2, end2);
    for (size_t i = 0; i < size - 1 && it1 != end1 && it2 != end2; ++i) {
        if (cmp(*it1, *it2)) {
            *buff++ = *it2++;
        } else {
            *buff++ = *it1++;
        }
    }
    if (it1 == end1) {
        std::copy(it2, end2, buff);
    } else if (it2 == end2) {
        std::copy(it1, end1, buff);
    }
}

template<typename Iter, typename Cmp>
void sort(Iter begin, Iter end, Iter buff, Cmp cmp) {
    std::cout << *begin << '\t' << *(end - 1) << '\n';
    if (std::distance(begin, end) < 2) {
        return;
    }
    Iter mid = begin;
    Iter mid_buff = buff;
    std::advance(mid_buff, std::distance(begin, end) / 2);
    std::advance(mid, std::distance(begin, end) / 2);
    sort(begin, mid, buff, cmp);
    sort(mid, end, ++mid_buff, cmp);
    merge(begin, mid, mid, end, buff, cmp);
}

template<typename T, typename Cmp>
void merge_sort(std::vector<T>& inp, Cmp cmp) {
    std::vector<T> buffer(inp.size());
    auto mid = inp.begin();
    std::advance(mid, inp.size() / 2);
//    merge(inp.begin(), mid, mid, inp.end(), buffer.begin(), cmp);
    sort(inp.begin(), inp.end(), buffer.begin(), cmp);
    std::copy(buffer.begin(), buffer.end(), inp.begin());
}

template<typename T>
void merge_sort(std::vector<T>& inp) {
    merge_sort(inp, std::greater<T>());
}

int main() {
    size_t n;
    std::cin >> n;
    std::vector<MyClass> inp(n);
    for (MyClass& i: inp) {
        std::cin >> i;
    }
    merge_sort(inp);
    std::copy(inp.begin(), inp.end(), std::ostream_iterator<MyClass>(std::cout, " "));
    std::cout << '\n';
    return 0;
}
