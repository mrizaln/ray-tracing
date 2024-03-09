#pragma once

#include "rtr/common.hpp"

#include <algorithm>

namespace rtr
{

    class Interval
    {
    public:
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

    private:
        double m_min;
        double m_max;

        static const Interval s_universe;
        static const Interval s_empty;
    };

    inline const Interval Interval::s_universe = { -n::infinity, +n::infinity };
    inline const Interval Interval::s_empty    = { +n::infinity, -n::infinity };
}
