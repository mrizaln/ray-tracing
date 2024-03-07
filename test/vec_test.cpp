#include "rtr/concepts.hpp"
#include "rtr/vec.hpp"

#include <fmt/core.h>
#include <boost/ut.hpp>

#include <cmath>
#include <concepts>
#include <string>
#include <utility>

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

        // with scalar
        Vf3   v3_3{ 1.0f, 2.0f, 3.0f };
        float fScalar = 2.0f;
        int   iScalar = 2;    // beware of using unsigned, it will cause unexpected results (rollover, etc.)

        ut::expect(v3_3 * fScalar == Vf3{ 2.0f, 4.0f, 6.0f });
        ut::expect(v3_3 / fScalar == Vf3{ 0.5f, 1.0f, 1.5f });
        ut::expect(v3_3 + fScalar == Vf3{ 3.0f, 4.0f, 5.0f });
        ut::expect(v3_3 - fScalar == Vf3{ -1.0f, 0.0f, 1.0f });

        ut::expect(fScalar * v3_3 == Vf3{ 2.0f, 4.0f, 6.0f });
        ut::expect(fScalar + v3_3 == Vf3{ 3.0f, 4.0f, 5.0f });

        ut::expect(v3_3 * iScalar == Vf3{ 2.0f, 4.0f, 6.0f });
        ut::expect(v3_3 / iScalar == Vf3{ 0.5f, 1.0f, 1.5f });
        ut::expect(v3_3 + iScalar == Vf3{ 3.0f, 4.0f, 5.0f });
        ut::expect(v3_3 - iScalar == Vf3{ -1.0f, 0.0f, 1.0f });

        ut::expect(iScalar * v3_3 == Vf3{ 2.0f, 4.0f, 6.0f });
        ut::expect(iScalar + v3_3 == Vf3{ 3.0f, 4.0f, 5.0f });
        ut::expect(iScalar + v3_3 == Vf3{ 3.0f, 4.0f, 5.0f });
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

    "custom types"_test = [] {
        struct CustomType
        {
            int x1{};
            int x2{};

            // NOTE: regular requirements is that the type should be default constructible, copyable, and movable.
            CustomType()                             = default;
            ~CustomType()                            = default;
            CustomType(const CustomType&)            = default;
            CustomType& operator=(const CustomType&) = default;

            CustomType(int x1, int x2)
                : x1{ x1 }
                , x2{ x2 }
            {
            }

            CustomType(CustomType&& other)
                : x1{ std::exchange(other.x1, 0) }
                , x2{ std::exchange(other.x2, 0) }
            {
            }

            CustomType& operator=(CustomType&& other)
            {
                x1 = std::exchange(other.x1, 0);
                x2 = std::exchange(other.x2, 0);
                return *this;
            }

            // NOTE: regular also requires the type to be equality comparable
            auto operator<=>(const CustomType&) const = default;

            // NOTE: rtr::Arith requires the following operators to be defined.
            CustomType operator+(const CustomType& rhs) const { return { x1 + rhs.x1, x2 + rhs.x2 }; }
            CustomType operator-(const CustomType& rhs) const { return { x1 - rhs.x1, x2 - rhs.x2 }; }
            CustomType operator*(const CustomType& rhs) const { return { x1 * rhs.x1, x2 * rhs.x2 }; }
            CustomType operator/(const CustomType& rhs) const { return { x1 / rhs.x1, x2 / rhs.x2 }; }
            CustomType operator-() const { return { -x1, -x2 }; }

            // NOTE: to be able to use rtr::vecfn::toString and or Vec<T, N>::toString
            // comment out the following line and see the compilation error.
            operator std::string() const { return fmt::format("{{ x1 = {}, x2 = {} }}", x1, x2); }
        };

        static_assert(std::regular<CustomType>, "CustomType is not regular");
        static_assert(rtr::Arith<CustomType>, "CustomType should be able to do arithmetic operations");

        using V = Vec<CustomType, 2>;
        V v1{ CustomType{ 1, 2 }, CustomType{ 3, 4 } };

        // copy
        V v2 = v1;
        ut::expect(v1 == v2);

        // move
        V v3 = std::move(v1);
        ut::expect(v3 == v2);

        // NOTE: this one depends on the implementation of the move ctor/assignment of the custom type.
        // CustomType zeroes out the moved from object, so the following should be true.
        ut::expect(v1 == V{}) << fmt::format("{} != {}", rtr::vecfn::toString(v1), rtr::vecfn::toString(V{}));
    };
}
