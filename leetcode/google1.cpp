#include <algorithm>
#include <vector>
#include <iostream>

using vec = std::vector<uint64_t>;
using pair_int = std::pair<uint64_t, uint64_t>;
using pair_req = std::pair<char, uint64_t>;
using vec_tree = std::vector<pair_int>;
using vec_req = std::vector<std::pair<pair_req, pair_int>>;

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
int solution(vector<int> &S, vector<int> &E) {
    vec_tree tree(4001, std::make_pair(0, 0));
    size_t n = S.size();
    size_t max = 1000;
    for (size_t i = 0; i < n; ++i) {
        update(0, 0, max , S[i] - 1, E[i] - 1, 1, tree);
    }
    return get_max(0, 0, max, 0, max, 0, tree);
}

int main() {
    return 0;
}
