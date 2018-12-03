//Builds magical square (horizontal and vericals) of a given MGSQ_SIZE edge size
#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

using vec = std::vector<uint64_t>;

const size_t MGSQ_SIZE = 4;
size_t count = 0;

uint64_t sum_row(size_t i, const vec& out) {
    return std::accumulate(out.begin() + i * MGSQ_SIZE,
            out.begin() + (i + 1) * MGSQ_SIZE, 0);
}

uint64_t sum_col(size_t j, const vec& out) {
    uint64_t sum(0);
    for (size_t i = 0; i < MGSQ_SIZE; ++i) {
        sum += out[i * MGSQ_SIZE + j];
    }
    return sum;
}

bool build(size_t i, size_t j, uint64_t sum,
        vec& out, std::vector<bool>& used, const vec& inp) {
    if (i == MGSQ_SIZE) {
        for (size_t k = 0; k < MGSQ_SIZE; ++k) {
            if (sum_col(k, out) != sum) {
                return false;
            }
        }
        return true;
    } else if (j == MGSQ_SIZE && sum_row(i, out) != sum) {
        return false;
    } else if (j == MGSQ_SIZE) {
        return build(i + 1, 0, sum, out, used, inp);
    }
    for (size_t k = 0; k < inp.size(); ++k) {
        if (!used[k]) {
            used[k] = true;
            out[i * MGSQ_SIZE + j] = inp[k];
            if (build(i, j + 1, sum, out, used, inp)) {
                return true;
            }
            used[k] = false;
        }
    }
    return false;
}

bool build_square(vec& out, const vec& inp) {
    std::vector<bool> used(inp.size(), false);
    uint64_t sum = std::accumulate(inp.begin(), inp.end(), 0);
    if (sum % MGSQ_SIZE) {
        return false;
    } else {
        return build(0, 0, sum / MGSQ_SIZE, out, used, inp);
    }
}
/*
int main() {
    vec inp(MGSQ_SIZE * MGSQ_SIZE);
    for (size_t i = 0; i < inp.size(); ++i) {
        std::cin >> inp[i];
    }
    vec out(inp.size());
    if (build_square(out, inp)) {
        for (auto s: out) {
            std::cout << s << '\t';
        }
    } else {
        std::cout << "NO SOLUTION\n";
    }
    return 0;
}
*/
