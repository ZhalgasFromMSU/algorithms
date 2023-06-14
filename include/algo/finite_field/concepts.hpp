#pragma once

#include <concepts>

namespace algo {

    // Compile-time poly
    template<typename T>
    struct IsIntegral : std::false_type {

    };

    template<typename T>
    concept Integral = IsIntegral<T>::value || std::integral<T>;

    // Integral traits
    template<Integral T>
    struct IntegralTraits {
        static constexpr std::size_t kBitSize = sizeof(T) * 8;
    };

} // namespace algo
