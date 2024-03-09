#pragma once

#include "rtr/common.hpp"
#include "rtr/color.hpp"
#include "rtr/hittable_list.hpp"
#include "rtr/progress.hpp"
#include "rtr/util.hpp"

#include <fmt/core.h>

#include <chrono>
#include <ranges>
#include <vector>

namespace rtr
{

    struct Dimension
    {
        int m_width;
        int m_height;
    };

    struct Viewport
    {
        double       m_width;
        double       m_height;
        Vec3<double> m_u;
        Vec3<double> m_v;
        Vec3<double> m_du;
        Vec3<double> m_dv;
        Vec3<double> m_upperLeft;
        Vec3<double> m_pixel00Loc;
    };

    struct Camera
    {
        double       m_focalLength;
        Vec3<double> m_center;
    };

    struct Image
    {
        std::vector<Color<double>> m_pixels;
        int                        m_width;
        int                        m_height;
    };

    class RayTracer
    {
    public:
        RayTracer(HittableList&& world, double aspectRatio, int height)
            : m_aspectRatio{ aspectRatio }
            , m_world{ std::move(world) }
        {
            auto width = int(height * m_aspectRatio);

            auto actualRatio = double(width) / double(height);
            auto viewHeight  = 2.0;
            auto viewWidth   = viewHeight * actualRatio;

            Vec viewport_u{ viewWidth, 0.0, 0.0 };
            Vec viewport_v{ 0.0, -viewHeight, 0.0 };
            Vec viewport_du = viewport_u / width;
            Vec viewport_dv = viewport_v / height;

            Vec    camCenter{ 0.0, 0.0, 0.0 };
            double camFocalLength{ 1.0 };

            Vec viewUpperLeft = camCenter - Vec{ 0.0, 0.0, camFocalLength }    //
                              - viewport_u / 2 - viewport_v / 2;
            auto pixel00loc = viewUpperLeft + 0.5 * (viewport_du + viewport_dv);

            m_dimension = {
                .m_width  = width,
                .m_height = height,
            };

            m_camera = {
                .m_focalLength = camFocalLength,
                .m_center      = camCenter,
            };

            m_viewport = {
                .m_width      = viewWidth,
                .m_height     = viewHeight,
                .m_u          = viewport_u,
                .m_v          = viewport_v,
                .m_du         = viewport_du,
                .m_dv         = viewport_dv,
                .m_upperLeft  = viewUpperLeft,
                .m_pixel00Loc = pixel00loc,
            };
        }

        Image run(rtr::ProgressBarManager& progressBar)
        {
            // render
            std::vector<Color<double>> pixels(std::size_t(m_dimension.m_width * m_dimension.m_height));

            const int concurrencyLevel = (int)std::thread::hardware_concurrency();
            const int chunkSize        = m_dimension.m_height / concurrencyLevel;

            fmt::println("concurrency level = {} | chunk size: {}", concurrencyLevel, chunkSize);

            std::vector<std::jthread> threads;
            threads.reserve((std::size_t)concurrencyLevel);

            for (auto i : rv::iota(0, concurrencyLevel)) {
                auto chunkBegin = i * chunkSize;
                auto chunkEnd   = std::min(chunkBegin + chunkSize, m_dimension.m_height);

                std::string name = fmt::format("render thread {}", i);
                progressBar.add(name, chunkBegin, chunkEnd);

                threads.emplace_back([this, chunkBegin, chunkEnd, name, &pixels, &progressBar] {
                    for (auto row : rv::iota(chunkBegin, chunkEnd)) {
                        auto rowSize = std::size_t(m_dimension.m_width);
                        progressBar.update(name, row + 1);

                        for (auto col : rv::iota(0, m_dimension.m_width)) {
                            auto idx    = (std::size_t)row * rowSize + (std::size_t)col;
                            pixels[idx] = sampleColorAt(col, (int)row);
                        }
                    }
                });
            }

            for (auto& thread : threads) {
                thread.join();
            }

            return {
                .m_pixels = std::move(pixels),
                .m_width  = m_dimension.m_width,
                .m_height = m_dimension.m_height,
            };
        }

    private:
        Color<double> rayColor(const Ray& ray) const
        {
            if (std::optional record = m_world.hit(ray, { 0.0, n::infinity }); record.has_value()) {
                const auto&   n = record->m_normal;
                const Color<> offset{ 1.0, 1.0, 1.0 };
                return 0.5 * (n + offset);
            };

            auto dir = vecfn::normalized(ray.direction());

            auto          a = 0.5 * (dir.y() + 1.0);
            const Color<> white{ 1.0, 1.0, 1.0 };
            const Color<> blue{ 0.5, 0.7, 1.0 };

            // linear blend (lerp)
            return (1.0 - a) * white + a * blue;
        }

        Color<double> sampleColorAt(int col, int row) const
        {
            Color<> cummulativeColor{ 0.0, 0.0, 0.0 };
            auto    pixelCenter = m_viewport.m_pixel00Loc + (col * m_viewport.m_du) + (row * m_viewport.m_dv);

            for (auto i [[maybe_unused]] : rv::iota(0, static_cast<int>(m_samplesPerPixel))) {
                auto pixelSample  = pixelCenter + sampleUnitSquare();
                auto rayDirection = pixelSample - m_camera.m_center;

                Ray ray{ m_camera.m_center, rayDirection };
                cummulativeColor += rayColor(ray);
            }

            return cummulativeColor / static_cast<double>(m_samplesPerPixel);
        }

        Vec3<double> sampleUnitSquare() const
        {
            auto px = -0.5 + util::getRandomDouble();
            auto py = -0.5 + util::getRandomDouble();
            return (px * m_viewport.m_du) + (py * m_viewport.m_dv);
        }

        double    m_aspectRatio;
        Dimension m_dimension;
        Viewport  m_viewport;
        Camera    m_camera;

        // scene
        HittableList m_world;

        std::size_t m_samplesPerPixel = 10;
    };
}
