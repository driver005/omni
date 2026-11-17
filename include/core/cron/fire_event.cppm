export module core_cron:fire_event;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cron {

/** @brief One scheduled firing handed to the registered fire callback. */
class FireEvent
{
public:
    using TimePoint = std::chrono::system_clock::time_point;

    FireEvent() = default;

    FireEvent(std::string job_name, TimePoint due, TimePoint fired) :
        m_job_name{std::move(job_name)},
        m_due{due},
        m_fired{fired}
    {
    }

    [[nodiscard]] const std::string& job_name() const noexcept
    {
        return m_job_name;
    }

    [[nodiscard]] TimePoint due() const noexcept
    {
        return m_due;
    }

    [[nodiscard]] TimePoint fired() const noexcept
    {
        return m_fired;
    }

private:
    std::string m_job_name{};
    TimePoint m_due{};
    TimePoint m_fired{};
};

} // namespace core::cron

#ifdef CONGELADO_TEST
namespace core::cron::tests {
using namespace boost::ut;

suite<"core_cron_fire_event"> core_cron_fire_event_suite = []
{
    "carries job name and timestamps"_test = []
    {
        const auto now = std::chrono::system_clock::now();
        const FireEvent event{"nightly", now, now + std::chrono::seconds{1}};
        expect(event.job_name() == "nightly");
        expect(event.due() == now);
        expect(event.fired() == now + std::chrono::seconds{1});
    };
};

} // namespace core::cron::tests
#endif
