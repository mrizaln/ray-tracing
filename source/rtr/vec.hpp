#pragma once

#include "rtr/concepts.hpp"
#include "rtr/util.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <array>
#include <cmath>
#include <concepts>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace rtr
{
    // forward declarations
    template <typename T, std::size_t N>
        requires std::regular<T> && Arith<T>
    class Vec;

    namespace vecfn
    {
        template <typename T, std::size_t N>
        T dot(const Vec<T, N>&, const Vec<T, N>&);

        template <typename T, std::size_t N>
        T lengthSquared(const Vec<T, N>&);

        template <typename T, std::size_t N>
        T length(const Vec<T, N>&);

        template <typename T, std::size_t N>
        Vec<T, N> normalized(const Vec<T, N>&);
    }
    // forward declarations end

    template <typename T, std::size_t N>
        requires std::regular<T> && Arith<T>
    class Vec
    {
    public:
        Vec() = default;

        template <typename... TT>
        Vec(TT&&... data)
            requires(AllOf<T, std::remove_cvref_t<TT>...> && sizeof...(data) == N)
            : m_data{ std::forward<T>(data)... }
        {
        }

        static std::size_t dimensions() { return N; }

        T  operator[](std::size_t i) const { return m_data[i]; }
        T& operator[](std::size_t i) { return m_data[i]; }

        auto operator<=>(const Vec& other) const = default;

        Vec operator-() const
        {
            auto data = m_data;
            for (auto& e : data) {
                e = -e;
            }
            return Vec{ std::move(data) };
        }

        Vec operator+(const Vec& other) const
        {
            auto data = m_data;
            for (std::size_t i = 0; i < data.size(); ++i) {
                data[i] = data[i] + other.m_data[i];
            };
            return Vec{ std::move(data) };
        }

        Vec operator-(const Vec& other) const
        {
            auto data = m_data;
            for (std::size_t i = 0; i < data.size(); ++i) {
                data[i] = data[i] - other.m_data[i];
            };
            return Vec{ std::move(data) };
        }

        Vec operator*(const Vec& other) const
        {
            auto data = m_data;
            for (std::size_t i = 0; i < data.size(); ++i) {
                data[i] = data[i] * other.m_data[i];
            };
            return Vec{ std::move(data) };
        }

        Vec operator/(const Vec& other) const
        {
            auto data = m_data;
            for (std::size_t i = 0; i < data.size(); ++i) {
                data[i] = data[i] / other.m_data[i];
            };
            return Vec{ std::move(data) };
        }

        template <typename TT>
            requires Add<T, TT, T>
        Vec operator+(const TT& other) const
        {
            auto data = m_data;
            for (auto& e : data) {
                e = e + other;
            };
            return Vec{ std::move(data) };
        }

        template <typename TT>
            requires Neg<TT, TT>
        Vec operator-(const TT& other) const
        {
            return *this + (-other);
        }

        template <typename TT>
            requires Mul<T, TT, T>
        Vec operator*(const TT& other) const
        {
            auto data = m_data;
            for (auto& e : data) {
                e = e * other;
            }
            return Vec{ std::move(data) };
        }

        template <typename TT>
            requires Div<T, TT, T>
        Vec operator/(const TT& other) const
        {
            auto data = m_data;
            for (auto& e : data) {
                e = e / other;
            };
            return Vec{ std::move(data) };
        }

        Vec& operator+=(const Vec& other)
        {
            for (std::size_t i = 0; i < m_data.size(); ++i) {
                m_data[i] = m_data[i] + other.m_data[i];
            };
            return *this;
        }

        Vec& operator-=(const Vec& other)
        {
            for (std::size_t i = 0; i < m_data.size(); ++i) {
                m_data[i] = m_data[i] - other.m_data[i];
            };
            return *this;
        }

        Vec& operator*=(const Vec& other)
        {
            for (std::size_t i = 0; i < m_data.size(); ++i) {
                m_data[i] = m_data[i] * other.m_data[i];
            };
            return *this;
        }

        Vec& operator/=(const Vec& other)
        {
            for (std::size_t i = 0; i < m_data.size(); ++i) {
                m_data[i] = m_data[i] / other.m_data[i];
            };
            return *this;
        }

        template <typename TT>
            requires Add<T, TT, T>
        Vec& operator+=(const TT& other)
        {
            for (auto& e : m_data) {
                e = e + other;
            };
            return *this;
        }

        template <typename TT>
            requires Neg<TT, TT>
        Vec& operator-=(const TT& other)
        {
            return *this += (-other);
        }

        template <typename TT>
            requires Mul<T, TT, T>
        Vec& operator*=(const TT& other)
        {
            for (auto& e : m_data) {
                e = e * other;
            };
            return *this;
        }

        template <typename TT>
            requires Div<T, TT, T>
        Vec& operator/=(const TT& other)
        {
            for (auto& e : m_data) {
                e = e / other;
            };
            return *this;
        }

        // operations with other type but reversed (subtraction and division are omitted because it doesn't makes sense)
        template <typename TT>
            requires Add<T, TT, T>
        friend Vec operator+(const TT& lhs, const Vec& rhs)
        {
            return rhs + lhs;
        }

        template <typename TT>
            requires Mul<T, TT, T>
        friend Vec operator*(const TT& lhs, const Vec& rhs)
        {
            return rhs * lhs;
        }

        auto tie() const
        {
            const auto make = [&]<std::size_t... I>(std::index_sequence<I...>) constexpr {
                return std::tie(m_data[I]...);
            };
            return make(std::make_index_sequence<N>{});
        }

        auto tie()
        {
            const auto make = [&]<std::size_t... I>(std::index_sequence<I...>) constexpr {
                return std::tie(m_data[I]...);
            };
            return make(std::make_index_sequence<N>{});
        }

        auto unpack() const
        {
            using namespace std;
            const auto make = [&]<size_t... I>(index_sequence<I...>) constexpr { return make_tuple(m_data[I]...); };
            return make(make_index_sequence<N>{});
        }

        std::string toString() const
            requires(fmt::is_formattable<T>::value || std::convertible_to<const T, std::string>)
        {
            if constexpr (fmt::is_formattable<T>::value) {
                return fmt::format("{}", m_data);
            } else {
                auto str  = std::string{ "[" };
                str      += static_cast<std::string>(m_data[0]);
                for (std::size_t i = 1; i < m_data.size(); ++i) {
                    str += ", " + static_cast<std::string>(m_data[i]);
                }
                str += ']';
                return str;
            }
        }

        // getter
        // clang-format off
        T&       x()       requires(N >= 1 && N <= 4) { return m_data[0]; }
        const T& x() const requires(N >= 1 && N <= 4) { return m_data[0]; }
        T&       y()       requires(N >= 2 && N <= 4) { return m_data[1]; }
        const T& y() const requires(N >= 2 && N <= 4) { return m_data[1]; }
        T&       z()       requires(N >= 3 && N <= 4) { return m_data[2]; }
        const T& z() const requires(N >= 3 && N <= 4) { return m_data[2]; }
        T&       w()       requires(N >= 4 && N <= 4) { return m_data[3]; }
        const T& w() const requires(N >= 4 && N <= 4) { return m_data[3]; }
        // clang-format on

    protected:
        friend auto vecfn::dot<T, N>(const Vec<T, N>&, const Vec<T, N>&) -> T;
        friend auto vecfn::lengthSquared<T, N>(const Vec&) -> T;
        friend auto vecfn::length<T, N>(const Vec&) -> T;
        friend auto vecfn::normalized<T, N>(const Vec&) -> Vec;

    private:
        explicit Vec(std::array<T, N>&& data)
            : m_data{ std::move(data) }
        {
        }

        std::array<T, N> m_data{};
    };

    // aliases
    template <typename T = double>
    using Vec2 = Vec<T, 2>;

    template <typename T = double>
    using Vec3 = Vec<T, 3>;

    template <typename T = double>
    using Vec4 = Vec<T, 4>;

    // deduction guides
    template <typename T>
    Vec(T, T) -> Vec<T, 2>;

    template <typename T>
    Vec(T, T, T) -> Vec<T, 3>;

    template <typename T>
    Vec(T, T, T, T) -> Vec<T, 4>;

    namespace vecfn
    {

        // special for Vec<T, 3>
        template <typename T>
        Vec<T, 3> cross(const Vec<T, 3>& lhs, const Vec<T, 3>& rhs)
        {
            auto x = lhs.y() * rhs.z() - lhs.z() * rhs.y();
            auto y = lhs.z() * rhs.x() - lhs.x() * rhs.z();
            auto z = lhs.x() * rhs.y() - lhs.y() * rhs.x();
            return { std::move(x), std::move(y), std::move(z) };
        }

        template <typename T, std::size_t N = 3>
        T dot(const Vec<T, N>& lhs, const Vec<T, N>& rhs)
        {
            T acc{};
            for (std::size_t i = 0; i < lhs.m_data.size(); ++i) {
                acc += lhs.m_data[i] * rhs.m_data[i];
            };
            return acc;
        }

        template <typename T, std::size_t N = 3>
        T lengthSquared(const Vec<T, N>& vec)
        {
            return dot(vec, vec);
        }

        template <typename T, std::size_t N = 3>
        T length(const Vec<T, N>& vec)
        {
            return std::sqrt(lengthSquared(vec));
        }

        template <typename T, std::size_t N = 3>
        Vec<T, N> normalized(const Vec<T, N>& vec)
        {
            auto l = length(vec);
            if (l == 0) {
                throw std::domain_error{ "Vector has 0 length! Can't normalize a 0 length vector!" };
            }
            return vec / l;
        }

        template <typename T, std::size_t N = 3>
            requires(fmt::is_formattable<T>::value || std::convertible_to<const T, std::string>)
        std::string toString(const Vec<T, N>& vec)
        {
            return vec.toString();
        }

        template <typename T, std::size_t N = 3>
        Vec<T, N> random(T min, T max)
        {
            const auto make = [&]<std::size_t... I>(std::index_sequence<I...>) constexpr {
                return Vec<T, N>{ (I, util::getRandom<T>(min, max))... };    // NOLINT
            };
            return make(std::make_index_sequence<N>{});
        }

        template <std::size_t N = 3>
        Vec<double, N> random()
        {
            return random<double, N>(0.0, 1.0);
        }

        // only makes sense in 3D
        template <typename T>
        Vec<T, 3> randomInUnitSphere()
        {
            while (true) {
                auto point = random<T, 3>(T{ -1 }, T{ 1 });
                if (lengthSquared(point) < T{ 1 }) {
                    return point;
                }
            }
        }

        template <typename T>
        Vec<T, 3> randomUnitVector()
        {
            return normalized(randomInUnitSphere<T>());
        }

        template <typename T>
        Vec<T, 3> randomOnHemisphere(const Vec<T, 3>& normal)
        {
            auto vector = randomUnitVector<T>();
            if (dot(vector, normal) > 0.0) {
                return vector;
            } else {
                return -vector;
            }
        }

    }    // namespace vec

}    // namespace rtr
