#pragma once

#include "rtr/color.hpp"
#include "rtr/common.hpp"
#include "rtr/progress.hpp"
#include "rtr/ray.hpp"
#include "rtr/vec.hpp"

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
            auto width  = 400;
            auto height = int(width / m_aspectRatio);
            height      = height < 1 ? 1 : height;

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
        }

        Image run()
        {
            // calculate vector accross the horizontal and down the vertical viewport edges

            // figure out location of the upper left pixel.
            Vec viewport_upper_left = m_camera.m_center - Vec{ 0.0, 0.0, m_camera.m_focalLength }    //
                                    - m_viewport.m_u / 2 - m_viewport.m_v / 2;
            auto pixel100_loc = viewport_upper_left + 0.5 * (m_viewport.m_du + m_viewport.m_dv);

            // render
            std::vector<Color<double>> pixels;
            pixels.reserve(std::size_t(m_dimension.m_width * m_dimension.m_height));

            m_progressBar.start({}, [](auto start, auto end, auto /* reason */) {
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
                auto msg      = std::format("Render completed in {}", duration);
                fmt::println(stderr, "{}", msg);
            });

            for (auto row : rv::iota(0, m_dimension.m_height)) {
                auto progress = std::size_t((double)row / (double)m_dimension.m_width) * 100;
                m_progressBar.update(progress);

                for (auto col : rv::iota(0, m_dimension.m_width)) {
                    auto pixelCenter  = pixel100_loc + (col * m_viewport.m_du) + (row * m_viewport.m_dv);
                    auto rayDirection = pixelCenter - m_camera.m_center;

                    Ray  ray{ m_camera.m_center, rayDirection };
                    auto pixelColor = hitSphere({ 0.0, 0.0, -1.0 }, 0.5, ray) ? Color<>{ 1.0, 0.0, 0.0 }
                                                                              : rayColor(ray);
                    pixels.push_back(pixelColor);
                }
            }
            m_progressBar.stop();

            return {
                .m_pixels = std::move(pixels),
                .m_width  = m_dimension.m_width,
                .m_height = m_dimension.m_height,
            };
        }

    private:
        Color<double> rayColor(const Ray& ray) const
        {
            auto dir = vecfn::normalized(ray.direction()).value();

            auto          a = 0.5 * (dir.y() + 1.0);
            const Color<> white{ 1.0, 1.0, 1.0 };
            const Color<> blue{ 0.5, 0.7, 1.0 };

            // linear blend (lerp)
            return (1.0 - a) * white + a * blue;
        }

        bool hitSphere(const Vec3<double>& center, double radius, const Ray& ray)
        {
            // basically quadration formula
            auto oc           = ray.origin() - center;
            auto a            = vecfn::dot(ray.direction(), ray.direction());
            auto b            = 2.0 * vecfn::dot(oc, ray.direction());
            auto c            = vecfn::dot(oc, oc) - radius * radius;
            auto discriminant = b * b - 4 * a * c;
            return discriminant >= 0;
        }

        rtr::ProgressBar m_progressBar;

        double    m_aspectRatio = 16.0 / 9.0;
        Dimension m_dimension;
        Viewport  m_viewport;
        Camera    m_camera;
    };
}
