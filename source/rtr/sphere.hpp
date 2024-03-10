#pragma once

#include "rtr/hittable.hpp"

#include <cmath>

namespace rtr
{

    class Sphere : public Hittable
    {
    public:
        Sphere(Vec3<double> center, double radius)
            : m_center{ std::move(center) }
            , m_radius{ radius }
        {
        }

        void setMaterial(std::unique_ptr<Material> material) { m_material = std::move(material); }

        Hittable::HitResult hit(const Ray& ray, Interval tRange) const override
        {
            // basically quadratic formula
            const Vec  oc     = ray.origin() - m_center;
            const auto a      = vecfn::lengthSquared(ray.direction());
            const auto b_half = vecfn::dot(oc, ray.direction());
            const auto c      = vecfn::lengthSquared(oc) - m_radius * m_radius;

            const auto D = b_half * b_half - a * c;
            if (D < 0) {
                return {};
            }

            const auto D_sqrt = std::sqrt(D);
            const auto root1  = (-b_half - D_sqrt) / a;
            const auto root2  = (-b_half + D_sqrt) / a;

            if (!tRange.surrounds(root1) && !tRange.surrounds(root2)) {
                return {};
            }

            auto root = std::min(root1, root2);
            if (root < tRange.min()) {
                root = std::max(root1, root2);
            }

            const Vec point     = ray.at(root);
            const Vec outNormal = (point - m_center) / m_radius;

            auto hit = HitRecord::from(ray, outNormal, point, root);

            if (!m_material) {
                return hit;
            }

            if (auto scatter = m_material->scatter(ray, hit); scatter.has_value()) {
                return std::move(scatter).value();
            } else {
                return hit;
            }
        }

        Vec3<double> center() const { return m_center; }
        double       radius() const { return m_radius; }

    private:
        Vec3<double> m_center;
        double       m_radius;
    };

};
