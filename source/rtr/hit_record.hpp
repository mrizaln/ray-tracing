#pragma once

#include "rtr/ray.hpp"
#include "rtr/vec.hpp"
#include "rtr/color.hpp"

#include <optional>

namespace rtr
{

    struct ScatterResult
    {
        Ray           m_ray;
        Color<double> m_attenuation;
        double        m_t;
    };

    struct HitRecord
    {
        Vec3<double> m_point;
        Vec3<double> m_normal;
        double       m_t;
        bool         m_frontFace;

        static HitRecord from(const Ray& ray, const Vec3<double>& outNormal, Vec3<double> point, double t)
        {
            bool front  = vecfn::dot(ray.direction(), outNormal) < 0;
            Vec  normal = front ? outNormal : -outNormal;

            return {
                .m_point     = point,
                .m_normal    = std::move(normal),
                .m_t         = t,
                .m_frontFace = front,
            };
        }
    };

}
