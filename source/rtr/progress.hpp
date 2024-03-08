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

        enum class Status
        {
            NotStarted,
            Ongoing,
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
            std::optional<std::size_t>                                           current    = {},
            std::function<void(TimePoint start, TimePoint end, Status reason)>&& onComplete = {}
        )
        {
            m_current = current ? *current : m_min;
            m_status  = Status::Ongoing;

            m_thread = std::jthread{ [this, onComplete = std::move(onComplete)](std::stop_token st) mutable {
                auto time1 = Clock::now();

                while (!st.stop_requested() && m_current <= m_max) {
                    std::this_thread::sleep_for(s_delay);

                    print(m_status);

                    if (auto now = Clock::now(); now - time1 >= s_timeout) {
                        break;
                    }
                }

                auto time2 = Clock::now();

                if (!onComplete) {
                    return;
                }

                if (st.stop_requested() && m_current < m_max) {
                    m_status = Status::Stopped;
                    print(m_status);
                    std::putchar('\n');
                    onComplete(time1, time2, m_status);
                } else {
                    m_status = Status::Completed;
                    print(m_status);
                    std::putchar('\n');
                    onComplete(time1, time2, m_status);
                }
            } };
        }

        void update(std::size_t current) { m_current = std::clamp(current, m_min, m_max); }

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

            auto ratio   = (float)m_current / (float)m_max;
            m_spinnerIdx = ++m_spinnerIdx % static_cast<int>(spinnerSize);

            auto filledSize = std::size_t(ratio * width);
            auto emptySize  = width - filledSize;

            switch (status) {
            case Status::Ongoing: {
                fmt::print(stderr, "{}{:#>{}}{:->{}}{}", s_startChar, "", filledSize, "", emptySize, s_endChar);
                fmt::print(stderr, " ({}) {:.2f}%", s_spinner[static_cast<std::size_t>(m_spinnerIdx)], ratio * 100);
                fmt::print(stderr, "\r");    // carriage return
                break;
            }
            case Status::Completed: {
                fmt::print(stderr, "{}{:#>{}}{:->{}}{}", s_startChar, "", width, "", 0, s_endChar);
                fmt::print(stderr, " (#) {:.2f}%", 100.0);
                fmt::print(stderr, "\r");    // carriage return
                break;
            }
            case Status::Stopped: {
                fmt::print(stderr, "{}{:#>{}}{:->{}}{}", s_startChar, "", filledSize, "", emptySize, s_endChar);
                fmt::print(stderr, " (#) stopped at {:.2f}%", ratio * 100);
                fmt::print(stderr, "\r");    // carriage return
                break;
            }
            case Status::NotStarted: [[unlikely]] break;
            }
        }

        inline static constexpr std::size_t s_width      = 80;
        inline static constexpr char        s_startChar  = '[';
        inline static constexpr char        s_endChar    = ']';
        inline static constexpr char        s_filledChar = '#';
        inline static constexpr char        s_emptyChar  = '-';
        inline static constexpr std::array  s_spinner    = { '/', '-', '\\', '|' };

        inline static constexpr std::chrono::milliseconds s_delay{ 50 };
        inline static constexpr std::chrono::milliseconds s_timeout{ 2000 };

        std::jthread m_thread;

        std::size_t              m_max;
        std::size_t              m_min;
        std::atomic<std::size_t> m_current{ 0 };
        std::size_t              m_spinnerIdx{ 0 };
        Status                   m_status{ Status::NotStarted };
    };

}
