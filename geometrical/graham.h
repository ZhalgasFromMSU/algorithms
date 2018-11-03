//Set of dots is given. Builds simple convex hull. Clears input vector and fills with convex hull

#pragma once
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>



bool clockwise(const std::pair<int, int>& d1, const std::pair<int, int>& d2, const std::pair<int, int>& d3) {
    return d1.first * (d2.second - d3.second) + d2.first * (d3.second - d1.second) + d3.first * (d1.second - d2.second) < 0;
}

bool anti_clockwise(const std::pair<int, int>& d1, const std::pair<int, int>& d2, const std::pair<int, int>& d3) {
    return d1.first * (d2.second - d3.second) + d2.first * (d3.second - d1.second) + d3.first * (d1.second - d2.second) > 0;
}


void delete_twins(std::vector<std::pair<int, int>>& dots) {
    std::vector<std::pair<int, int>> tmp;
    tmp.push_back(dots[0]);
    for (size_t i = 1; i < dots.size(); ++i) {
        if (dots[i] != dots[i - 1]) {
            tmp.push_back(dots[i]);
        }
    }
    dots.clear();
    dots = tmp;
}

void convex_hull(std::vector<std::pair<int, int>>& dots) {
    using std::vector;
    using std::pair;
    std::sort(dots.begin(), dots.end());
    delete_twins(dots);
    size_t n = dots.size();
    if (n < 2) {
        return;
    }
    
    vector<pair<int, int>> top, bot;
    top.push_back(dots[0]);
    bot.push_back(dots[0]);
    for (size_t i = 1; i < n; ++i) {
        if (i == n - 1 || clockwise(dots[0], dots[i], dots[n - 1])) {
            int tmp = top.size();
            while (tmp >= 2 && !clockwise(top[tmp - 2], top[tmp - 1], dots[i])) {
                top.pop_back();
                tmp = top.size();
            }
            top.push_back(dots[i]);
        }

        if (i == n - 1 || anti_clockwise(dots[0], dots[i], dots[n - 1])) {
            int tmp = bot.size();
            while (tmp >= 2 && !anti_clockwise(bot[tmp - 2], bot[tmp - 1], dots[i])) {
                bot.pop_back();
                tmp = bot.size();
            }
            bot.push_back(dots[i]);
        }
    }

    dots.clear();
    top.insert(top.end(), bot.rbegin() + 1, bot.rend() - 1);
    dots = top;
}
