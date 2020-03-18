#include <iostream>
#include <vector>
#include <string>
#include <variant>


std::vector<int> prefixFunc(const std::string& str) {
    std::vector<int> ans(str.size());
    for (int i = 1; i < str.size(); ++i) {
        int j = ans[i - 1];
        while (j > 0 && str[i] != str[j]) {
            j = ans[j - 1];
        }
        if (str[j] == str[i]) {
            ans[i] = j + 1;
        }
    }
    return ans;
}


std::vector<int> findSubstr(const std::string& pat, const std::string& text) {
    std::vector<int> ret;
    auto vec = prefixFunc(pat + char(0) + text);
    for (int i = pat.size() + 1; i < vec.size(); ++i) {
        if (vec[i] == pat.size()) {
            ret.push_back(i - pat.size() - pat.size());
        }
    }
    return ret;
}
