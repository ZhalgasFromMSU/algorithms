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
void heap_sort(std::vector<int>& inp) {
    int n = inp.size();
    
    for (int i = n / 2; i >= 0; --i) {
        push_down(i, n - 1, inp);
    }

    for (int i = n - 1; i > 0; --i) {
        std::swap(inp[0], inp[i]);
        push_down(0, i - 1, inp);
    }
}
