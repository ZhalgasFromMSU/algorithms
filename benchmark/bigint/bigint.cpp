#include <BigInt.hpp>
#include <algo/bigint.hpp>
#include <exception>

#ifndef NCRYPTOPP
#include <cryptopp/integer.h>
#endif

#include <benchmark/benchmark.h>

#include <random>

template <typename T> struct BigIntFactory {
  using Type = T;

  static T New(uint64_t val) noexcept { return T(val); }

  static T New(std::string &&s) noexcept {
#ifndef NCRYPTOPP
    if constexpr (std::same_as<T, CryptoPP::Integer>) {
      return T(s.c_str());
    }
#endif

    return T(std::move(s));
  }
};

template <typename Factory> static void BM_LongMul(benchmark::State &state) {
  using BigInt = Factory::Type;
  const std::size_t len = 10'000;
  std::string lhs_str, rhs_str;
  lhs_str.resize(len);
  rhs_str.resize(len);

  std::default_random_engine e{0};
  std::uniform_int_distribution<char> dist('0', '9');

  lhs_str[0] = std::uniform_int_distribution<char>('1', '9')(e);
  rhs_str[0] = std::uniform_int_distribution<char>('1', '9')(e);

  for (std::size_t i = 1; i < len; ++i) {
    lhs_str[i] = dist(e);
    rhs_str[i] = dist(e);
  }

  BigInt lhs = Factory::New(std::move(lhs_str));
  BigInt rhs = Factory::New(std::move(rhs_str));

  for (auto _ : state) {
    BigInt mul = lhs * rhs;

    if (mul != rhs * lhs) {
      std::terminate();
    }

    if (mul / rhs != lhs) {
      std::terminate();
    }

    if (mul % lhs != 0) {
      std::terminate();
    }
  }
}

template <typename Factory> static void BM_Fermat(benchmark::State &state) {
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

#ifndef NCRYPTOPP
BENCHMARK(BM_Fermat<BigIntFactory<CryptoPP::Integer>>); // CryptoPP
BENCHMARK(BM_LongMul<BigIntFactory<CryptoPP::Integer>>);
#endif

BENCHMARK(BM_LongMul<BigIntFactory<algo::BigInt<2100>>>);

BENCHMARK(BM_Fermat<BigIntFactory<algo::BigInt<8, uint8_t, uint16_t>>>);
BENCHMARK(BM_Fermat<BigIntFactory<algo::BigInt<2, uint32_t, uint64_t>>>);
BENCHMARK(BM_Fermat<BigIntFactory<uint64_t>>);
BENCHMARK(BM_Fermat<BigIntFactory<BigInt>>); // faheel bigint
