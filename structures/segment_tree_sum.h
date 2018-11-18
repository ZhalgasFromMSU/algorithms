//Vector of numbers is given, vector of requests is given. On each requests replies with either sum or update
//ex: s 1 5 (find sum from first to fifth elem in array (indexing start from 1))
//u 2 100 (change second elem to 100)
// {everything else} (print tree)

#include <iostream>
#include <vector>
#include <utility>
#include <string>

using vec = std::vector<int64_t>;
using vec_req = std::vector<std::pair<std::string, std::pair<int64_t, int64_t>>>;


void make_tree(size_t cur, size_t l, size_t r, vec& tree, const vec& inp) {
    if (l == r) {
        tree[cur] = inp[l];
        return;
    }
    size_t m = (l + r) / 2;
    make_tree(cur * 2 + 1, l, m, tree, inp);
    make_tree(cur * 2 + 2, m + 1, r, tree, inp);
    tree[cur] = tree[cur * 2 + 1] + tree[cur * 2 + 2];
}


int64_t find_sum(size_t cur, size_t l, size_t r,
        size_t ll, size_t rr, const vec& tree) {
    if (l == ll && r == rr) {
        return tree[cur];
    }
    size_t m = (ll + rr) / 2;
    if (r <= m) {
        return find_sum(cur * 2 + 1, l, r, ll, m, tree);
    } else if (l > m) {
        return find_sum(cur * 2 + 2, l, r, m + 1, rr, tree);
    }
    return find_sum(cur * 2 + 1, l, m, ll, m, tree) +
        find_sum(cur * 2 + 2, m + 1, r, m + 1, rr, tree);
}


void update(size_t cur, size_t ll, size_t rr, size_t ind, int64_t val,
        vec& tree) {
    if (ll == rr) {
        tree[cur] = val;
        return;
    }
    size_t tmp = (ll + rr) / 2;
    if (ind > tmp) {
        update(cur * 2 + 2, tmp + 1, rr, ind, val, tree);
    } else {
        update(cur * 2 + 1, ll, tmp, ind, val, tree);
    }
    tree[cur] = tree[cur * 2 + 1] + tree[cur * 2 + 2];
}


void print(const vec& tree) {
    for (size_t i = 0; i < tree.size(); ++i) {
        std::cout << tree[i] << ' ';
    }
    std::cout << '\n';
}


void segment_tree_sum(const vec& inp, const vec_req& req) {
    size_t n = inp.size(), m = req.size();
    vec tree(4 * n);
    make_tree(0, 0, n - 1, tree, inp);
    for (size_t i = 0; i < m; ++i) {
        if (req[i].first == "s") {
            std::cout << find_sum(0, req[i].second.first - 1,
                req[i].second.second - 1, 0, n - 1, tree) << ' ';
        } else if (req[i].first == "u") {
            update(0, 0, n - 1, req[i].second.first - 1,
                req[i].second.second, tree);
        } else {
            print(tree);
        }
    }
    std::cout << '\n';
}
