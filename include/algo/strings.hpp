#pragma once

#include <helpers/result.hpp>

#include <vector>
#include <string>

namespace algo {

    /*
     * Populates provided vector with sizes of biggest palindromes centered in corresponding positions.
     * E.g. ret[i] == size of a biggest palindrome centered in i.
     * For even-sized palindromes, i - is left leaned center
     */
    void MaxPalindromes(std::ranges::random_access_range auto&& view, std::vector<size_t>* ret) noexcept {
        // https://e-maxx.ru/algo/palindromes_count
        ret->resize(std::ranges::size(view));

        auto data = std::ranges::begin(view);
        size_t l {0}, r{0}; // borders of right most palindrome
        size_t n = ret->size();

        auto biggest_palindrome = [&](size_t pos, size_t initial_size) -> size_t {
            size_t size {0};
            for (size_t j = initial_size; j <= pos && pos + j < n && data[pos - j] == data[pos + j]; ++j) {

            }
        };

        for (size_t i = 0; i < n; ++i) {
            if (i >= r) {
            }
        }
    }

    /*
     * Return vector, with i-th element being size of a biggest palindrome centered in i-th element.
     * For even-sized palindromes, i - is left leaned center
     *
     * Returns error if allocation of storage for resulting vector failed
     */
    Result<std::vector<size_t>> MaxPalindromes(std::ranges::random_access_range auto&& view) noexcept {
        std::vector<size_t> ret;

        try {
            ret.reserve(std::ranges::size(view));
        } catch (const std::bad_alloc&) {
            return std::errc::not_enough_memory;
        }

        MaxPalindromes(std::forward<decltype(view)>(view), &ret);
        return std::move(ret);
    }


} // namespace algo
