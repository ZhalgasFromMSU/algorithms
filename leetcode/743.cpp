#include <iostream>
#include <vector>
#include <set>
#include <utility>
#include <algorithm>

const int INF = 200000000;

int network_delay_time(std::vector<std::vector<int>>& times, int N, int K) {
    std::set<std::pair<int, int>> q;
    std::vector<int> dist(N, INF);
    std::sort(times.begin(), times.end(),
        [](const std::vector<int>& a, const std::vector<int>& b) {
            return a[0] < b[0];
        }
    );
    dist[K] = 0;
    q.insert(std::make_pair(K, 0));
    while (!q.empty()) {
        int from = q.begin()->first;
        int len = q.begin()->second;
        q.erase(q.begin());
        auto it = std::lower_bound(times.begin(), times.end(), from,
            [](const std::vector<int>& a, int from) {
                return a[0] < from;
            }
        );
        while ((*it)[0] == from) {
            if (dist[(*it)[1]] > dist[from] + )
        }
    }
}

int main() {
    size_t n;
    std::cin >> n;
    std::vector<std::vector<int>> times(n, std::vector<int>(3));
    for (size_t i = 0; i < n; ++i) {
        std::cin >> times[i][0] >> times[i][1] >> times[i][2];
    }
    network_delay_time(times, n, 3);
    return 0;
}
