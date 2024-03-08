#pragma once

#include "rtr/vec.hpp"

namespace rtr
{

    class Ray
    {
    public:
        using Point = Vec3<>;
        using Dir   = Vec3<>;

        Ray(const Point& origin, const Dir& direction)
            : m_origin{ origin }
            , m_direction{ direction }
        {
        }

        Dir   direction() const { return m_direction; }
        Point origin() const { return m_origin; }
        Point at(double t) const { return m_origin + t * m_direction; }

    private:
        Point m_origin;
        Dir   m_direction;
    };

}
