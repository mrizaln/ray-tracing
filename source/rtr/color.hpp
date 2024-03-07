#pragma once

namespace rtr
{

    template <typename T = double>
    struct Color
    {
        union
        {
            T r;
            T h;
            T x;
        };
        union
        {
            T g;
            T s;
            T y;
        };
        union
        {
            T b;
            T v;
            T z;
        };
    };

    template <typename T = double>
    struct Color4
    {
        union
        {
            T r;
            T h;
            T x;
        };
        union
        {
            T g;
            T s;
            T y;
        };
        union
        {
            T b;
            T v;
            T z;
        };
        union
        {
            T a;
            T w;
        };
    };

    template <typename To, typename From>
    Color<To> colorCast(const Color<From>& from, From fmin, From fmax, To tmin, To tmax)
    {
        const auto fromScale = fmax - fmin;
        const auto toScale   = tmax - tmin;

        const auto cast = [&](auto value) { return static_cast<To>((value - fmin) / fromScale * toScale + tmin); };

        return {
            .r = cast(from.r),
            .g = cast(from.g),
            .b = cast(from.b),
        };
    }

}
