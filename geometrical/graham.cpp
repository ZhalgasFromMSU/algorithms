//Set of dots is given. Builds simple convex hull

#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>

using std::vector;
using std::pair;


bool clockwise(const pair<int, int>& d1, const pair<int, int>& d2, const pair<int, int>& d3) {
    return d1.first * (d2.second - d3.second) + d2.first * (d3.second - d1.second) + d3.first * (d1.second - d2.second) < 0;
}

bool anti_clockwise(const pair<int, int>& d1, const pair<int, int>& d2, const pair<int, int>& d3) {
    return d1.first * (d2.second - d3.second) + d2.first * (d3.second - d1.second) + d3.first * (d1.second - d2.second) > 0;
}


void delete_twins(vector<pair<int, int>>& dots) {
    vector<pair<int, int>> tmp;
    tmp.push_back(dots[0]);
    for (size_t i = 1; i < dots.size(); ++i) {
        if (dots[i] != dots[i - 1]) {
            tmp.push_back(dots[i]);
        }
    }
    dots.clear();
    dots = tmp;
}

void convex_hull(vector<pair<int, int>>& top, vector<pair<int, int>>& bot, vector<pair<int, int>>& dots) {
    size_t n = dots.size();
    std::sort(dots.begin(), dots.end());
    
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
}

void print(const vector<pair<int, int>>& v1, const vector<pair<int, int>>& v2) {
    for (size_t i = 0; i < v1.size(); ++i) {
        std::cout << v1[i].first << ' ' << v1[i].second << '\n';
    }
    for (size_t i = v2.size() - 2; i > 0; --i) {
        std::cout << v2[i].first << ' ' << v2[i].second << '\n';
    }
}

int main(void) {
    size_t n;
    std::cin >> n;
    vector<pair<int, int>> dots(n);
    for (size_t i = 0; i < n; ++i) {
        std::cin >> dots[i].first >> dots[i].second;
    }
    delete_twins(dots);
    if (dots.size() == 1) {
        std::cout << 1 << '\n';
        std::cout << dots[0].first << ' ' << dots[0].second << '\n';
        return 0;
    }

    vector<pair<int, int>> top, bot;
    convex_hull(top, bot, dots);
    std::cout << top.size() + bot.size() - 2 << '\n';
    
    if (n % 2 == 1) {
        print(top, bot);
    } else {
        print(bot, top);
    }
    return 0;
}
