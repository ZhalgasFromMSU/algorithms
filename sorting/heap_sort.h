//Vector of ints is given
#pragma once

#include <iostream>
#include <vector>


//Просеивание
void push_down(int cur, int n, std::vector<int>& inp) {
    int tmp = inp[cur];
    int child;
    while (cur <= n / 2) {
        child = 2 * cur;
        if (child < n && inp[child] < inp[child + 1]) {
            child++;
        }
        if (tmp >= inp[child]) {
            break;
        } else {
            inp[cur] = inp[child];
        }
        cur = child;
    }
    inp[cur] = tmp;
}

//Сортировка
template<typename Iter, typename Cmp>
void heap_sort(Iter begin, Iter end, Cmp cmp) {
    size_t n = std::distance(begin, end);
    for (size_t i = n / 2;)
}
void heap_sort(std::vector<int>& in) {
    int n = inp.size();

    for (int i = n / 2; i >= 0; --i) {
        push_down(i, n - 1, inp);
    }

    for (int i = n - 1; i > 0; --i) {
        std::swap(inp[0], inp[i]);
        push_down(0, i - 1, inp);
    }
}

template<typename Iter>
void heap_sort(Iter begin, Iter end) {
    heap_sort(begin, end, std::greater<decltype(*begin)>());
}
