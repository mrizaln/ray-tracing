#pragma once

#include "rtr/common.hpp"

#include <concepts>
#include <algorithm>

namespace rtr
{

    template <std::regular T = double>
    class Interval
    {
    public:
        static const Interval<double> s_universe;
        static const Interval<double> s_empty;

        Interval(double min, double max)
            : m_min{ min }
            , m_max{ max }
        {
        }

        double min() const { return m_min; }
        double max() const { return m_max; }

        bool   contains(double value) const { return m_min <= value && value <= m_max; }
        bool   surrounds(double value) const { return m_min < value && value < m_max; }
        double clamp(double value) const { return std::clamp(value, m_min, m_max); }

        std::pair<double&, double&> tie() { return { m_min, m_max }; }

    private:
        double m_min;
        double m_max;
    };

    template <>
    inline const Interval<double> Interval<double>::s_universe = { -n::infinity, +n::infinity };

    template <>
    inline const Interval<double> Interval<double>::s_empty = { +n::infinity, -n::infinity };

}
