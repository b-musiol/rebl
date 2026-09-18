#ifndef PROGRESS_BAR_HPP
#define PROGRESS_BAR_HPP

#include <algorithm>
#include <chrono>
#include <format>
#include <iostream>
#include <string>
#include <string_view>

static constexpr const unsigned long long SECONDS_PER_HOUR   = 3600;
static constexpr const unsigned long long SECONDS_PER_MINUTE = 60;

namespace ProgressBar
{

class Instance
{
  public:
    Instance(unsigned long long total,
             std::string_view subject      = "",
             unsigned long long bar_length = 40,
             unsigned long long ctr_start  = 0,
             bool show_eta                 = true,
             char bar_character            = '#',
             char non_bar_character        = '-')
        : m_bar_length(bar_length), m_total(total), m_iteration(ctr_start),
          m_show_eta(show_eta), m_start_time(std::chrono::system_clock::now()),
          m_subject(subject), m_bar_character(bar_character),
          m_non_bar_character(non_bar_character)
    {
    }
    ~Instance() = default;

  public:
    void tick_up(bool silent              = false,
                 bool advance_ctr         = true,
                 long long advance_ctr_by = 1,
                 bool autoset_success     = false,
                 bool overflow_is_success = true)
    {
        if (advance_ctr) { m_iteration += advance_ctr_by; }
        if (autoset_success)
        {
            if (overflow_is_success) m_success = m_iteration >= m_total;
            else m_success = m_iteration == m_total;
        }
        if (!silent)
        {
            double percent = 100.0 * static_cast<double>(m_iteration) /
                             static_cast<double>(m_total);
            unsigned long long filled = m_bar_length * m_iteration / m_total;
            std::string bar;
            bar.reserve(m_bar_length);
            for (size_t i = 0; i < filled; i++) bar.push_back(m_bar_character);
            for (size_t i = 0; i < (m_bar_length - filled); i++)
            {
                bar.push_back(m_non_bar_character);
            }
            std::string eta_string;
            if (m_show_eta) { eta_string = std::format(" [{}]", format_eta()); }
            std::cout << '\r' << m_subject << ": |" << bar << "| " << percent
                      << "% " << eta_string;
        }
    }

    std::string format_eta()
    {
        if (m_iteration == 0) { return "--:--:--"; }
        auto elapsed = std::chrono::system_clock::now().time_since_epoch() -
                       m_start_time.time_since_epoch();
        auto full_seconds =
            std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
        auto rate      = static_cast<double>(full_seconds) / m_iteration;
        auto remaining = rate * (m_total - m_iteration);
        auto hours =
            static_cast<unsigned long long>(remaining) / SECONDS_PER_HOUR;
        auto rem =
            static_cast<unsigned long long>(remaining) % SECONDS_PER_HOUR;
        auto minutes = rem / SECONDS_PER_MINUTE;
        auto seconds = rem % SECONDS_PER_MINUTE;
        auto h_width = std::max<size_t>(2, std::to_string(hours).size());

        std::ostringstream eta_string;
        eta_string << std::setfill('0') << std::setw(static_cast<int>(h_width))
                   << hours << ':' << std::setw(2) << minutes << ':'
                   << std::setw(2) << seconds;

        return eta_string.str();
    }

    void flush()
    {
        long long eta_padding =
            m_show_eta ? static_cast<std::int64_t>(11 + m_subject.size() + 2)
                       : 0;

        std::cout << '\r' << m_subject << ' ' << (m_success ? "Done" : "Failed")
                  << '!'
                  << std::string(
                         static_cast<std::size_t>(
                             std::max<std::int64_t>(0, m_bar_length + 4)),
                         ' ')
                  << std::string(static_cast<std::size_t>(
                                     std::max<std::int64_t>(0, eta_padding)),
                                 ' ')
                  << "\n\n"
                  << std::flush;
    }

  public:
    unsigned long long m_bar_length;
    unsigned long long m_total;
    bool m_success{};
    unsigned long long m_iteration;
    bool m_show_eta;
    std::chrono::system_clock::time_point m_start_time;
    std::string m_subject;
    char m_bar_character, m_non_bar_character;

  private:
};

} // namespace ProgressBar

#endif // PROGRESS_BAR_HPP