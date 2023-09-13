#pragma once

#include <algo/expected.hpp>
#include <algo/assert.hpp>

#include <type_traits>
#include <vector>
#include <string>
#include <tuple>
#include <iterator>

namespace algo {

    class String {
    public:
        /*
         * Return vector, consisting of sizes of biggest palindromes centered at corresponding position
         * e.g. ret[i] is a size of a biggest palindromes, centered at i
         * For even-sized palindromes, i is a left leaned center
         *
         * returns error if couldn't allocate resulting vector
         */
        static Expected<std::vector<size_t>> MaxPalindromes(std::ranges::random_access_range auto&& view) noexcept;

        /*
         * Return vector, depicting Z-function of a range:
         * ret[idx] == size of a biggest prefix of a view, that is also a prefix of sub_view starting at idx
         */
        static Expected<std::vector<size_t>> ZFunc(std::ranges::random_access_range auto&& view) noexcept;
    };

    // Implementation
    Expected<std::vector<size_t>> String::MaxPalindromes(std::ranges::random_access_range auto&& view) noexcept {
        const size_t n = std::ranges::size(view);

        std::vector<size_t> sizes;
        try {
            sizes.resize(n * 2);
        } catch (const std::bad_alloc&) {
            return std::make_error_condition(std::errc::not_enough_memory);
        }

        {
            // odd sized palindromes
            auto inc = [&](size_t i, size_t* k) {
                do {
                    *k += 1;
                } while (i + *k < n && i >= *k && view[i + *k] == view[i - *k]);
                *k -= 1;
            };

            size_t l {0}, r {0};
            for (size_t i = 0; i < n; ++i) {
                if (i >= r) {
                    size_t k = 0;
                    inc(i, &k);

                    sizes[i] = 2 * k + 1;
                    l = i - k;
                    r = i + k;
                } else if (size_t rev_size = sizes[l + r - i]; i + rev_size / 2 >= r) {
                    size_t k = r - i;
                    inc(i, &k);

                    sizes[i] = 2 * k + 1;
                    l = i - k;
                    r = i + k;
                } else {
                    sizes[i] = rev_size;
                }
            }
        }

        {
            // even sized palindromes
            auto inc = [&](size_t i, size_t* k) {
                do {
                    *k += 1;
                } while (i + *k < 2 * n && i + 1 - n >= *k && view[i + *k - n] == view[i + 1 - *k - n]);
                *k -= 1;
            };

            size_t l {n}, r {n};
            for (size_t i = n; i < 2 * n; ++i) {
                if (i + 1 >= r) {
                    size_t k = 0;
                    inc(i, &k);

                    if (k != 0) {
                        sizes[i] = 2 * k;
                        l = i + 1 - k;
                        r = i + k;
                    }
                } else if (size_t rev_size = sizes[l + r - i - 1]; i + rev_size / 2 >= r) {
                    size_t k = r - i;
                    inc(i, &k);

                    if (k != 0) {
                        sizes[i] = 2 * k;
                        l = i + 1 - k;
                        r = i + k;
                    }
                } else {
                    sizes[i] = rev_size;
                }
            }
        }

        for (size_t i = 0; i < n; ++i) {
            if (sizes[i + n] > sizes[i]) {
                sizes[i] = sizes[i + n];
            }
        }
        sizes.resize(n);
        return std::move(sizes);
    }

    Expected<std::vector<size_t>> String::ZFunc(std::ranges::random_access_range auto&& view) noexcept {
        // https://e-maxx.ru/algo/z_function
        const size_t n = std::ranges::size(view);

        std::vector<size_t> ret;
        try {
            ret.resize(n);
        } catch (const std::bad_alloc&) {
            return std::make_error_condition(std::errc::not_enough_memory);
        }

        ret[0] = n;
        size_t l {0}, r {0};
        for (size_t i = 1; i < n; ++i) {
            if (i < r) {
                ret[i] = std::min(ret[i - l], r - i);
            }

            while (i + ret[i] < n && view[i + ret[i]] == view[ret[i]]) {
                ++ret[i];
            }

            if (i + ret[i] > r) {
                l = i;
                r = i + ret[i];
            }
        }
        return std::move(ret);
    }

} // namespace algo
