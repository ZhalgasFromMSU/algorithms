#include <algo/sync/queue.hpp>

#include <benchmark/benchmark.h>

static void BM_String(benchmark::State& state) {
    for (auto _ : state) {
        std::string empty_string;
    }
}

BENCHMARK(BM_String);

