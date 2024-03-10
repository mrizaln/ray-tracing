#pragma once

#include "rtr/common.hpp"
#include "rtr/concepts.hpp"

#include <concurrencpp/concurrencpp.h>
#include <fmt/core.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <mutex>
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

    class ProgressBarEntry
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

        ProgressBarEntry(std::string name, int min, int max)
            : m_name{ std::move(name) }
            , m_min{ min }
            , m_max{ max }
        {
        }

        ProgressBarEntry(const ProgressBarEntry&)            = default;
        ProgressBarEntry& operator=(const ProgressBarEntry&) = default;
        ProgressBarEntry(ProgressBarEntry&&)                 = default;
        ProgressBarEntry& operator=(ProgressBarEntry&&)      = default;

        void update(int current)
        {
            auto last = m_current;
            m_current = std::clamp(current, m_min, m_max);

            auto currentTime = Clock::now();
            auto deltaTime   = currentTime - m_lastUpdate;
            m_lastUpdate     = currentTime;

            auto diff        = std::max(0, current - last);
            auto deltaTimeMs = std::chrono::duration_cast<TimeInterval>(deltaTime);

            m_updateRecords.update({
                .m_time = deltaTimeMs,
                .m_diff = diff,
            });

            m_spinnerIdx = (m_spinnerIdx + 1) % s_spinner.size();
        }

        void print() const
        {
            constexpr auto width = s_width - 10;    // hard-coded for now

            auto ratio      = (float)(m_current - m_min) / float(m_max - m_min);
            auto percentage = ratio * 100;

            auto filledSize = std::size_t(ratio * width);
            auto emptySize  = width - filledSize;

            using Seconds = std::chrono::duration<double>;
            auto eta      = std::chrono::duration_cast<Seconds>(calculateRemainingTime());

            fmt::print(stderr, "\r\033[2K");    // carriage return and clear line
            fmt::print(stderr, "{0:<{1}.{1}s}: [{2:#>{3}}{4:->{5}}]", m_name, width / 4, "", filledSize, "", emptySize);

            if (percentage != 100) {
                fmt::print(stderr, " ({0}) {1:.2f}% ({2:.2f}s)\n", s_spinner[m_spinnerIdx], percentage, eta.count());
            } else {
                fmt::print(stderr, " done\n");
            }
        }

        const std::string& name() const { return m_name; }

    private:
        TimeInterval calculateRemainingTime() const
        {
            auto remaining    = m_max - m_current;
            auto [time, diff] = m_updateRecords.getAverage();
            auto speed        = (double)diff / (double)time.count();
            if (speed == 0) {
                return TimeInterval{ 0 };
            }

            auto remainingTime = long((double)remaining / speed);
            return TimeInterval{ remainingTime };
        }

        inline static constexpr std::size_t s_width   = 80;
        inline static constexpr std::array  s_spinner = { '/', '-', '\\', '|' };

        inline static constexpr TimeInterval s_delay{ 100 };
        inline static constexpr TimeInterval s_timeout{ 2000 };

        std::string m_name;

        int               m_min;
        int               m_max;
        int               m_current    = 0;
        std::size_t       m_spinnerIdx = 0;
        Clock::time_point m_lastUpdate;

        MovingAverage<UpdateRecord, 10> m_updateRecords;
    };

    class ProgressBarManager
    {
    public:
        using Executor = concurrencpp::worker_thread_executor;

        ProgressBarManager(concurrencpp::runtime& runtime)
            : m_executor{ runtime.make_worker_thread_executor() }
        {
        }

        ~ProgressBarManager()
        {
            fmt::println(stderr, "\033[{}B", m_entries.size());    // move cursor down
            stop();
        }

        void add(std::string name, int min, int max)
        {
            std::scoped_lock lock{ m_mutex };
            m_entries.emplace_back(name, min, max);
        }

        void update(std::string name, int current)
        {
            m_executor->post([=, this, name = std::move(name)] {
                std::scoped_lock lock{ m_mutex };
                auto             found = std::find_if(m_entries.begin(), m_entries.end(), [&](const auto& entry) {
                    return entry.name() == name;
                });
                if (found == m_entries.end()) {
                    return;
                }
                found->update(current);
            });
        }

        void start(concurrencpp::runtime& runtime)
        {
            fmt::print(stderr, "\0337");    // DECSC
            m_timer = runtime.timer_queue()->make_timer(0s, 100ms, m_executor, [this] { printLoop(); });
        }

        void stop()
        {
            m_timer.cancel();
            m_executor->shutdown();
        }

    private:
        void printLoop()
        {
            auto entries = [this] {
                std::scoped_lock lock{ m_mutex };
                return m_entries;
            }();

            if (entries.empty()) {
                fmt::println("No progress bars to print");
                fmt::print(stderr, "\033[1A");
                return;
            } else {
                for (auto& entry : entries) {
                    entry.print();
                }
            }
            fmt::print(stderr, "\033[{}A", entries.size());    // move cursor up
        }

        std::shared_ptr<Executor> m_executor;
        std::mutex                m_mutex;

        concurrencpp::timer           m_timer;
        std::vector<ProgressBarEntry> m_entries;
    };

}
