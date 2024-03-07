#pragma once

#include <fmt/core.h>

#include <algorithm>
#include <atomic>
#include <array>
#include <chrono>
#include <functional>
#include <optional>
#include <stop_token>
#include <thread>

namespace rtr
{

    class ProgressBar
    {
    public:
        using Clock     = std::chrono::steady_clock;
        using TimePoint = std::chrono::time_point<Clock>;

        enum class StopReason
        {
            Completed,
            Stopped
        };

        ProgressBar(std::size_t max, std::size_t min = 0)
            : m_max{ max }
            , m_min{ min }
        {
        }

        // if current is not provided, it will be set to min
        void start(
            std::optional<std::size_t>                                               current    = {},
            std::function<void(TimePoint start, TimePoint end, StopReason reason)>&& onComplete = {}
        )
        {
            m_current = current ? *current : m_min;

            m_thread = std::jthread{ [this, onComplete = std::move(onComplete)](std::stop_token st) mutable {
                constexpr std::size_t spinnerSize = s_spinner.size();
                constexpr auto        width = s_width - 3 - 1 - 1 - 1;    // hard-coded for now
                std::size_t           count = 0;

                auto time1 = Clock::now();

                while (!st.stop_requested() && m_current <= m_max) {
                    auto ratio = (float)m_current / (float)m_max;

                    auto filledSize = std::size_t(ratio * width);
                    auto emptySize  = width - filledSize;

                    std::string filled(filledSize, s_filledChar);
                    std::string empty(emptySize, s_emptyChar);

                    fmt::print(stderr, "{}{}{}{}", s_startChar, filled, empty, s_endChar);
                    fmt::print(stderr, " ({}) {:.2f}%", s_spinner[count], ratio * 100);
                    fmt::print(stderr, "\r");    // carriage return

                    std::this_thread::sleep_for(s_delay);

                    count = ++count % spinnerSize;
                }
                std::putchar('\n');

                auto time2 = Clock::now();

                if (!onComplete) {
                    return;
                }

                if (st.stop_requested() && m_current < m_max) {
                    onComplete(time1, time2, StopReason::Stopped);
                } else {
                    onComplete(time1, time2, StopReason::Completed);
                }
            } };
        }

        void update(std::size_t current) { m_current = std::clamp(current, m_min, m_max); }
        void stop() { m_thread.request_stop(); }

    private:
        inline static constexpr std::size_t s_width      = 80;
        inline static constexpr char        s_startChar  = '[';
        inline static constexpr char        s_endChar    = ']';
        inline static constexpr char        s_filledChar = '#';
        inline static constexpr char        s_emptyChar  = '-';
        inline static constexpr std::array  s_spinner    = { '/', '-', '\\', '|' };

        inline static constexpr std::chrono::milliseconds s_delay{ 100 };

        std::jthread m_thread;

        std::size_t              m_max;
        std::size_t              m_min;
        std::atomic<std::size_t> m_current{ 0 };
    };

}
