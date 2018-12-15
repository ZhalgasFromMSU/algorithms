//Fastest Dijkstra (working for set of dots on plane) I could come up with
//Was written long ago
//Finds shortest distance between two given dots
//Works for n * n * log(n)
#pragma once
#include <iostream>
#include <set>
#include <vector>
#include <cmath>
#include <map>

using std::map;
using std::min;
using std::cin;
using std::cout;
using std::endl;
using std::vector;


const double inf = 2000000000.0;

struct dot {
    double x, y;
};


double dist(int a, int b, vector<map<int, double>>& cost, const vector<dot>& graph) {
    if (cost[a].find(b) != cost[a].end()) {
        return cost[a][b];
    }
    return sqrt((graph[a].x - graph[b].x) * (graph[a].x - graph[b].x) + (graph[a].y - graph[b].y) * (graph[a].y - graph[b].y));
}


void deikstra(unsigned int n, int a, int b, vector<double>& ans, vector<map<int, double>>& cost, const vector<dot>& graph) {
    ans[b] = dist(a, b, cost, graph);
    vector<bool> used(n, false);
    ans[a] = 0;
    double tmp = inf - 1;
    for (unsigned int i = 0; i < n; i++) {
        int curr = -1;
        for (unsigned int j = 0; j < n; j++) {
            if (!used[j] && (curr == -1 || ans[j] < ans[curr])) {
                curr = j;
            }
        }
        if (ans[curr] > tmp) {
            break;
        }
        used[curr] = true;
        if (ans[curr] >= ans[b]) {
            continue;
        }
        for (unsigned int j = 0; j < n; j++) {
            double tmp2 = dist(curr, j, cost, graph);
            ans[j] = min(ans[curr] + tmp2, ans[j]);
        }
    }
}

/*
int main(void) {
    unsigned int n;
    cin >> n;
    vector<dot> graph(n);
    for (unsigned int i = 0; i < n; i++) {
        cin >> graph[i].x >> graph[i].y;
    }
    unsigned int m;
    cin >> m;
    vector<map<int, double>> cost(n);
    for (unsigned int i = 0; i < m; i++) {
        int v1, v2, a;
        cin >> v1 >> v2 >> a;
        cost[v1 - 1][v2 - 1] = a;
        cost[v2 - 1][v1 - 1] = a;
    }
    int a, b;
    cin >> a >> b;
    vector<double> ans(n, inf);
    deikstra(n, a - 1, b - 1, ans, cost, graph);
    cout << ans[b - 1] << endl;
    return 0;
}
*/
