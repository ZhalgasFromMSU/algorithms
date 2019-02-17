#pragma once
#include <iostream>
#include <vector>
#include <list>

template<typename T>
using vec = std::vector<T>;

enum class CLR {
    BLCK,
    WHTE,
    GREY
};

bool dfs(size_t cur, vec<CLR>& clr, const vec<vec<bool>>& graph, size_t& mark,
          std::list<size_t>& ans) {
    clr[cur] = CLR::GREY;
    size_t n = graph.size();
    for (size_t i = 0; i < n; ++i) {
        if (graph[cur][i] == true && clr[i] == CLR::WHTE) {
            if (dfs(i, clr, graph, mark, ans)) {
                if (mark != i && mark != static_cast<size_t>(-1)) {
                    ans.push_front(i + 1);
                } else {
                    mark = static_cast<size_t>(-1);
                }
                return true;
            }
        } else if (graph[cur][i] == true && clr[i] == CLR::GREY) {
            mark = i;
            ans.push_front(i + 1);
            return true;
        }
    }
    clr[cur] = CLR::BLCK;
    return false;
}

    /**
     *  @brief      checks if oriented graph, given with adjacency matrix,
     *              has cycles.
     *  @return     returns true if and only if thereis a cycle
     *  @params     graph - n x n adjacency matrix with bool values
     *              ans - list where the cycle will be stored if any found
     *                  numeration starts from 1
    */
bool find_cycle(const vec<vec<bool>>& graph, std::list<size_t>& ans) {
    size_t n(graph.size());
    vec<CLR> clr(n, CLR::WHTE);
    size_t mark = static_cast<size_t>(-1);
    for (size_t i = 0; i < n; ++i) {
        if (clr[i] == CLR::BLCK) {
            continue;
        } else if (dfs(i, clr, graph, mark, ans)) {
            return true;
        }
    }
    return false;
}
