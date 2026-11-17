export module core_cron:cron_driver;

import std;
import :cron_engine;
import :cron_job;
import :fire_event;
import :cron_backend;
import :local_cron_backend;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::cron::detail {

/** @brief Owns the active backend. Internal to the module. */
class CronDriver
{
public:
    using FireCallback = CronBackend::FireCallback;
    using TimePoint = CronBackend::TimePoint;

    explicit CronDriver(CronEngine engine)
    {
        switch (engine) {
            case CronEngine::LOCAL:
                m_backend = std::make_unique<LocalCronBackend>();
                break;
            case CronEngine::EXTERNAL:
                throw std::runtime_error(
                    "core_cron: EXTERNAL engine is reserved; the plugin backend "
                    "has not been registered"
                );
        }
    }

    CronDriver(const CronDriver&) = delete;
    CronDriver& operator=(const CronDriver&) = delete;
    CronDriver(CronDriver&&) noexcept = default;
    CronDriver& operator=(CronDriver&&) noexcept = default;

    void open()
    {
        m_backend->open();
    }

    void close()
    {
        m_backend->close();
    }

    [[nodiscard]] bool is_open() const noexcept
    {
        return m_backend && m_backend->is_open();
    }

    [[nodiscard]] std::string_view name() const noexcept
    {
        return m_backend ? m_backend->name() : "none";
    }

    void set_fire_callback(FireCallback callback)
    {
        m_backend->set_fire_callback(std::move(callback));
    }

    void upsert(const CronJob& job)
    {
        m_backend->upsert(job);
    }

    bool remove(std::string_view job_name)
    {
        return m_backend->remove(job_name);
    }

    [[nodiscard]] std::vector<CronJob> jobs() const
    {
        return m_backend->jobs();
    }

    [[nodiscard]] std::vector<FireEvent> poll_due(TimePoint now)
    {
        return m_backend->poll_due(now);
    }

private:
    std::unique_ptr<CronBackend> m_backend{};
};

} // namespace core::cron::detail

#ifdef CONGELADO_TEST
namespace core::cron::detail::tests {
using namespace boost::ut;

suite<"core_cron_driver"> core_cron_driver_suite = []
{
    "selects and drives the local backend"_test = []
    {
        CronDriver driver{CronEngine::LOCAL};
        driver.open();
        expect(driver.is_open());
        expect(driver.name() == "local");
        const auto job = CronJob::create("j", "* * * * *");
        expect(job.has_value());
        driver.upsert(*job);
        expect(driver.jobs().size() == 1);
        expect(driver.remove("j"));
        driver.close();
        expect(!driver.is_open());
    };

    "external engine is rejected until a plugin registers"_test = []
    {
        expect(throws<std::runtime_error>([] { CronDriver driver{CronEngine::EXTERNAL}; }));
    };
};

} // namespace core::cron::detail::tests
#endif
