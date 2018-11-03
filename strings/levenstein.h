#include <iostream>
#include <vector>
//Given two strings and weights of adding, removing and changing letter respectively. Returns levenstein_dist between string1 and string2

#pragma once

#include <string>


int64_t min(int64_t a, int64_t b, int64_t c) {
    return std::min(a, std::min(b, c));
}


int64_t levenstein_dist(const std::string& inp, const std::string& out, int w_add, int w_rm, int w_ch) {
    size_t ni(inp.size()), no(out.size());
    std::vector<int64_t> tmp(ni + 1);
    std::vector<std::vector<int64_t>> table(no + 1, tmp);
    for (size_t i = 0; i <= ni; ++i) {
        table[0][i] = i;
    }

    for (size_t i = 1; i <= no; ++i) {
        table[i][0] = i;
        for (size_t j = 1; j <= ni; ++j) {
            table[i][j] = min(table[i - 1][j - 1] + (inp[j - 1] != out[i - 1]) * w_ch, table[i - 1][j] + w_add, table[i][j - 1] + w_rm);
        }
    }
    
    return table[no][ni];
}
