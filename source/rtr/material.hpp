#pragma once

#include "rtr/color.hpp"
#include "rtr/hit_record.hpp"
#include "rtr/ray.hpp"
#include "rtr/util.hpp"
#include "rtr/vec.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

namespace rtr
{

    class Material
    {
    public:
        virtual ~Material() = default;

        virtual std::optional<ScatterResult> scatter(const Ray& ray, const HitRecord& record) const = 0;
    };

    class Lambertian final : public Material
    {
    public:
        Lambertian(Color<double> color)
            : m_albedo{ std::move(color) }
        {
        }

        std::optional<ScatterResult> scatter(const Ray& /* ray */, const HitRecord& record) const override
        {
            auto scatterDirection = record.m_normal + vecfn::randomUnitVector();

            if (vecfn::nearZero(scatterDirection)) {
                scatterDirection = record.m_normal;
            }

            return ScatterResult{
                .m_ray         = { record.m_point, scatterDirection },
                .m_attenuation = m_albedo,
            };
        }

    private:
        Color<double> m_albedo;
    };

    class Metal final : public Material
    {
    public:
        Metal(Color<double> color, double fuzz)
            : m_albedo{ std::move(color) }
            , m_fuzz{ std::clamp(fuzz, 0.0, 1.0) }
        {
        }

        std::optional<ScatterResult> scatter(const Ray& ray, const HitRecord& record) const override
        {
            auto reflected = vecfn::reflect(vecfn::normalized(ray.direction()), record.m_normal);
            Ray  scattered{ record.m_point, reflected + m_fuzz * vecfn::randomInUnitSphere() };

            if (vecfn::dot(scattered.direction(), record.m_normal) <= 0) {
                return {};
            }

            return ScatterResult{
                .m_ray         = std::move(scattered),
                .m_attenuation = m_albedo,
            };
        };

    private:
        Color<double> m_albedo;
        double        m_fuzz;
    };

    class Dielectric final : public Material
    {
    public:
        Dielectric(double refractiveIndex)
            : m_refractiveIndex{ refractiveIndex }
        {
        }

        std::optional<ScatterResult> scatter(const Ray& ray, const HitRecord& record) const override
        {
            double refractionRatio = record.m_frontFace ? (1.0 / m_refractiveIndex) : m_refractiveIndex;

            auto unitDirection = vecfn::normalized(ray.direction());

            // total internal reflection
            double cosTheta = std::min(vecfn::dot(-unitDirection, record.m_normal), 1.0);
            double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);

            bool cannotRefract = refractionRatio * sinTheta > 1.0
                              || reflectance(cosTheta, refractionRatio) > util::getRandomDouble();

            auto scatter = cannotRefract ? vecfn::reflect(unitDirection, record.m_normal)
                                         : vecfn::refract(unitDirection, record.m_normal, refractionRatio);

            return ScatterResult{
                .m_ray         = { record.m_point, scatter },
                .m_attenuation = { 1.0, 1.0, 1.0 },
            };
        }

    private:
        static double reflectance(double cosine, double refractionIndex)
        {
            // Schlick approximation for reflectance
            auto r0 = (1 - refractionIndex) / (1 + refractionIndex);
            r0      = r0 * r0;
            return r0 + (1 - r0) * std::pow(1 - cosine, 5.0);
        }

        double m_refractiveIndex;
    };

}
