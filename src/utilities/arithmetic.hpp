#pragma once
#include <optional>
#include <ordefs.hpp>
#include <orc_export.hpp>
using namespace orc::core::defines;


namespace orc::utils::arithmetic {
    template<typename T>
    requires std::is_integral_v<T>
    ORC_API constexpr auto add_with_overflow(T left, T right) -> std::pair<T, bool> {
        T res;
        if constexpr (std::is_unsigned_v<T>) {
            res = left + right;
            if (res < left)
                return std::make_pair(0, true);
            return std::make_pair(res, false);
        } else {
            if constexpr (std::is_same_v<T, i8>) {
                if (_add_overflow_i8(0,
                                     static_cast<i8>(left),
                                     static_cast<i8>(right),
                                     &res)
                        )
                    return std::make_pair(0, true);
                return std::make_pair(res, false);
            } else if constexpr (std::is_same_v<T, i16>) {
                if (_add_overflow_i16(0,
                                      static_cast<i16>(left),
                                      static_cast<i16>(right),
                                      &res)
                        )
                    return std::make_pair(0, true);
                return std::make_pair(res, false);
            } else if constexpr (std::is_same_v<T, i32>) {
                if (_add_overflow_i32(0,
                                      static_cast<i32>(left),
                                      static_cast<i32>(right),
                                      &res)
                        )
                    return std::make_pair(0, true);
                return std::make_pair(res, false);
            } else {
                if (_add_overflow_i64(0,
                                      static_cast<i64>(left),
                                      static_cast<i64>(right),
                                      &res)
                        )
                    return std::make_pair(0, true);
                return std::make_pair(res, false);
            }
        }
    }
    template<typename T>
    requires std::is_floating_point_v<T>
    ORC_API constexpr auto add_with_overflow(T left, T right) -> std::pair<T, bool> {
        T res = left + right;
        if (isinf(res))
            return std::make_pair(0, true);
        return std::make_pair(res, false);
    }

    template<typename T>
    requires std::is_arithmetic_v<T>
    ORC_API constexpr auto checked_add(const T left, const T right) -> std::optional<T> {
        if (auto [a, b] = add_with_overflow(left, right); b) return std::make_optional(a);
        return std::nullopt;
    }

    template<typename T>
    requires std::is_integral_v<T>
    ORC_API constexpr auto mul_with_overflow(T left, T right) -> std::pair<T, bool> {
        T res;
        T dummy;
        if constexpr (std::is_unsigned_v<T>) {
            res = left * right;
            if (res < left)
                return std::make_pair(0, true);
            return std::make_pair(res, false);
        } else {
            if constexpr (std::is_same_v<T, i8>) {
                if (_mul_full_overflow_i8(
                        static_cast<i8>(left),
                        static_cast<i8>(right),
                        &res)
                        )
                    return std::make_pair(0, true);
                return std::make_pair(res, false);
            } else if constexpr (std::is_same_v<T, i16>) {

                if (_mul_full_overflow_i16(
                        static_cast<i16>(left),
                        static_cast<i16>(right),
                        &res,
                        &dummy)
                        )
                    return std::make_pair(0, true);
                return std::make_pair(res, false);
            } else if constexpr (std::is_same_v<T, i32>) {
                if (_mul_full_overflow_i32(
                        static_cast<i32>(left),
                        static_cast<i32>(right),
                        &res,
                        &dummy)
                        )
                    return std::make_pair(0, true);
                return std::make_pair(res, false);
            } else {
                if (_mul_full_overflow_i64(
                        static_cast<i64>(left),
                        static_cast<i64>(right),
                        &res,
                        &dummy)
                        )
                    return std::make_pair(0, true);
                return std::make_pair(res, false);
            }
        }
    }
    template<typename T>
    requires std::is_arithmetic_v<T>
    ORC_API constexpr auto checked_mul(const T left, const T right) -> std::optional<T> {
        if (auto [a, b] = mul_with_overflow(left, right); b) return std::make_optional(a);
        return std::nullopt;
    }

    template<typename T>
    requires std::is_arithmetic_v<T>
    ORC_API constexpr auto div_euclid(const T self, const T rhs) -> T {
        const T q = self / rhs;
        if (self % rhs < 0) {
            if (rhs > 0) {
                return q - 1;
            }
            return q + 1;
        }
        return q;
    }

    template<typename T>
    requires std::is_arithmetic_v<T>
    ORC_API constexpr auto wrapping_add(const T self, const T rhs) -> T {
#undef max
#undef min
        const T max = std::numeric_limits<T>::max();
        if (max - self < rhs) {
            return std::numeric_limits<T>::min() + (max - self - rhs);
        }
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))
        return self + rhs;
    }
    template<typename T>
    requires std::is_arithmetic_v<T>
    ORC_API constexpr auto wrapping_abs(const T self) -> T {
        if constexpr (std::is_signed_v<T>) {
            return std::abs(self);
        } else {
            return self;
        }
    }

    template<typename T>
    requires std::is_arithmetic_v<T>
    ORC_API constexpr auto rem_euclid(const T self, const T rhs) -> T {
        const T r = self % rhs;
        if (r < 0) {
            return wrapping_add(r, wrapping_abs(rhs));
        }
        return r;

    }
}


