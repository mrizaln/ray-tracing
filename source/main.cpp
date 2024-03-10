#include "rtr/color.hpp"
#include "rtr/progress.hpp"
#include "rtr/ray_tracer.hpp"
#include "rtr/sphere.hpp"
#include "rtr/util.hpp"

#include <concurrencpp/runtime/runtime.h>
#include <fmt/core.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

std::string formatName(std::string_view name, std::string_view extension)
{
    const auto* zone = std::chrono::current_zone();
    auto        time = zone->to_local(std::chrono::system_clock::now());
    return std::format("{}_{:%F_%H-%M-%OS}.{}", name, time, extension);
};

void generatePpmImage(std::span<rtr::Color<double>> pixels, int width, int height, std::filesystem::path outPath)
{
    constexpr int maxColor = 255;

    std::ofstream outFile{ outPath, std::ios::out | std::ios::trunc };
    if (!outFile.good()) {
        throw std::runtime_error{ fmt::format("Problem opening file '{}'", outPath.string()) };
    }

    // prelude
    std::string temp = fmt::format("P3\n{} {}\n{}\n", width, height, maxColor);

    for (int w = 0; const auto& pixel : pixels) {
        auto corrected  = rtr::colorfn::correctGamma(pixel);
        auto clamped    = rtr::colorfn::clamp(corrected, { 0.0, 0.999 });
        auto color      = rtr::colorfn::cast<int>(clamped, { 0.0, 1.0 }, { 0, maxColor });
        temp           += fmt::format("{} {} {}\n", color.x(), color.y(), color.z());

        if (++w % width == 0) {
            outFile << std::exchange(temp, {});
        }
    }
}

rtr::HittableList createScene()
{
    using namespace rtr;

    static constexpr double glassRefractionIndex = 1.5;

    HittableList scene;

    auto& ground = scene.emplace<Sphere>(Vec{ 0.0, -1000.0, 0.0 }, 1000.0);
    ground.setMaterial<Lambertian>(Color<>{ 0.5, 0.5, 0.5 });

    // small spheres
    for (int a : rv::iota(-11, 11)) {
        for (int b : rv::iota(-11, 11)) {
            Vec center{ a + 0.9 * util::getRandomDouble(), 0.2, b + 0.9 * util::getRandomDouble() };
            Vec offset{ 4.0, 0.2, 0.0 };

            if (vecfn::length(center - offset) <= 0.9) {
                break;
            }

            auto& sphere = scene.emplace<Sphere>(center, 0.2);

            if (double chooseMaterial = util::getRandomDouble(); chooseMaterial < 0.8) {
                // diffuse
                auto albedo = vecfn::random(0.0, 1.0) * vecfn::random(0.0, 1.0);
                sphere.setMaterial<Lambertian>(albedo);
            } else if (chooseMaterial < 0.95) {
                // metal
                auto albedo = vecfn::random(0.5, 1.0);
                auto fuzz   = util::getRandomDouble(0.0, 0.5);
                sphere.setMaterial<Metal>(albedo, fuzz);
            } else {
                // glass
                sphere.setMaterial<Dielectric>(glassRefractionIndex);
            }
        }
    }

    // big spheres
    auto& sphere1 = scene.emplace<Sphere>(Vec{ 0.0, 1.0, 0.0 }, 1.0);
    sphere1.setMaterial<Dielectric>(glassRefractionIndex);

    auto& sphere2 = scene.emplace<Sphere>(Vec{ -4.0, 1.0, 0.0 }, 1.0);
    sphere2.setMaterial<Lambertian>(Color<>{ 0.4, 0.2, 0.1 });

    auto& sphere3 = scene.emplace<Sphere>(Vec{ 4.0, 1.0, 0.0 }, 1.0);
    sphere3.setMaterial<Metal>(Color<>{ 0.7, 0.6, 0.5 }, 0.0);

    return scene;
}

int main(int argc, char** argv)
{
    std::filesystem::path outFile = formatName("out", "ppm");
    if (argc > 1) {
        if (std::filesystem::exists(argv[1])) {
            fmt::println("File '{}' already exist, will overwrite", argv[1]);
        }

        if (std::filesystem::is_directory(argv[1])) {
            fmt::println(stderr, "File '{}' is a directory, reverting to default name...", argv[1]);
        } else {
            outFile = argv[1];
        }
    }

    concurrencpp::runtime   runtime;
    rtr::ProgressBarManager progressBar{ runtime };
    progressBar.start(runtime);

    rtr::RayTracer rayTracer{
        createScene(),
        {
            .m_aspectRatio   = 16.0 / 9.0,
            .m_height        = 1080,
            .m_samplingRate  = 100,
            .m_maxDepth      = 25,
            .m_fov           = 20.0,
            .m_focusDistance = 10.0,
            .m_defocusAngle  = 0.6,
            .m_lookFrom      = { 13.0, 2.0, 3.0 },
            .m_lookAt        = { 0.0, 0.0, 0.0 },
        },
    };

    rtr::Image image = rayTracer.run(progressBar);

    generatePpmImage(image.m_pixels, image.m_width, image.m_height, outFile);
}
