//Gets vector and requests. (requests: s 1 5 or u 2 19 (sum from first to fifth
//or update second elem to 19))
//All indexing start from 1
#pragma once
#include <vector>
#include <iostream>
#include <utility>

using vec = std::vector<uint64_t>;
using vec_req = std::vector<std::pair<char, std::pair<uint64_t, uint64_t>>>;


uint64_t find_sum(int64_t l, int64_t r, const vec& tree) {
    uint64_t sum_to_l = 0, sum_to_r = 0;
    for (l = l - 1; l >= 0; l = (l & (l + 1)) - 1) {
        sum_to_l += tree[l];
    }
    for (; r >= 0; r = (r & (r + 1)) - 1) {
        sum_to_r += tree[r];
    }
    return sum_to_r - sum_to_l;
}

void update(size_t pos, uint64_t delta, vec& tree) {
    size_t n = tree.size();
    for (; pos < n; pos = pos | (pos + 1)) {
        tree[pos] += delta;
    }
}

void fenvick_tree_sum(vec& inp, const vec_req& req) {
    size_t n = inp.size(), m = req.size();
    vec tree(n, 0);
    for (size_t i = 0; i < n; ++i) {
        update(i, inp[i], tree);
    }
    for (size_t i = 0; i < m; ++i) {
        if (req[i].first == 's') {
            std::cout <<
                find_sum(static_cast<int64_t>(req[i].second.first - 1),
                static_cast<int64_t>(req[i].second.second - 1), tree) << ' ';
        } else {
            int64_t delta = req[i].second.second -
                inp[req[i].second.first - 1];
            inp[req[i].second.first - 1] = req[i].second.second;
            update(req[i].second.first - 1, delta, tree);
        }
    }
}
