#include <fmt/core.h>
#include <boost/ut.hpp>

#include "rtr/vec.hpp"

using rtr::Vec;
using rtr::Vec2;
using rtr::Vec3;
using rtr::Vec4;

int main()
{
    namespace ut = boost::ut;
    using namespace ut::literals;
    using namespace ut::operators;

    "getter"_test = [] {
        Vec2<> v2{ 1.0f, 2.0f };
        ut::expect(v2.x() == 1_i);
        ut::expect(v2.y() == 2_i);

        Vec3<> v3{ 1.0f, 2.0f, 3.0f };
        ut::expect(v3.x() == 1_i);
        ut::expect(v3.y() == 2_i);
        ut::expect(v3.z() == 3_i);

        Vec4<> v4{ 1.0f, 2.0f, 3.0f, 4.0f };
        ut::expect(v4.x() == 1_i);
        ut::expect(v4.y() == 2_i);
        ut::expect(v4.z() == 3_i);
        ut::expect(v4.w() == 4_i);

        using V = Vec<int, 8>;
        V v8{ 1, 2, 3, 4, 5, 6, 7, 8 };
        ut::expect(v8[0] == 1_i);
        ut::expect(v8[1] == 2_i);
        ut::expect(v8[2] == 3_i);
        ut::expect(v8[3] == 4_i);
        ut::expect(v8[4] == 5_i);
        ut::expect(v8[5] == 6_i);
        ut::expect(v8[6] == 7_i);
        ut::expect(v8[7] == 8_i);
    };

    "operators"_test = [] {
        using Vf3 = Vec3<float>;

        Vf3 v3_1{ 1.0f, 2.0f, 3.0f };
        ut::expect(-v3_1 == Vf3{ -1.0f, -2.0f, -3.0f });

        Vf3 v3_2{ 4.0f, 5.0f, 6.0f };
        ut::expect(v3_1 + v3_2 == Vf3{ 5.0f, 7.0f, 9.0f });
        ut::expect(v3_1 - v3_2 == Vf3{ -3.0f, -3.0f, -3.0f });
        ut::expect(v3_1 * v3_2 == Vf3{ 4.0f, 10.0f, 18.0f });
        ut::expect(v3_1 / v3_2 == Vf3{ 1.0f / 4.0f, 2.0f / 5.0f, 3.0f / 6.0f });

        Vf3 temp{};
        temp  = v3_1;
        temp += v3_2;
        ut::expect(temp == (v3_1 + v3_2));

        temp  = v3_1;
        temp -= v3_2;
        ut::expect(temp == (v3_1 - v3_2));

        temp  = v3_1;
        temp *= v3_2;
        ut::expect(temp == (v3_1 * v3_2));

        temp  = v3_1;
        temp /= v3_2;
        ut::expect(temp == (v3_1 / v3_2));
    };

    "functions"_test = [] {
        using Vf3 = Vec3<float>;

        Vf3 v{ 1.0f, 2.0f, 3.0f };

        auto lengthSquared = 1.0f * 1.0f + 2.0f * 2.0f + 3.0f * 3.0f;
        ut::expect(rtr::vecfn::lengthSquared(v) == lengthSquared);
        ut::expect(rtr::vecfn::length(v) == std::sqrt(lengthSquared));
        ut::expect(rtr::vecfn::dot(v, v) == lengthSquared);

        Vf3 temp;
        Vf3 v1{ 1.0f, 0.0f, 0.0f };
        temp = rtr::vecfn::cross(v, v1);
        ut::expect(temp == Vf3{ 0.0f, 3.0f, -2.0f }) << rtr::vecfn::toString(temp);

        Vf3 v2{ 0.0f, 1.0f, 0.0f };
        temp = rtr::vecfn::cross(v, v2);
        ut::expect(temp == Vf3{ -3.0f, 0.0f, 1.0f }) << rtr::vecfn::toString(temp);

        Vf3 v3{ 0.0f, 0.0f, 1.0f };
        temp = rtr::vecfn::cross(v, v3);
        ut::expect(temp == Vf3{ 2.0f, -1.0f, 0.0f }) << rtr::vecfn::toString(temp);

        Vf3 v4{ 2.0f, 3.0f, 4.0f };
        temp = rtr::vecfn::cross(v, v4);
        ut::expect(temp == Vf3{ -1.0f, 2.0f, -1.0f }) << rtr::vecfn::toString(temp);
    };
}
