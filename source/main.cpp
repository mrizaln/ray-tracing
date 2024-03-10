#include "rtr/color.hpp"
#include "rtr/progress.hpp"
#include "rtr/ray_tracer.hpp"
#include "rtr/sphere.hpp"

#include <concurrencpp/runtime/runtime.h>
#include <fmt/core.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>

std::string formatName(std::string_view name, std::string_view extension)
{
    const auto* zone = std::chrono::current_zone();
    auto        time = zone->to_local(std::chrono::system_clock::now());
    return std::format("{}_{:%F_%H-%M-%OS}.{}", name, time, extension);
};

void generatePpmImage(
    rtr::ProgressBarManager&      progress,
    std::span<rtr::Color<double>> pixels,
    int                           width,
    int                           height,
    std::filesystem::path         outPath
)
{
    constexpr int maxColor = 255;

    std::ofstream outFile{ outPath };
    if (!outFile.good()) {
        throw std::runtime_error{ fmt::format("Problem opening file '{}'", outPath.string()) };
    }

    const auto write = [&outFile]<typename... Ts>(std::format_string<Ts...> fmt, Ts&&... args) {
        outFile << std::format(fmt, std::forward<Ts>(args)...);
    };

    progress.add("write", 0, height);

    write("P3\n");
    write("{} {}\n", width, height);
    write("{}\n", maxColor);

    for (int w = 0; const auto& pixel : pixels) {
        auto corrected = rtr::colorfn::correctGamma(pixel);
        auto clamped   = rtr::colorfn::clamp(corrected, { 0.0, 0.999 });
        auto color     = rtr::colorfn::cast<int>(clamped, 0.0, 1.0, 0, maxColor);
        write("{} {} {}\n", color.x(), color.y(), color.z());

        if (++w % width == 0) {
            progress.update("write", w / width + 1);
        }
    }
    progress.update("write", height);
}

int main(int argc, char** argv)
{
    std::filesystem::path outFile = formatName("out", "ppm");
    if (argc > 1) {
        std::filesystem::path newOutFile = argv[1];
        if (std::filesystem::exists(newOutFile)) {
            std::filesystem::remove(newOutFile);
            fmt::println("File exist already, will overwrite");
        }
        outFile = newOutFile;
    }

    concurrencpp::runtime   runtime;
    rtr::ProgressBarManager progressBar{ runtime };
    progressBar.start(runtime);

    rtr::HittableList world{};

    // clang-format off
    auto& ground    = world.emplace<rtr::Sphere>(rtr::Vec{  0.0, -100.5, -1.0 },  100);
    auto& center    = world.emplace<rtr::Sphere>(rtr::Vec{  0.0,  0.0,   -1.0 },  0.5);
    auto& leftOuter = world.emplace<rtr::Sphere>(rtr::Vec{ -1.0,  0.0,   -1.0 },  0.5);
    auto& leftInner = world.emplace<rtr::Sphere>(rtr::Vec{ -1.0,  0.0,   -1.0 }, -0.4);
    auto& right     = world.emplace<rtr::Sphere>(rtr::Vec{  1.0,  0.0,   -1.0 },  0.5);

    ground   .setMaterial<rtr::Lambertian>(rtr::Color<>{ 0.8, 0.8, 0.0 });
    center   .setMaterial<rtr::Lambertian>(rtr::Color<>{ 0.7, 0.3, 0.3 });
    leftOuter.setMaterial<rtr::Dielectric>(1.5);
    leftInner.setMaterial<rtr::Dielectric>(1.5);
    right    .setMaterial<rtr::Metal>(rtr::Color<>{ 0.8, 0.6, 0.2 }, 0.1);
    // clang-format on

    double aspectRatio  = 16.0 / 9.0;
    int    height       = 720;
    int    samplingRate = 100;

    rtr::RayTracer rayTracer{ std::move(world), aspectRatio, height, samplingRate };
    rtr::Image     image = rayTracer.run(progressBar);

    generatePpmImage(progressBar, image.m_pixels, image.m_width, image.m_height, outFile);
}
