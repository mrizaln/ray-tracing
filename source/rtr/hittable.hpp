#pragma once

#include "rtr/interval.hpp"
#include "rtr/material.hpp"
#include "rtr/ray.hpp"
#include "rtr/hit_record.hpp"

#include <memory>
#include <variant>

namespace rtr
{

    class Hittable
    {
    public:
        using HitResult = std::variant<std::monostate, HitRecord, ScatterResult>;

        Hittable()
            : m_material{ std::make_unique<Lambertian>(Color<>{ 0.1, 0.1, 0.11 }) }
        {
        }

        Hittable(Hittable&&)                 = default;
        Hittable& operator=(Hittable&&)      = default;
        Hittable(const Hittable&)            = delete;
        Hittable& operator=(const Hittable&) = delete;
        virtual ~Hittable()                  = default;

        virtual HitResult hit(const Ray& ray, Interval<double> tRange) const = 0;

        template <std::derived_from<Material> T, typename... Args>
            requires std::constructible_from<T, Args...>
        Material& setMaterial(Args&&... args)
        {
            m_material = std::make_unique<T>(std::forward<Args>(args)...);
            return *m_material;
        }

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

        Hittable::HitResult hit(const Ray& ray, Interval<double> tRange) const override
        {
            Hittable::HitResult currentHit{};

            double tClosest = tRange.max();
            for (const auto& object : m_objects) {
                auto hit = object->hit(ray, { tRange.min(), tClosest });
                std::visit(
                    [&](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::same_as<T, HitRecord>) {
                            tClosest   = arg.m_t;
                            currentHit = std::move(arg);
                        } else if constexpr (std::same_as<T, ScatterResult>) {
                            tClosest   = arg.m_t;
                            currentHit = std::move(arg);
                        }
                    },
                    std::move(hit)
                );
            }

            return currentHit;
        }

    private:
        std::vector<std::unique_ptr<Hittable>> m_objects;
    };

}
