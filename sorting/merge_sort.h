#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include <iterator>

template<typename Iter, typename BuffIter, typename Cmp>
void merge(Iter it1, Iter end1, Iter it2, Iter end2, BuffIter buff, Cmp cmp) {
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

template<typename Iter, typename BuffIter, typename Cmp>
void sort(Iter begin, Iter end, BuffIter buff, Cmp cmp) {
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

    /**
     *  @brief      Sorts elements of the structure. Takes additional O(n) memory
     *              and works for O(n * log(n))
     *  @params     Two forward iterators, and compare function`
    */
template<typename Iter, typename Cmp>
void merge_sort(Iter begin, Iter end, Cmp cmp) {
    //could use any buffer, but vector seems to be easiest.
    std::vector<int> buffer(std::distance(begin, end));
    sort(begin, end, buffer.begin(), cmp);
}

template<typename Iter>
void merge_sort(Iter begin, Iter end) {
    merge_sort(begin, end, std::greater<decltype(*begin)>());
}
