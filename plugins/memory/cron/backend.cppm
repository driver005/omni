export module memory_backends:cron;

import std;
import core_cron;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::cron {

/** @brief Reference engine: in-process job table driven by explicit polling. */
class LocalCronBackend final : public CronBackend
{
public:
    [[nodiscard]] std::string_view name() const noexcept override
    {
        return "local";
    }

    [[nodiscard]] bool is_open() const noexcept override
    {
        return m_open;
    }

    void open() override
    {
        m_open = true;
    }

    void close() override
    {
        m_open = false;
        m_jobs.clear();
    }

    void set_fire_callback(FireCallback callback) override
    {
        m_callback = std::move(callback);
    }

    void upsert(const CronJob& job) override
    {
        const auto now = std::chrono::system_clock::now();
        const auto it = m_jobs.find(job.name());
        if (it != m_jobs.end()) {
            // Re-registration replaces the expression but keeps firing history.
            it->second.job = job;
            it->second.next_fire = job.next_after(now);
            return;
        }
        m_jobs.emplace(job.name(), Entry{job, job.next_after(now)});
    }

    bool remove(std::string_view job_name) override
    {
        return m_jobs.erase(std::string{job_name}) != 0;
    }

    [[nodiscard]] std::vector<CronJob> jobs() const override
    {
        std::vector<CronJob> result;
        result.reserve(m_jobs.size());
        for (const auto& [name, entry]: m_jobs) {
            result.push_back(entry.job);
        }
        return result;
    }

    [[nodiscard]] std::vector<FireEvent> poll_due(TimePoint now) override
    {
        std::vector<FireEvent> events;
        for (auto& [name, entry]: m_jobs) {
            if (!entry.next_fire || *entry.next_fire > now) {
                continue;
            }
            const TimePoint due = *entry.next_fire;
            events.emplace_back(name, due, now);
            entry.next_fire = entry.job.next_after(now);
            if (m_callback) {
                m_callback(name);
            }
        }
        return events;
    }

private:
    struct Entry
    {
        CronJob job{};
        std::optional<TimePoint> next_fire{};
    };

    std::map<std::string, Entry> m_jobs{};
    FireCallback m_callback{};
    bool m_open{false};
};

} // namespace core::cron

#ifdef CONGELADO_TEST
namespace core::cron::tests {
using namespace boost::ut;

suite<"core_cron_local_backend"> core_cron_local_backend_suite = []
{
    "registers jobs and fires due ones"_test = []
    {
        LocalCronBackend backend;
        backend.open();
        expect(backend.is_open());
        expect(backend.name() == "local");

        std::vector<std::string> fired;
        backend.set_fire_callback([&](std::string_view name) { fired.emplace_back(name); });

        const auto job = CronJob{}.create("minutely", "* * * * *");
        expect(job.has_value());
        backend.upsert(*job);
        expect(backend.jobs().size() == 1);

        const auto far_future = std::chrono::system_clock::now() + std::chrono::hours{24};
        const auto events = backend.poll_due(far_future);
        expect(!events.empty());
        expect(fired.size() == events.size());
        expect(fired.front() == "minutely");

        expect(backend.remove("minutely"));
        expect(backend.jobs().empty());
        expect(!backend.remove("missing"));
    };
};

} // namespace core::cron::tests
#endif
