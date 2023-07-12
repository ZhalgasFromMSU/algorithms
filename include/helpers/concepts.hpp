#pragma once

#include <concepts>
#include <ranges>

namespace algo {

    /*
     * T is a range that points to U
     */
    template<typename T, typename U>
    concept Range = std::ranges::range<T>
                    && !std::is_rvalue_reference_v<T>
                    && std::same_as<std::iter_value_t<decltype(std::ranges::begin(std::declval<T&>()))>, U>;

} // namespace algo
