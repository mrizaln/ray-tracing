#pragma once

#include "rtr/color.hpp"
#include "rtr/common.hpp"
#include "rtr/progress.hpp"
#include "rtr/ray.hpp"
#include "rtr/vec.hpp"

#include <cmath>
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

    class Sphere
    {
    public:
        Sphere(Vec3<double> center, double radius)
            : m_center{ std::move(center) }
            , m_radius{ radius }
        {
        }

        std::optional<double> hit(const Ray& ray) const
        {
            // basically quadratic formula
            auto oc     = ray.origin() - m_center;
            auto a      = vecfn::lengthSquared(ray.direction());
            auto b_half = vecfn::dot(oc, ray.direction());
            auto c      = vecfn::lengthSquared(oc) - m_radius * m_radius;
            auto D      = b_half * b_half - a * c;

            if (D < 0) {
                return {};
            }
            return -(b_half + std::sqrt(D)) / a;
        }

        Vec3<double> center() const { return m_center; }
        double       radius() const { return m_radius; }

    private:
        Vec3<double> m_center;
        double       m_radius;
    };

    class RayTracer
    {
    public:
        RayTracer()
            : m_progressBar{ 100, 0 }
            , m_aspectRatio{ 16.0 / 9.0 }
            , m_sphere{ Vec{ 0.0, 0.0, -1.0 }, 0.5 }
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
                    auto pixelColor = rayColor(ray);
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
            if (std::optional t = m_sphere.hit(ray); t.has_value()) {
                auto n = vecfn::normalized(ray.at(*t) - m_sphere.center());
                return 0.5 * Color<>{ n.x() + 1, n.y() + 1, n.z() + 1 };
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
        Sphere m_sphere;
    };
}
