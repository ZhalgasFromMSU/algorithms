//Set of dots is given. Builds simple convex hull

#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>

using std::vector;
using std::pair;
using std::make_pair;


bool check_above(const pair<int, int>& dot, const pair<int, int>& l, const pair<int, int>& r) {
    return dot.second > l.first + static_cast<double>(r.second - l.second) / (r.first - l.first) * dot.first;
}

bool clock_wise(const pair<int, int>& d1, const pair<int, int>& d2, const pair<int, int>& d3) {
    return d1.first * (d2.second - d3.second) + d2.first * (d3.second - d1.second) + d3.first * (d1.second - d2.second) < 0;
}

bool anti_clock_wise(const pair<int, int>& d1, const pair<int, int>& d2, const pair<int, int>& d3) {
    return d1.first * (d2.second - d3.second) + d2.first * (d3.second - d1.second) + d3.first * (d1.second - d2.second) > 0;
}


//Функция удаляет элементы из вектора и оставляет только точки, вошедшие в выпуклую оболочку
void convex_hull(vector<pair<int, int>>& inp) {
    
}

void print(vector<pair<int, int>> s) {
    std::cout << "_____________________" << '\n';
    for (int i = 0; i < s.size(); ++i) {
        std::cout << s[i].first << '\t' << s[i].second << '\n';
    }
    std::cout << "_____________________" << '\n';
}

int main(void) {
    int n;
    std::cin >> n;
    vector<pair<int, int>> dots(n);
    for (int i = 0; i < n; ++i) {
        std::cin >> dots[i].first >> dots[i].second;
    }

    std::sort(dots.begin(), dots.end());
    print(dots);
    return 0;
}
