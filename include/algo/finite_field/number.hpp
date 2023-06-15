#pragma once

#include <iostream>
#include <algorithm>
#include <span>

namespace algo {

    template<size_t bit_size>
    class BigInt {
        static_assert(bit_size > 1);

    public:
        static constexpr size_t kByteSize = (bit_size + 7) / 8;
        static constexpr unsigned char kFirstByteMask = static_cast<unsigned char>(0b1111'1111) >> (bit_size % 8 == 0 ? 0 : 8 - bit_size % 8);

    public:
        constexpr BigInt() noexcept = default;
        constexpr BigInt(int integer) noexcept;
        constexpr BigInt(const char* str) noexcept;

        template<size_t another_bit_size>
            requires (another_bit_size < bit_size)
        constexpr BigInt(const BigInt<another_bit_size>& other) noexcept;

        constexpr bool operator==(const BigInt&) const noexcept = default;
        constexpr auto operator<=>(const BigInt&) const noexcept;

        constexpr BigInt& operator<<=(int) noexcept;
        constexpr BigInt& operator>>=(int) noexcept;
        constexpr BigInt operator<<(int) const noexcept;
        constexpr BigInt operator>>(int) const noexcept;


        constexpr BigInt operator-() const noexcept;
        constexpr BigInt& operator+=(const BigInt&) noexcept;
        constexpr BigInt& operator-=(const BigInt&) noexcept;
        constexpr BigInt& operator*=(const BigInt&) noexcept;
        constexpr BigInt& operator/=(const BigInt&) noexcept;
        constexpr BigInt& operator%=(const BigInt&) noexcept;

        constexpr void Print() const noexcept {
            if (is_positive_) {
                std::cerr << "+ ";
            } else {
                std::cerr << "- ";
            }
            for (size_t i = 0; i < kByteSize; ++i) {
                unsigned char n = 0b1000'0000;
                while (n != 0) {
                    std::cerr << (binary_.at(i) & n ? 1 : 0);
                    n /= 2;
                }
            }
            std::cerr << std::endl;
        }

    private:
        constexpr std::pair<BigInt, BigInt> DivideUnsigned(const BigInt&) const noexcept; // returns div and remainder

        bool is_positive_ = true;
        std::array<unsigned char, kByteSize> binary_ = {}; // most significant bit is on the left, e.g. 8 == '1000'
                                                           // can't use bitset here, because not constexp (since C++23)
    };

    // impl
    template<size_t bit_size>
    constexpr BigInt<bit_size>::BigInt(int from) noexcept {
        if (from < 0) {
            is_positive_ = false;
            from = -from;
        }
        unsigned int integer = from;
        unsigned int mask = 0b1111'1111;
        for (size_t i = 0; i < std::min(sizeof(integer), kByteSize); ++i) {
            binary_.at(kByteSize - i - 1) = static_cast<unsigned char>((integer & mask) >> 8 * i);
            mask <<= 8;
        }

        binary_[0] &= kFirstByteMask;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>::BigInt(const char* str) noexcept {
        if (str[0] == '\0' || (str[0] == '-' && str[1] == '\0')) {
            std::cerr << "Parsing empty string" << std::endl;
            std::terminate();
        }

        if (str[0] == '-') {
            is_positive_ = false;
            ++str;
        }

        size_t count = 0;
        if (str[0] == '0' && str[1] == 'b') {
            str += 2;
            size_t octet = 0;
            unsigned char octet_mask = static_cast<unsigned char>(1 << (bit_size % 8 == 0 ? 7 : bit_size % 8 - 1));
            while (*str != '\0') {
                binary_.at(octet) |= *str == '1' ? 0b1111'1111 & octet_mask : 0b0000'0000 & octet_mask;
                octet_mask >>= 1;
                if (octet_mask == 0) {
                    octet += 1;
                    octet_mask = 0b1000'0000;
                }
                ++str;
                count += 1;
            }

            *this >>= bit_size - count;
        } else {
            std::cerr << "Only bytes are allowed" << std::endl;
            std::terminate();
        }
    }

    template<size_t bit_size>
    template<size_t another_byte_size>
    constexpr BigInt<bit_size>::BigInt(bool positive, const std::array<unsigned char, another_byte_size>& bits) noexcept
        : is_positive_{positive}
    {
        std::copy(bits.rbegin(), bits.rbegin() + kByteSize, binary_.rbegin());
        binary_[0] &= kFirstByteMask;
    }

    template<size_t bit_size>
    template<size_t another_bit_size>
        requires(another_bit_size < bit_size)
    constexpr BigInt<bit_size>::BigInt(const BigInt<another_bit_size>& other) noexcept
        : is_positive_{other.is_positive_}
    {
        std::copy(other.binary_.rbegin(), other.binary_.rend(), binary_.rbegin());
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator<<=(int shift_int) noexcept {
        size_t shift = shift_int;
        if (shift >= bit_size) {
            shift %= bit_size;
        }
        if (shift == 0) {
            return *this;
        }
        size_t shift_bytes = shift / 8;
        for (size_t i = 0; i < kByteSize - shift_bytes - 1; ++i) {
            binary_.at(i) = (binary_.at(i + shift_bytes) << (shift % 8)) | (binary_.at(i + shift_bytes + 1) >> (8 - shift % 8));
        }
        binary_.at(kByteSize - shift_bytes - 1) = binary_.at(kByteSize - 1) << (shift % 8);
        for (size_t i = kByteSize - shift_bytes; i < kByteSize; ++i) {
            binary_.at(i) = 0;
        }
        binary_[0] &= kFirstByteMask;
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> BigInt<bit_size>::operator<<(int shift_int) const noexcept {
        return BigInt{*this} <<= shift_int;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> BigInt<bit_size>::operator>>(int shift_int) const noexcept {
        return BigInt{*this} >>= shift_int;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator>>=(int shift_int) noexcept {
        size_t shift = shift_int;
        if (shift >= bit_size) {
            shift %= bit_size;
        }
        if (shift == 0) {
            return *this;
        }
        size_t shift_bytes = shift / 8;
        for (size_t i = kByteSize - 1; i >= shift_bytes + 1; --i) {
            binary_.at(i) = binary_.at(i - shift_bytes) >> (shift % 8) | binary_.at(i - shift_bytes - 1) << (8 - shift % 8);
        }
        binary_.at(shift_bytes) = (binary_.at(0) & kFirstByteMask) >> (shift % 8);
        for (size_t i = 0; i < shift_bytes; ++i) {
            binary_.at(i) = 0;
        }
        binary_[0] &= kFirstByteMask;

        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> BigInt<bit_size>::operator-() const noexcept {
        BigInt copy = *this;
        copy.is_positive_ ^= true;
        return copy;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator+=(const BigInt& rhs) noexcept {
        if (is_positive_ ^ rhs.is_positive_) {
            return *this -= -rhs;
        }

        unsigned char carry = 0b0000'0000;
        unsigned short tmp;
        for (size_t idx = kByteSize - 1; idx > 0; --idx) {
            tmp = binary_.at(idx) + rhs.binary_.at(idx) + carry;
            if (tmp > 0b1111'1111) {
                carry = 0b0000'0001;
            } else {
                carry = 0b0000'0000;
            }
            binary_.at(idx) = static_cast<unsigned char>(tmp & 0b1111'1111);
        }
        binary_.at(0) += rhs.binary_.at(0) + carry;

        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator-=(const BigInt& rhs) noexcept {
        if (is_positive_ ^ rhs.is_positive_) {
            return *this += -rhs;
        }

        if ((is_positive_ && rhs > *this) || (!is_positive_ && rhs < *this)) {
            is_positive_ ^= true;
            BigInt tmp = *this;
            binary_ = rhs.binary_;
            return *this -= tmp;
        }

        unsigned char borrow = 0b0000'0000;
        unsigned short tmp;
        for (size_t i = kByteSize - 1; i > 0; --i) {
            tmp = 0b1'0000'0000 + binary_.at(i) - rhs.binary_.at(i) - borrow;
            if (tmp < 0b1'0000'0000) {
                borrow = 0b0000'0001;
            } else {
                borrow = 0b0000'0000;
            }
            binary_.at(i) = static_cast<unsigned char>(tmp & 0b1111'1111);
        }
        binary_.at(0) -= rhs.binary_.at(0) + borrow;
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator*=(const BigInt& rhs) noexcept {
        is_positive_ ^= !rhs.is_positive_;

        // https://www.geeksforgeeks.org/karatsuba-algorithm-for-fast-multiplication-using-divide-and-conquer-algorithm/
        BigInt ret;
        for (size_t ii = 0; ii < kByteSize; ++ii) {
            size_t i = kByteSize - 1 - ii;
            BigInt tmp_res;
            for (size_t j = 0; j < kByteSize; ++j) {
                size_t idx = i - j;
                unsigned short tmp = binary_.at(i);
                tmp *= rhs.binary_.at(kByteSize - 1 - j);
                unsigned short tmp_l = tmp & 0b1111'1111;
                unsigned short tmp_h = tmp >> 8;

                if (idx > 0) {
                    tmp_res.binary_.at(idx - 1) = static_cast<unsigned char>(tmp_h);
                }

                if (0b1111'1111 - tmp_l < tmp_res.binary_.at(idx)) {
                    unsigned short sum = tmp_l + tmp_res.binary_.at(idx);
                    tmp_res.binary_.at(idx) = static_cast<unsigned char>(sum & 0b1111'1111);
                    if (idx > 0) {
                        tmp_res.binary_.at(idx - 1) += static_cast<unsigned char>(sum >> 8);
                    }
                } else {
                    tmp_res.binary_.at(idx) += static_cast<unsigned char>(tmp_l);
                }

                if (idx == 0) {
                    break;
                }
            }
            ret += tmp_res;
        }

        *this = ret;
        binary_.at(0) &= kFirstByteMask;
        return *this;
    }

    template<size_t bit_size>
    constexpr std::pair<BigInt<bit_size>, BigInt<bit_size>> BigInt<bit_size>::DivideUnsigned(const BigInt& rhs) const noexcept {
        std::pair<BigInt, BigInt> ret;
        BigInt& quotient = ret.first;
        BigInt& remainder = ret.second;

        auto less = [this, &rhs](size_t idx) {

        };

        return ret;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator/=(const BigInt& rhs) noexcept {
        is_positive_ ^= !rhs.is_positive_;
        *this = DivideUnsigned(rhs).first;
        return *this;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size>& BigInt<bit_size>::operator%=(const BigInt& rhs) noexcept {
        *this = DivideUnsigned(rhs).second;
        return *this;
    }

    template<size_t bit_size>
    constexpr auto BigInt<bit_size>::operator<=>(const BigInt& rhs) const noexcept {
        if (is_positive_ ^ rhs.is_positive_) {
            return is_positive_ <=> rhs.is_positive_;
        }

        for (size_t i = 0; i < kByteSize; ++i) {
            if (binary_.at(i) != rhs.binary_.at(i)) {
                if (is_positive_) {
                    return binary_.at(i) <=> rhs.binary_.at(i);
                } else {
                    return rhs.binary_.at(i) <=> binary_.at(i);
                }
            }
        }

        return std::strong_ordering::equal;
    }

    // Arithmetic opeartors
    template<size_t bit_size>
    constexpr BigInt<bit_size> operator+(const BigInt<bit_size>& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{lhs} += rhs;
    }

    template<size_t bit_size, typename T>
    constexpr BigInt<bit_size> operator+(const BigInt<bit_size>& lhs, T&& rhs) noexcept {
        return BigInt<bit_size>{lhs} += BigInt<bit_size>{std::forward<T>(rhs)};
    }

    template<size_t bit_size, typename T>
    constexpr BigInt<bit_size> operator+(T&& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{std::forward<T>(lhs)} += rhs;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> operator-(const BigInt<bit_size>& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{lhs} -= rhs;;
    }

    template<size_t bit_size, typename T>
    constexpr BigInt<bit_size> operator-(const BigInt<bit_size>& lhs, T&& rhs) noexcept {
        return BigInt<bit_size>{lhs} -= BigInt<bit_size>{std::forward<T>(rhs)};
    }

    template<size_t bit_size, typename T>
    constexpr BigInt<bit_size> operator-(T&& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{std::forward<T>(lhs)} -= rhs;
    }

    template<size_t bit_size>
    constexpr BigInt<bit_size> operator*(const BigInt<bit_size>& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{lhs} *= rhs;
    }

    template<size_t bit_size, typename T>
    constexpr BigInt<bit_size> operator*(const BigInt<bit_size>& lhs, T&& rhs) noexcept {
        return BigInt<bit_size>{lhs} *= BigInt<bit_size>{std::forward<T>(rhs)};
    }

    template<size_t bit_size, typename T>
    constexpr BigInt<bit_size> operator*(T&& lhs, const BigInt<bit_size>& rhs) noexcept {
        return BigInt<bit_size>{std::forward<T>(lhs)} *= rhs;
    }

} // namespace algo
