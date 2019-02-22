#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include <iterator>

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
    if (std::distance(begin, end) < 2) {
        return;
    }
    Iter mid = begin;
    std::advance(mid, std::distance(begin, end) / 2);
    sort(begin, mid, buff, cmp);
    sort(mid, end, buff, cmp);
    merge(begin, mid, mid, end, buff, cmp);
    for (auto i = begin; i != end; ++i) {
        *i = *buff;
        buff++;
    }
}

template<typename T, typename Cmp>
void merge_sort(std::vector<T>& inp, Cmp cmp) {
    std::vector<T> buffer(inp.size());
    auto mid = inp.begin();
    std::advance(mid, inp.size() / 2);
    sort(inp.begin(), inp.end(), buffer.begin(), cmp);
    std::copy(buffer.begin(), buffer.end(), inp.begin());
}

template<typename T>
void merge_sort(std::vector<T>& inp) {
    merge_sort(inp, std::greater<T>());
}
