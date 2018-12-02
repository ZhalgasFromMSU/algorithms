#include <iostream>
#include <unordered_set>
#include <string>
#include <vector>


int main() {
    size_t n, count(0);
    std::unordered_set<std::string> all, curr;
    std::vector<std::string> comp;
    std::cin >> n;
    for (size_t i = 0; i < n; ++i) {
        size_t k;
        std::cin >> k;
        std::vector<std::string> curr_comp(k);
        int8_t flag = 0;
        for (size_t j = 0; j < k; ++j) {
            std::cin >> curr_comp[j];
            curr.insert(curr_comp[j]);
        }
        if (comp.empty()) {
            flag = 1;
        }
        for (auto& s: comp) {
            if (curr.find(s) == curr.end()) {
                flag = 1;
                break;
            }
        }
        //Множество 
        if (flag == 1) {

        }
        if (flag) {
            ++count;
            for (auto& s: comp) {
                all.erase(s);
            }
            comp.resize(k);
            comp.assign(curr_comp.begin(), curr_comp.end());
            for (auto& s: curr_comp) {
                curr.erase(s);
            }
        } else {
            for (auto& s: curr_comp) {
                curr.erase(s);
                if (all.find(s) == all.end()) {
                    all.insert(s);
                    comp.push_back(s);
                }
            }
        }
    }
    std::cout << count << '\n';
    return 0;
}
