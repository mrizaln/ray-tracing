#include "rtr/color.hpp"
#include "rtr/ray_tracer.hpp"

#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <stdexcept>
#include <utility>

using namespace std::chrono_literals;

std::string formatName(std::string_view name, std::string_view extension)
{
    const auto* zone = std::chrono::current_zone();
    auto        time = zone->to_local(std::chrono::system_clock::now());
    return std::format("{}_{:%F_%H-%M-%OS}.{}", name, time, extension);
};

void generatePpmImage(std::span<rtr::Color<double>> pixels, int width, int height, std::filesystem::path outPath)
{
    constexpr int maxColor = 255;

    if (std::filesystem::exists(outPath)) {
        throw std::runtime_error{ fmt::format("File '{}' already exist", outPath.string()) };
    }

    std::ofstream outFile{ outPath };
    if (!outFile.good()) {
        throw std::runtime_error{ fmt::format("Problem opening file '{}'", outPath.string()) };
    }

    const auto write = [&outFile]<typename... Ts>(std::format_string<Ts...> fmt, Ts&&... args) {
        outFile << std::format(fmt, std::forward<Ts>(args)...);
    };

    write("P3\n");
    write("{} {}\n", width, height);
    write("{}\n", maxColor);

    for (const auto& color : pixels) {
        auto c = rtr::colorCast<int>(color, 0.0, 1.0, 0, 255);
        write("{} {} {}\n", c.x(), c.y(), c.z());
    }
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

    rtr::RayTracer rayTracer{};
    auto           image = rayTracer.run();

    generatePpmImage(image.m_pixels, image.m_width, image.m_height, outFile);
}
