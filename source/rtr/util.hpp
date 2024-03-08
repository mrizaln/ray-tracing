#pragma once

#include "rtr/common.hpp"

namespace rtr::util
{

    inline double toRadian(double deg)
    {
        return deg * n::pi / 180.0;
    }

    inline double toDegrees(double rad)
    {
        return rad * 180.0 / n::pi;
    }

    inline double operator""_rad_to_deg(long double rad)
    {
        return toDegrees(static_cast<double>(rad));
    }

    inline double operator""_deg_to_rad(long double deg)
    {
        return toRadian(static_cast<double>(deg));
    }

}
