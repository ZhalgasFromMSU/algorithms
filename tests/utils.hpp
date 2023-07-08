#pragma once

#include <gtest/gtest.h>

#include <random>
#include <string>

namespace algo::testing {

    class Randomizer : public ::testing::Test {
    public:
        void SetSeed(size_t seed) {
            gen_.seed(seed);
        }

        template<std::integral T>
        T RandomInt(T min, T max) {
            return std::uniform_int_distribution<T>{min, max}(gen_);
        }

        std::string RandomString(size_t size, std::string_view chars = "") {
            if (chars.empty()) {
                chars = "abcdefghijklmnopqrstuvwxyz"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "1234567890";
            }

            std::string ret;
            ret.resize(size);

            std::uniform_int_distribution<size_t> distr {0, chars.size() - 1};
            for (size_t i = 0; i < size; ++i) {
                ret[i] = chars.at(distr(gen_));
            }
            return ret;
        }

    private:
        std::mt19937 gen_;
    };

} // namespace algo::testing
