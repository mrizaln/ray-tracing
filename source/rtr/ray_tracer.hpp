#pragma once

#include "rtr/color.hpp"
#include "rtr/common.hpp"
#include "rtr/hittable.hpp"
#include "rtr/progress.hpp"
#include "rtr/ray.hpp"
#include "rtr/util.hpp"
#include "rtr/vec.hpp"

#include <cmath>
#include <fmt/core.h>

#include <ranges>
#include <variant>
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
        Vec3<double> m_center;
        Vec3<double> m_viewUp;
        Vec3<double> m_viewRight;
        Vec3<double> m_viewDir;    // opposite direction of the lookAt
        Vec3<double> m_defocusDisk_u;
        Vec3<double> m_defocusDisk_v;
        double       m_verticalFov;
        double       m_defocusAngle;
        double       m_focusDistance;
    };

    struct Image
    {
        std::vector<Color<double>> m_pixels;
        int                        m_width;
        int                        m_height;
    };

    struct TracerParam
    {
        double       m_aspectRatio   = 16.0 / 9.0;
        int          m_height        = 360;
        int          m_samplingRate  = 100;
        int          m_maxDepth      = 10;
        double       m_fov           = 90.0;
        double       m_focusDistance = 0.80;
        double       m_defocusAngle  = 10.0;
        Vec3<double> m_lookFrom      = { 0.0, 0.0, 0.0 };
        Vec3<double> m_lookAt        = { 0.0, 0.0, -1.0 };
    };

    class RayTracer
    {
    public:
        RayTracer(HittableList&& world, TracerParam param)
            : m_aspectRatio{ param.m_aspectRatio }
            , m_world{ std::move(world) }
            , m_samplesPerPixel{ param.m_samplingRate }
        {
            Vec worldUp = { 0.0, 1.0, 0.0 };

            Vec    camCenter  = { param.m_lookFrom };
            double camVertFov = param.m_fov;

            Vec viewDir   = vecfn::normalized(param.m_lookFrom - param.m_lookAt);
            Vec viewRight = vecfn::normalized(vecfn::cross(worldUp, viewDir));
            Vec viewUp    = vecfn::cross(viewDir, viewRight);

            auto theta = util::toRadian(camVertFov);
            auto h     = std::tan(theta / 2.0);

            auto height      = param.m_height;
            auto width       = int(height * m_aspectRatio);
            auto actualRatio = double(width) / double(height);
            auto viewHeight  = 2.0 * h * param.m_focusDistance;
            auto viewWidth   = viewHeight * actualRatio;

            Vec viewport_u  = viewWidth * viewRight;
            Vec viewport_v  = viewHeight * -viewUp;
            Vec viewport_du = viewport_u / width;
            Vec viewport_dv = viewport_v / height;

            Vec  viewUpperLeft = camCenter - (param.m_focusDistance * viewDir) - viewport_u / 2 - viewport_v / 2;
            auto pixel00Loc    = viewUpperLeft + 0.5 * (viewport_du + viewport_dv);

            auto defocusRadius = param.m_focusDistance * std::tan(util::toRadian(param.m_defocusAngle / 2));
            auto defocusDisk_u = viewRight * defocusRadius;
            auto defocusDisk_v = viewUp * defocusRadius;

            m_dimension = {
                .m_width  = width,
                .m_height = height,
            };

            m_camera = {
                .m_center        = camCenter,
                .m_viewUp        = viewUp,
                .m_viewRight     = viewRight,
                .m_viewDir       = viewDir,
                .m_defocusDisk_u = defocusDisk_u,
                .m_defocusDisk_v = defocusDisk_v,
                .m_verticalFov   = camVertFov,
                .m_defocusAngle  = param.m_defocusAngle,
                .m_focusDistance = param.m_focusDistance,
            };

            m_viewport = {
                .m_width      = viewWidth,
                .m_height     = viewHeight,
                .m_u          = viewport_u,
                .m_v          = viewport_v,
                .m_du         = viewport_du,
                .m_dv         = viewport_dv,
                .m_upperLeft  = viewUpperLeft,
                .m_pixel00Loc = pixel00Loc,
            };

            m_maxDepth = param.m_maxDepth;
        }

        Image run(rtr::ProgressBarManager& progressBar)
        {
            // render
            std::vector<Color<double>> pixels(std::size_t(m_dimension.m_width * m_dimension.m_height));

            const int concurrencyLevel = (int)std::thread::hardware_concurrency();
            const int chunkSize        = m_dimension.m_height / concurrencyLevel;

            fmt::println("Concurrency level = {} | chunk size: {}", concurrencyLevel, chunkSize);

            std::vector<std::jthread> threads;
            threads.reserve((std::size_t)concurrencyLevel);

            // each thread now works on interleaved rows
            for (auto i : rv::iota(0, concurrencyLevel)) {
                std::string name = fmt::format("render thread {}", i);
                auto numSteps = (chunkSize * concurrencyLevel + i < m_dimension.m_height) ? chunkSize + 1 : chunkSize;
                progressBar.add(name, 0, numSteps);

                threads.emplace_back([=, this, &pixels, &progressBar, name = std::move(name)] {
                    for (auto count : rv::iota(0, numSteps)) {
                        auto rowSize = std::size_t(m_dimension.m_width);
                        auto row     = (count * concurrencyLevel) + i;
                        progressBar.update(name, count + 1);

                        for (auto col : rv::iota(0, m_dimension.m_width)) {
                            auto idx    = (std::size_t)row * rowSize + (std::size_t)col;
                            pixels[idx] = colorfn::clamp(sampleColorAt(col, (int)row), { 0.0, 1.0 });
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
        Color<double> rayColor(const Ray& ray, int depth = 0) const
        {
            if (depth >= m_maxDepth) {
                return { 0.0, 0.0, 0.0 };
            }

            auto hit = m_world.hit(ray, { 0.001, n::infinity });

            return std::visit(
                [&](auto&& arg) -> Color<> {
                    using T = std::decay_t<decltype(arg)>;

                    if constexpr (std::same_as<T, ScatterResult>) {
                        // scattered
                        return arg.m_attenuation * rayColor(arg.m_ray, depth + 1);
                    } else if constexpr (std::same_as<T, HitRecord>) {
                        // absorbed
                        return Color<double>{ 0.0, 0.0, 0.0 };
                    } else {
                        // missed, use background color
                        auto dir = vecfn::normalized(ray.direction());

                        auto          a = 0.5 * (dir.y() + 1.0);
                        const Color<> white{ 1.0, 1.0, 1.0 };
                        const Color<> blue{ 0.5, 0.7, 1.0 };

                        // linear blend (lerp)
                        return (1.0 - a) * white + a * blue;
                    }
                },
                std::move(hit)
            );
        }

        Color<double> sampleColorAt(int col, int row) const
        {
            Color<> accumulatedColor{ 0.0, 0.0, 0.0 };
            auto    pixelCenter = m_viewport.m_pixel00Loc + (col * m_viewport.m_du) + (row * m_viewport.m_dv);

            for (auto i [[maybe_unused]] : rv::iota(0, m_samplesPerPixel)) {
                auto pixelSample   = pixelCenter + sampleUnitSquare();
                auto rayOrigin     = m_camera.m_defocusAngle <= 0 ? m_camera.m_center : defocusDiskSample();
                auto rayDirection  = pixelSample - rayOrigin;
                accumulatedColor  += rayColor({ rayOrigin, rayDirection });
            }

            return accumulatedColor / static_cast<double>(m_samplesPerPixel);
        }

        Vec3<double> sampleUnitSquare() const
        {
            auto px = -0.5 + util::getRandomDouble();
            auto py = -0.5 + util::getRandomDouble();
            return (px * m_viewport.m_du) + (py * m_viewport.m_dv);
        }

        Vec3<double> defocusDiskSample() const
        {
            const auto [x, y] = vecfn::randomInUnitDisk().tie();
            return m_camera.m_center + (x * m_camera.m_defocusDisk_u) + (y * m_camera.m_defocusDisk_v);
        }

        double    m_aspectRatio;
        Dimension m_dimension;
        Viewport  m_viewport;
        Camera    m_camera;

        // scene
        HittableList m_world;

        int m_samplesPerPixel;
        int m_maxDepth;
    };
}
