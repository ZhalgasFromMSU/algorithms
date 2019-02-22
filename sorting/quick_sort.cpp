#include <iostream>
#include <iterator>
#include <functional>
#include <vector>

template<typename Iter, typename Cmp>
void split(Iter begin, Iter end, Cmp cmp) {
    
}

template<typename Iter, typename Cmp>
void quick_sort(Iter begin, Iter end, Cmp cmp) {
    sort()
    return;
}

template<typename Iter>
void quick_sort(Iter begin, Iter end) {
    quick_sort(begin, end, std::greater<decltype(*begin)>());
}

int main() {
    size_t n;
    std::cin >> n;
    std::vector<uint32_t> inp(n);
    for (uint32_t& i: inp) {
        std::cin >> i;
    }
    quick_sort(inp.begin(), inp.end());
    return 0;
}
