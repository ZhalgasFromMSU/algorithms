#include <BigInt.hpp>
#include <algo/bigint.hpp>

#include <benchmark/benchmark.h>

template<typename T>
struct BigIntFactory {
  using Type = T;

  static T New(uint64_t val) noexcept {
    return T(val);
  }
};

template<typename Factory>
static void BM_BigInt(benchmark::State& state) {
  using BigInt = Factory::Type;
  uint64_t big_prime = (1ull << 19) - 1;
  BigInt big_prime_bi = Factory::New(big_prime);
  for (auto _ : state) {
    for (uint64_t i : {2, 3, 6, 10}) {
      BigInt base = Factory::New(i);

      BigInt power = Factory::New(1);
      for (uint64_t i = 0; i < big_prime - 1; ++i) {
        power *= base;
        if (power > big_prime) {
          power %= big_prime_bi;
        }
      }

      if (power != Factory::New(1)) {
        std::terminate();
      }
    }
  }
}

BENCHMARK(BM_BigInt<BigIntFactory<algo::BigInt<8, uint8_t, uint16_t>>>);
BENCHMARK(BM_BigInt<BigIntFactory<algo::BigInt<2, uint32_t, uint64_t>>>);
BENCHMARK(BM_BigInt<BigIntFactory<uint64_t>>);
BENCHMARK(BM_BigInt<BigIntFactory<BigInt>>); // faheel bigint
