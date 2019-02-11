#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

int main() {
    size_t n;
    std::cin >> n;
    std::vector<std::set<std::string>> inp(n);
    std::vector<std::vector<size_t>> graph(n);
    for (size_t i = 0; i < n; ++i) {
        size_t k;
        std::cin >> k;
        std::string tmp;
        for (size_t j = 0; j < k; ++j) {
            std::cin >> tmp;
            inp[i].insert(tmp);
        }
        for (size_t j = 0; j < n; ++j) {
            if (i != j) {
                if (std::includes(inp[j].begin(), inp[j].end(),
                        inp[i].begin(), inp[i].end())) {
                    graph[j].push_back(i);
                }
                if (std::includes(inp[i].begin(), inp[i].end(),
                        inp[j].begin(), inp[j].end())) {
                    graph[i].push_back(j);
                }
            }
        }
    }
    for (auto i: graph) {
        for (auto j: i) {
            std::cout << j << '\t';
        }
        std::cout << '\n';
    }
    return 0;
}
