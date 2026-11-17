export module core_cron:builder;

import std;
import :engine;
import :context;
import :cron;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cron {

/** @brief Fluent entry point. Selects an engine, then builds an opaque Cron handle. */
class CronBuilder
{
public:
    CronBuilder& engine(CronEngine engine)
    {
        m_context.set_engine(engine);
        return *this;
    }

    CronBuilder& use_backend(std::unique_ptr<CronBackend> backend)
    {
        m_context.use_backend(std::move(backend));
        return *this;
    }

    CronBuilder& option(std::string key, std::string value)
    {
        m_context.set_option(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] Cron build() const
    {
        Cron cron;
        cron.use_backend(m_context.take_backend());
        return cron;
    }

private:
    CronContext m_context{};
};

} // namespace core::cron

#ifdef CONGELADO_TEST
namespace core::cron::tests {

struct BuiltCronJob
{
     void run() {}
};
CORE_CRON_JOB(BuiltCronJob, "built-cron-job", "*/5 * * * *");

using namespace boost::ut;

suite<"core_cron_cron_builder"> core_cron_cron_builder_suite = []
{
    "builds an open handle and schedules through the facade"_test = []
    {
        CronBuilder builder;
        Cron cron = builder.engine(CronEngine::LOCAL).option("timezone", "UTC").build();
        expect(cron.is_open());
        expect(cron.engine_name() == "local");

        std::vector<std::string> fired;
        cron.set_fire_callback([&](std::string_view name) { fired.emplace_back(name); });

        const auto job = cron.schedule<BuiltCronJob>();
        expect(job.has_value());
        expect(job->name() == "built-cron-job");
        expect(cron.jobs().size() == 1);

        auto query = cron.query();
        query.where_name(CronCompare::EQ, "built-cron-job");
        expect(cron.execute(query.spec()).size() == 1);

        const auto far_future = std::chrono::system_clock::now() + std::chrono::hours{24};
        const auto events = cron.poll_due(far_future);
        expect(!events.empty());
        expect(fired.front() == "built-cron-job");

        expect(cron.remove("built-cron-job"));
        expect(cron.jobs().empty());
    };
};

} // namespace core::cron::tests
#endif
