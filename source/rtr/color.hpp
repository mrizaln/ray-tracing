#pragma once

#include "rtr/common.hpp"
#include "rtr/interval.hpp"
#include "rtr/util.hpp"
#include "rtr/vec.hpp"

namespace rtr
{

    template <typename T = double>
    using Color = rtr::Vec<T, 3>;

    namespace colorfn
    {

        template <typename To, typename From>
        Color<To> cast(const Color<From>& from, From fmin, From fmax, To tmin, To tmax)
        {
            const auto fromScale = fmax - fmin;
            const auto toScale   = tmax - tmin;

            const auto cast = [&](auto value) { return static_cast<To>((value - fmin) / fromScale * toScale + tmin); };

            return {
                cast(from.x()),
                cast(from.y()),
                cast(from.z()),
            };
        }
        inline Color<double> correctGamma(const Color<double>& color)
        {
            return {
                util::linearToGamma(color.x()),
                util::linearToGamma(color.y()),
                util::linearToGamma(color.z()),
            };
        }

        inline Color<double> clamp(const Color<double>& color, Interval interval)
        {
            return {
                interval.clamp(color.x()),
                interval.clamp(color.y()),
                interval.clamp(color.z()),
            };
        }

    }

}
