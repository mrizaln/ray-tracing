#pragma once

#include "rtr/concepts.hpp"

#include <fmt/core.h>

#include <algorithm>
#include <atomic>
#include <array>
#include <chrono>
#include <concepts>
#include <format>
#include <functional>
#include <optional>
#include <stop_token>
#include <thread>
#include <vector>

namespace rtr
{
    template <typename T, std::size_t N>
        requires(rtr::Div<T, int, T> && rtr::Add<T, T, T>)
    class MovingAverage
    {
    public:
        MovingAverage()
            : m_entries(N)
            , m_index{ 0 }
        {
        }

        static std::size_t size() { return N; }

        T getAverage() const { return m_average; }

        T update(T newEntry)
        {
            m_entries[m_index] = std::move(newEntry);
            m_index            = (m_index + 1) % N;

            T acc{};
            for (const auto& e : m_entries) {
                acc = acc + e;
            }

            return m_average = acc / static_cast<int>(N);
        }

    private:
        std::vector<T> m_entries;
        std::size_t    m_index;
        T              m_average;
    };

    // FIXME: ETA will be incomprehensible if update is called with the same value multiple times
    class ProgressBar
    {
    public:
        using Clock        = std::chrono::steady_clock;
        using TimePoint    = std::chrono::time_point<Clock>;
        using TimeInterval = std::chrono::milliseconds;

        struct UpdateRecord
        {
            TimeInterval m_time{ 1 };
            int          m_diff{ 0 };

            UpdateRecord operator+(const UpdateRecord& other) const
            {
                return { m_time + other.m_time, m_diff + other.m_diff };
            }

            UpdateRecord operator/(int divisor) const
            {
                return {
                    .m_time = { std::chrono::duration_cast<TimeInterval>(m_time / divisor) },
                    .m_diff = int((double)m_diff / (double)divisor),
                };
            }

            auto operator<=>(const UpdateRecord&) const = default;
        };

        enum class Status
        {
            NotStarted,
            Ongoing,
            Completed,
            Stopped
        };

        ProgressBar(int max, int min = 0)
            : m_max{ max }
            , m_min{ min }
        {
        }

        // if current is not provided, it will be set to min
        void start(
            std::optional<int>                                                   current    = {},
            std::function<void(TimePoint start, TimePoint end, Status reason)>&& onComplete = {}
        )
        {
            m_current = current ? *current : m_min;
            m_status  = Status::Ongoing;

            m_thread = std::jthread{ [this, onComplete = std::move(onComplete)](std::stop_token st) mutable {
                m_startTime = Clock::now();

                while (!st.stop_requested() && m_current <= m_max) {
                    std::this_thread::sleep_for(s_delay);

                    print(m_status);

                    if (auto now = Clock::now(); now >= m_deadline.load()) {
                        break;
                    }
                }

                if (!onComplete) {
                    return;
                }

                auto currentTime = Clock::now();
                if (st.stop_requested() && m_current < m_max) {
                    m_status = Status::Stopped;
                    print(m_status);
                    std::putchar('\n');
                    onComplete(m_startTime, currentTime, m_status);
                } else {
                    m_status = Status::Completed;
                    print(m_status);
                    std::putchar('\n');
                    onComplete(m_startTime, currentTime, m_status);
                }
            } };
        }

        void update(int current)
        {
            auto last = m_current.load();
            m_current = std::clamp(current, m_min, m_max);

            auto currentTime = Clock::now();
            auto deltaTime   = currentTime - m_deadline.load() + s_timeout;
            m_deadline       = currentTime + s_timeout;

            auto diff        = std::max(0, current - last);
            auto deltaTimeMs = std::chrono::duration_cast<TimeInterval>(deltaTime);

            m_updateRecords.update({
                .m_time = deltaTimeMs,
                .m_diff = diff,
            });
        }

        void stop(bool wait = false)
        {
            m_thread.request_stop();
            if (wait) {
                m_thread.join();
            }
        }

    private:
        void print(Status status)
        {
            constexpr auto        width       = s_width - 3 - 1 - 1 - 1;    // hard-coded for now
            constexpr std::size_t spinnerSize = s_spinner.size();

            auto ratio   = (float)m_current / float(m_max - m_min);
            m_spinnerIdx = (m_spinnerIdx + 1) % int(spinnerSize);

            auto filledSize = std::size_t(ratio * width);
            auto emptySize  = width - filledSize;

            using Seconds      = std::chrono::duration<double>;
            auto remainingTime = std::chrono::duration_cast<Seconds>(calculateRemainingTime());
            auto eta           = std::format("{}", remainingTime);

            switch (status) {
            case Status::Ongoing: {
                fmt::print(stderr, "\r\033[K");    // carriage return and clear line
                fmt::print(stderr, "{}{:#>{}}{:->{}}{}", s_startChar, "", filledSize, "", emptySize, s_endChar);
                fmt::print(stderr, " ({}) {:.2f}% (ETA: {})", s_spinner[std::size_t(m_spinnerIdx)], ratio * 100, eta);
                break;
            }
            case Status::Completed: {
                fmt::print(stderr, "\r\033[K");    // carriage return and clear line
                fmt::print(stderr, "{}{:#>{}}{:->{}}{}", s_startChar, "", width, "", 0, s_endChar);
                fmt::print(stderr, " (#) {:.2f}%", 100.0);
                break;
            }
            case Status::Stopped: {
                fmt::print(stderr, "\r\033[K");    // carriage return and clear line
                fmt::print(stderr, "{}{:#>{}}{:->{}}{}", s_startChar, "", filledSize, "", emptySize, s_endChar);
                fmt::print(stderr, " (#) stopped at {:.2f}%", ratio * 100);
                break;
            }
            case Status::NotStarted: [[unlikely]] break;
            }
        }

        TimeInterval calculateRemainingTime() const
        {
            auto remaining    = (m_max - m_min) - m_current;
            auto [time, diff] = m_updateRecords.getAverage();
            auto speed        = (double)diff / (double)time.count();

            // fmt::println("remaining: {} speed: {}", remaining, speed);

            auto remainingTime = long((double)remaining / speed);
            return TimeInterval{ remainingTime };
        }

        inline static constexpr std::size_t s_width      = 80;
        inline static constexpr char        s_startChar  = '[';
        inline static constexpr char        s_endChar    = ']';
        inline static constexpr char        s_filledChar = '#';
        inline static constexpr char        s_emptyChar  = '-';
        inline static constexpr std::array  s_spinner    = { '/', '-', '\\', '|' };

        inline static constexpr TimeInterval s_delay{ 50 };
        inline static constexpr TimeInterval s_timeout{ 2000 };

        std::jthread m_thread;

        int m_max;
        int m_min;

        std::atomic<int>               m_current{ 0 };
        std::atomic<Clock::time_point> m_deadline;

        std::size_t m_spinnerIdx{ 0 };
        Status      m_status{ Status::NotStarted };

        Clock::time_point               m_startTime;
        MovingAverage<UpdateRecord, 20> m_updateRecords;
    };

}
