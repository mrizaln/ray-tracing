#pragma once

#include "rtr/common.hpp"
#include "rtr/interval.hpp"

#include <optional>
#include <utility>

namespace rtr
{

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

    class Hittable
    {
    public:
        virtual ~Hittable() = default;

        virtual std::optional<HitRecord> hit(const Ray& ray, Interval tRange) const = 0;
    };

}
