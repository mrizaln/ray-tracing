#pragma once

#include "rtr/ray.hpp"
#include "rtr/vec.hpp"

#include <limits>
#include <ranges>
#include <chrono>
#include <numbers>

namespace rtr
{

    namespace rv = std::views;
    namespace rr = std::ranges;

    using namespace std::chrono_literals;

    namespace n
    {
        using namespace std::numbers;

        static constexpr double infinity = std::numeric_limits<double>::infinity();
        static constexpr double epsilon  = std::numeric_limits<double>::epsilon();
    }

}
