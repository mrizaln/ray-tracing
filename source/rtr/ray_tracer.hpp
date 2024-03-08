#pragma once

#include "rtr/common.hpp"
#include "rtr/color.hpp"
#include "rtr/hittable_list.hpp"
#include "rtr/progress.hpp"
#include "rtr/sphere.hpp"

#include <fmt/core.h>

#include <chrono>
#include <memory>
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
        RayTracer()
            : m_progressBar{ 100, 0 }
            , m_aspectRatio{ 16.0 / 9.0 }
        {
            auto height = 720;
            auto width  = int(height * m_aspectRatio);

            auto actualRatio = double(width) / double(height);
            auto viewHeight  = 2.0;
            auto viewWidth   = viewHeight * actualRatio;

            Vec viewport_u{ viewWidth, 0.0, 0.0 };
            Vec viewport_v{ 0.0, -viewHeight, 0.0 };

            m_dimension = {
                .m_width  = width,
                .m_height = height,
            };

            m_viewport = {
                .m_width  = viewWidth,
                .m_height = viewHeight,
                .m_u      = viewport_u,
                .m_v      = viewport_v,
                .m_du     = viewport_u / m_dimension.m_width,
                .m_dv     = viewport_v / m_dimension.m_height,
            };

            m_camera = {
                .m_focalLength = 1.0,
                .m_center      = { 0.0, 0.0, 0.0 },
            };

            m_world.emplace<Sphere>(Vec{ 0.0, 0.0, -1.0 }, 0.5);
            m_world.emplace<Sphere>(Vec{ 0.0, -100.5, -1.0 }, 100);
        }

        Image run()
        {
            // figure out location of the upper left pixel.
            Vec viewport_upper_left = m_camera.m_center - Vec{ 0.0, 0.0, m_camera.m_focalLength }    //
                                    - m_viewport.m_u / 2 - m_viewport.m_v / 2;
            auto pixel100_loc = viewport_upper_left + 0.5 * (m_viewport.m_du + m_viewport.m_dv);

            // render
            std::vector<Color<double>> pixels;
            pixels.reserve(std::size_t(m_dimension.m_width * m_dimension.m_height));

            fmt::println("Starting render...");
            m_progressBar.start({}, [](auto start, auto end, auto /* status */) {
                using Seconds = std::chrono::duration<double>;
                auto duration = std::chrono::duration_cast<Seconds>(end - start);
                auto msg      = std::format("Render completed in {}", duration);
                fmt::println("{}", msg);
            });

            for (auto row : rv::iota(0, m_dimension.m_height)) {
                auto progress = std::size_t((row + 1) / (double)m_dimension.m_height * 100);
                m_progressBar.update(progress);

                for (auto col : rv::iota(0, m_dimension.m_width)) {
                    auto pixelCenter  = pixel100_loc + (col * m_viewport.m_du) + (row * m_viewport.m_dv);
                    auto rayDirection = pixelCenter - m_camera.m_center;

                    Ray  ray{ m_camera.m_center, rayDirection };
                    auto pixelColor = rayColor(ray);
                    pixels.push_back(pixelColor);
                }
            }
            m_progressBar.stop(true);

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

        rtr::ProgressBar m_progressBar;

        double    m_aspectRatio = 16.0 / 9.0;
        Dimension m_dimension;
        Viewport  m_viewport;
        Camera    m_camera;

        // scene
        HittableList m_world;
    };
}
