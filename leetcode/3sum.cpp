//https://leetcode.com/problems/3sum/
#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

using namespace std;

vector<vector<int>> threeSum(vector<int>& nums) {
    if (nums.size() < 3) {
        return vector<vector<int>>(0);
    }
    unordered_set<int> tmp;
    vector<vector<int>> ans;
    sort(nums.begin(), nums.end());
    for (size_t i = 0; i < nums.size() - 2; ++i) {
        if (i && nums[i] == nums[i - 1]) {
            continue;
        }
        for (size_t j = i + 1; j < nums.size(); ++j) {
            auto it = tmp.find(-nums[i] - nums[j]);
            if (it == tmp.end()) {
                tmp.insert(nums[j]);
            } else {
                ans.emplace_back(vector<int>{nums[i], *it, nums[j]});
                tmp.erase(it);
            }
        }
        tmp.clear();
    }
    return ans;
}

int main() {
    vector<int> nums = {0,2,2,3,0,1,2,3,-1,-4,2};
    auto ans = threeSum(nums);
    for (auto& i: ans) {
        for (auto j: i) {
            std::cout << j << '\t';
        }
        std::cout << '\n';
    }
    return 0;
}
