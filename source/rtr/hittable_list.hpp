#pragma once

#include "rtr/hittable.hpp"

#include <concepts>
#include <memory>
#include <optional>
#include <vector>

namespace rtr
{

    class HittableList : public Hittable
    {
    public:
        HittableList() = default;

        Hittable& add(std::shared_ptr<Hittable> object)
        {
            m_objects.push_back(std::move(object));
            return *m_objects.back();
        }

        template <typename T, typename... Args>
        Hittable& emplace(Args&&... args)
        {
            m_objects.push_back(std::make_shared<T>(std::forward<Args>(args)...));
            return *m_objects.back();
        }

        void clear() { m_objects.clear(); }

        std::optional<HitRecord> hit(const Ray& ray, std::pair<double, double> tRange) const override
        {
            std::optional<HitRecord> currentRecord{};

            double tClosest = tRange.second;
            for (const auto& object : m_objects) {
                if (auto record = object->hit(ray, { tRange.first, tClosest }); record.has_value()) {
                    tClosest      = record->m_t;
                    currentRecord = std::move(record);
                }
            }

            return currentRecord;
        }

    private:
        std::vector<std::shared_ptr<Hittable>> m_objects;
    };

}
