#pragma once

#include <concepts>

namespace rtr
{

    // clang-format off
    template <typename T1, typename T2 = T1, typename T3 = T1>
    concept Add = requires(T1 t1, T2 t2) {
        { t1 + t2 } -> std::same_as<T3>;
    };

    template <typename T1, typename T2 = T1, typename T3 = T1>
    concept Sub = requires(T1 t1, T2 t2) {
        { t1 - t2 } -> std::same_as<T3>;
    };

    template <typename T1, typename T2 = T1, typename T3 = T1>
    concept Mul = requires(T1 t1, T2 t2) {
        { t1 * t2 } -> std::same_as<T3>;
    };

    template <typename T1, typename T2 = T1, typename T3 = T1>
    concept Div = requires(T1 t1, T2 t2) {
        { t1 / t2 } -> std::same_as<T3>;
    };

    template <typename T1, typename T2 = T1>
    concept Neg = requires(T1 t1) {
        { -t1 } -> std::same_as<T2>;
    };
    // clang-format on

    template <typename T>
    concept Arith = Add<T> && Sub<T> && Mul<T> && Div<T> && Neg<T>;

}
