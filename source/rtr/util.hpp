#pragma once

#include "rtr/common.hpp"

#include <cmath>
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

    inline double getRandomCanonical()
    {
        thread_local static std::mt19937 mt{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };
        thread_local static std::uniform_real_distribution<double> dist{ 0.0, 1.0 };
        return dist(mt);
    }

    template <typename T>
    T getRandom(T min, T max)
    {
        return min + (max - min) * getRandomCanonical();
    }

    // canonical: 0 <= x < 1
    inline double getRandomDouble(double min = 0.0, double max = 1.0)
    {
        return getRandom(min, max);
    }

    inline double linearToGamma(double linear)
    {
        // inverse of gamma2
        return std::sqrt(linear);
    }

}
