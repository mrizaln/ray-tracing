#pragma once

#include "rtr/common.hpp"

#include <ctime>
#include <random>

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

    // canonical: 0 <= x < 1
    inline double getRandomDouble()
    {
        thread_local static std::mt19937 mt{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };
        thread_local static std::uniform_real_distribution dist{ 0.0, 1.0 };
        return dist(mt);
    }

}
