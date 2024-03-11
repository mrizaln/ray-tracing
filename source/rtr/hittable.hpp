#pragma once

#include "rtr/interval.hpp"
#include "rtr/material.hpp"
#include "rtr/ray.hpp"
#include "rtr/hit_record.hpp"

#include <memory>

namespace rtr
{

    class Hittable
    {
    public:
        Hittable()
            : m_material{ std::make_unique<Lambertian>(Color<>{ 0.1, 0.1, 0.11 }) }
        {
        }

        Hittable(Hittable&&)                 = default;
        Hittable& operator=(Hittable&&)      = default;
        Hittable(const Hittable&)            = delete;
        Hittable& operator=(const Hittable&) = delete;
        virtual ~Hittable()                  = default;

        virtual std::optional<HitResult> hit(const Ray& ray, Interval<double> tRange) const = 0;

        template <std::derived_from<Material> T, typename... Args>
            requires std::constructible_from<T, Args...>
        Material& setMaterial(Args&&... args)
        {
            m_material = std::make_unique<T>(std::forward<Args>(args)...);
            return *m_material;
        }

        const Material* getMaterial() const { return m_material.get(); }

    protected:
        std::unique_ptr<Material> m_material = nullptr;
    };

    class HittableList : public Hittable
    {
    public:
        HittableList() = default;

        Hittable& add(std::unique_ptr<Hittable> object)
        {
            m_objects.push_back(std::move(object));
            return *m_objects.back();
        }

        template <typename T, typename... Args>
            requires std::constructible_from<T, Args...>
        Hittable& emplace(Args&&... args)
        {
            m_objects.push_back(std::make_unique<T>(std::forward<Args>(args)...));
            return *m_objects.back();
        }

        void clear() { m_objects.clear(); }

        std::optional<HitResult> hit(const Ray& ray, Interval<double> tRange) const override
        {
            std::optional<HitResult> currentHit{};

            double tClosest = tRange.max();
            for (const auto& object : m_objects) {
                if (auto hit = object->hit(ray, { tRange.min(), tClosest }); hit.has_value()) {
                    tClosest   = hit->m_record.m_t;
                    currentHit = std::move(hit);
                }
            }

            return currentHit;
        }

    private:
        std::vector<std::unique_ptr<Hittable>> m_objects;
    };

}
