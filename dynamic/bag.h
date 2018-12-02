//vector inp is vector of pairs, where first is mass and second is its const
//m - max mass
//Counts max value of bag given that the mass of bag is lesser than m
//Example of main is given
#pragma once
#include <iostream>
#include <vector>
#include <utility>

using vec = std::vector<std::pair<uint64_t, uint64_t>>;
using vec_row = std::vector<uint64_t>;


uint64_t find_cost(uint64_t m, const vec& inp) {
    size_t n = inp.size();
    std::vector<vec_row> ans(n + 1, vec_row(m + 1, 0));
    for (size_t i = 1; i <= n; ++i) {
        for (size_t j = 1; j <= m; ++j) {
            if (j < inp[i - 1].first) {
                ans[i][j] = ans[i - 1][j];
            } else {
                ans[i][j] = std::max(
                    ans[i - 1][j],
                    ans[i - 1][j - inp[i - 1].first] + inp[i - 1].second
                );
            }
        }
    }
    return ans[n][m];
}
/*
int main() {
    size_t n;
    uint64_t m;
    std::cin >> n >> m;
    vec inp(n);
    for (size_t i = 0; i < n; ++i) {
        std::cin >> inp[i].first;
    }
    for (size_t i = 0; i < n; ++i) {
        std::cin >> inp[i].second;
    }

    std::cout << find_cost(m, inp) << '\n';
    return 0;
}
*/
