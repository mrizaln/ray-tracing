#include <fmt/core.h>

#include <ranges>

namespace rv = std::views;

void generatePpmImage()
{
    constexpr int maxColor    = 255;
    constexpr int imageWidth  = 256;
    constexpr int imageHeight = 256;

    fmt::println("P3");
    fmt::println("{} {}", imageWidth, imageHeight);
    fmt::println("{}", maxColor);

    for (auto j : rv::iota(0, imageHeight)) {
        for (auto i : rv::iota(0, imageWidth)) {
            auto r = double(i) / (imageWidth - 1);
            auto g = double(j) / (imageHeight - 1);
            auto b = 0;

            int ir = static_cast<int>(255.999 * r);
            int ig = static_cast<int>(255.999 * g);
            int ib = static_cast<int>(255.999 * b);

            fmt::println("{} {} {}", ir, ig, ib);
        }
    }
}

int main()
{
    generatePpmImage();
}
