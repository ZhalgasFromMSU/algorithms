#include <iostream>
#include <vector>
#include <utility>
#include <set>


using std::set;
using std::pair;
using std::make_pair;
using std::vector;
using std::cin;
using std::cout;
using std::endl;


long long int inf = 2000000000000000000;


void dijkstra(int a, int b, const vector<vector<pair<long long int, int>>>& edge) {
    vector<long long int> ans(edge.size(), inf);
    ans[a] = 0;
    set<pair<long long int, int>> p;
    p.insert(make_pair(0, a));
    while(!p.empty()) {
        int from = (*p.begin()).second;
        long long int dist = (*p.begin()).first;
        p.erase(p.begin());
        if (ans[from] == inf || ans[from] >= ans[b]) {
            break;
        }
        if (dist > ans[from]) {
            continue;
        }
        for (size_t i = 0; i < edge[from].size(); i++) {
            if (ans[from] + edge[from][i].first < ans[edge[from][i].second]) {
                ans[edge[from][i].second] = ans[from] + edge[from][i].first;
                p.insert(make_pair(ans[edge[from][i].second], edge[from][i].second));
            }
        }
    }
    if (ans[b] != inf)
        cout << ans[b] << endl;
    else
        cout << -1 << endl;
}


int main(void) {
    int n, m, a, b;
    cin >> n >> m;
    vector<vector<pair<long long int, int>>> edge(n);


    for (int i = 0; i < m; i++) {
        int v1, v2, cost;
        cin >> v1 >> v2 >> cost;
        edge[v1 - 1].push_back(make_pair(cost, v2 - 1));
        edge[v2 - 1].push_back(make_pair(cost, v1 - 1));
    }
    cin >> a >> b;
    

    dijkstra(a - 1, b - 1, edge);
    return 0;
}
