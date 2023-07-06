#pragma once

#include <helpers/result.hpp>
#include <helpers/assert.hpp>

#include <vector>
#include <string>
#include <tuple>

namespace algo {

    /*
     * Populates provided vector with sizes of biggest palindromes centered in corresponding positions.
     * E.g. ret[i] == size of a biggest palindrome centered in i.
     * For even-sized palindromes, i - is left leaned center
     */
    void MaxPalindromes(std::ranges::random_access_range auto&& view, std::vector<size_t>* ret) noexcept {
        // https://e-maxx.ru/algo/palindromes_count
        static constexpr size_t npos = -1;

        size_t n = std::ranges::size(view);
        ret->resize(n);
        auto data = std::ranges::begin(view);

        auto left_bound_from_size = [&](size_t pos, size_t size) -> size_t {
            // return left boundary of sub_view, centered at position pos of size size
            // if such sub_view won't fit into original view, return npos
            ASSERT(size > 0);
            if (pos >= (size - 1) / 2 && pos + size / 2 < n) {
                return pos - (size - 1) / 2;
            } else {
                return npos;
            }
        };

        auto biggest_palindrome = [&](size_t pos, size_t initial_size) -> std::pair<size_t, size_t> {
            // try to find biggest palindrome centered at pos. Returns left boundary and size of palindrome
            // tries to do so by linearly increasing possible size, starting at initial_size
            ASSERT(initial_size > 0);
            size_t l = left_bound_from_size(pos, initial_size);
            size_t size = initial_size; // single word is always palindrome
            bool odd_size_possible = true;
            bool even_size_possible = true;
            while (true) {
                size_t tmp_l;
                size_t tmp_size;

                if (size % 2 == 0) {
                    if (odd_size_possible) {
                        tmp_size = size + 1;
                    } else {
                        tmp_size = size + 2;
                    }
                } else {
                    if (even_size_possible) {
                        tmp_size = size + 1;
                    } else {
                        tmp_size = size + 2;
                    }
                }

                tmp_l = left_bound_from_size(pos, tmp_size);
                if (tmp_l == npos) {
                    break;
                }

                if (data[tmp_l] != data[tmp_l + tmp_size - 1]) {
                    if (tmp_size % 2 == 0) {
                        even_size_possible = false;
                    } else {
                        odd_size_possible = false;
                    }
                }

                if (!even_size_possible && !odd_size_possible) {
                    break;
                }

                l = tmp_l;
                size = tmp_size;
            }
            return {l, size};
        };

        size_t l {0}, r{0}; // borders of right most palindrome
        (*ret)[0] = 1;
        for (size_t i = 1; i < n; ++i) {
            size_t size;
            if (i >= r) {
                std::tie(l, size) = biggest_palindrome(i, 1);
            } else {
                size_t possible_size = (*ret)[l + i - r]; // size of inversed palindrome
                if (i + possible_size / 2 >= r) {
                    std::tie(l, size) = biggest_palindrome(i, (r - i) * 2 + 1);
                } else {
                    size = possible_size;
                }
            }

            r = l + size - 1;
            (*ret)[i] = size;
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
            return std::make_error_condition(std::errc::not_enough_memory);
        }

        MaxPalindromes(std::forward<decltype(view)>(view), &ret);
        return std::move(ret);
    }


} // namespace algo