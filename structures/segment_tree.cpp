#include <iostream>
#include <vector>
#include <utility>
#include <string>


void add_elems(size_t left, size_t right, std::vector<int64_t>& data) {
    
    size_t mid = (right + left) / 2;
    size_t tmp;
    for (size_t i = left; i <= mid; ++i) {
        tmp += data[i];
    }
    data.push_back()
}
void find_sum(std::vector<int64_t>& ans, std::vector<std::pair<char, std::pair<size_t, int64_t>>>& req, std::vector<int64_t>& data) {
    std::vector<int64_t> tmp(data.size() * 2);

    return;
}

int main(void) {
    using std::cin;
    using std::vector;
    using std::string;
    using std::pair;
    size_t n;
    cin >> n;
    vector<int64_t> data(n);
    for (size_t i = 0; i < n; ++i) {
        cin >> data[i];
    }
    size_t m;
    cin >> m;
    vector<pair<char, pair<size_t, int64_t>>> req(m);
    for (size_t i = 0; i < m; ++i) {
        cin >> req[i].first >> req[i].second.first >> req[i].second.second;
    }

    vector<int64_t> ans;
    find_sum(ans, req, data);
    return 0;
}
