//Gets vector of inputs and vector of changes (addition on the segment)
#pragma once
#include <iostream>
#include <vector>
#include <utility>


using vec = std::vector<uint64_t>;
using pair_int = std::pair<uint64_t, uint64_t>;
using pair_req = std::pair<char, uint64_t>;
using vec_tree = std::vector<pair_int>;
using vec_req = std::vector<std::pair<pair_req, pair_int>>;


void make_tree(size_t cur, size_t l, size_t r, vec_tree& tree, const vec& inp) {
    tree[cur].second = 0;
    if (l == r) {
        tree[cur].first = inp[l];
        return;
    }
    size_t tmp = (l + r) / 2;
    make_tree(cur * 2 + 1, l, tmp, tree, inp);
    make_tree(cur * 2 + 2, tmp + 1, r, tree, inp);
    tree[cur].first = std::max(
        tree[cur * 2 + 1].first,
        tree[cur * 2 + 2].first
    );
}

void update(size_t cur, size_t l, size_t r,
        size_t ll, size_t rr, uint64_t delta,
        vec_tree& tree) {
    if (l > r) {
        return;
    }
    if (l == ll && r == rr) {
        tree[cur].second += delta;
        size_t tmp = cur;
        while (tmp != 0) {
            if (tree[tmp].first + tree[tmp].second > tree[(tmp - 1) / 2].first) {
                tree[(tmp - 1) / 2].first = tree[tmp].first + tree[tmp].second;
            }
            tmp = (tmp - 1) / 2;
        }
        return;
    }
    size_t tmp = (l + r) / 2;
    tree[cur].first += tree[cur].second;
    tree[cur * 2 + 2].second += tree[cur].second;
    tree[cur * 2 + 1].second += tree[cur].second;
    tree[cur].second = 0;
    if (ll > tmp) {
        update(cur * 2 + 2, tmp + 1, r, ll, rr, delta, tree);
    } else if (rr <= tmp) {
        update(cur * 2 + 1, l, tmp, ll, rr, delta, tree);
    } else {
        update(cur * 2 + 1, l, tmp, ll, tmp, delta, tree);
        update(cur * 2 + 2, tmp + 1, r, tmp + 1, rr, delta, tree);
    }
}

uint64_t get_max(size_t cur, size_t l, size_t r,
        size_t ll, size_t rr, uint64_t sum, const vec_tree& tree) {
    sum += tree[cur].second;
    if (l == ll && r == rr) {
        return tree[cur].first + sum;
    }
    size_t tmp = (l + r) / 2;
    if (rr <= tmp) {
        return get_max(cur * 2 + 1, l, tmp, ll, rr, sum, tree);
    } else if (ll > tmp) {
        return get_max(cur * 2 + 2, tmp + 1, r, ll, rr, sum, tree);
    }
    return std::max(
        get_max(cur * 2 + 1, l, tmp, ll, tmp, sum, tree),
        get_max(cur * 2 + 2, tmp + 1, r, tmp + 1, rr, sum, tree)
    );
}
void segment_tree_max(const vec& inp, const vec_req& req) {
    size_t n = inp.size(), m = req.size();
    vec_tree tree(4 * n);
    make_tree(0, 0, n - 1, tree, inp);
    for (size_t i = 0; i < m; ++i) {
        if (req[i].first.first == 'm') {
            std::cout <<
                get_max(0, 0, n - 1,
                    req[i].second.first - 1, req[i].second.second - 1, 0,
                        tree) << ' ';
        } else {
            update(0, 0, n - 1,
                req[i].second.first - 1, req[i].second.second - 1,
                    req[i].first.second, tree);
        }
    }
}
