#include "rtr/color.hpp"
#include "rtr/progress.hpp"

#include <fmt/core.h>

#include <cstdlib>
#include <ctime>
#include <ranges>

namespace rv = std::views;
using namespace std::chrono_literals;

void generatePpmImage()
{
    constexpr int maxColor    = 255;
    constexpr int imageWidth  = 256;
    constexpr int imageHeight = 256;

    fmt::println("P3");
    fmt::println("{} {}", imageWidth, imageHeight);
    fmt::println("{}", maxColor);

    std::size_t      max = imageHeight;
    rtr::ProgressBar bar{ max, 0 };

    bar.start({}, [](auto start, auto end, auto reason) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        using R       = rtr::ProgressBar::StopReason;
        switch (reason) {
        case R::Stopped: fmt::println(stderr, "Stopped after {}ms", duration.count()); break;
        case R::Completed: fmt::println(stderr, "Completed in {}ms", duration.count()); break;
        }
    });

    for (auto j : rv::iota(0, imageHeight)) {
        bar.update(static_cast<std::size_t>(j + 1));

        for (auto i : rv::iota(0, imageWidth)) {

            rtr::Color<double> color{
                .r = double(i) / (imageWidth - 1),
                .g = double(j) / (imageHeight - 1),
                .b = 0.0,
            };

            auto c = rtr::colorCast<int>(color, 0.0, 1.0, 0, 255);
            fmt::println("{} {} {}", c.r, c.g, c.b);
        }
    }
}

int main()
{
    generatePpmImage();
}
