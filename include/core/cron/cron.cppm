export module core_cron:cron;

import std;
import :backend;
import :engine;
import :expression;
import :job;
import :fire_event;
import :job_meta;
import :query;
import :query_spec;
import :runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

// CronRuntime is forward-declared (exported) by :cron_runtime.
export namespace core::cron {

/**
 * @brief The only handle a consumer holds. All members defined inline —
 * no free functions, no static functions, no out-of-line namespace block.
 */
class Cron
{
public:
    using TimePoint = CronExpression::TimePoint;
    using FireCallback = std::function<void(std::string_view)>;

    Cron() :
        Cron(CronEngine::LOCAL)
    {
    }

    explicit Cron(CronEngine engine) :
        m_runtime{std::make_shared<CronRuntime>(nullptr)}
    {
    }

    void use_backend(std::unique_ptr<CronBackend> backend)
    {
        m_runtime = std::make_shared<CronRuntime>(std::move(backend));
    }

    Cron(const Cron&) = delete;
    Cron& operator=(const Cron&) = delete;
    Cron(Cron&&) noexcept = default;
    Cron& operator=(Cron&&) noexcept = default;
    ~Cron() = default;

    [[nodiscard]] bool is_open() const noexcept
    {
        return m_runtime->is_open();
    }

    [[nodiscard]] std::string_view engine_name() const noexcept
    {
        return m_runtime->name();
    }

    [[nodiscard]] bool validate(std::string_view expression) const noexcept
    {
        return CronExpression{}.parse(expression).has_value();
    }

    [[nodiscard]] std::optional<TimePoint>
    next_after(std::string_view expression, TimePoint base) const noexcept
    {
        const auto parsed = CronExpression{}.parse(expression);
        return parsed ? parsed->next_after(base) : std::nullopt;
    }

    [[nodiscard]] std::vector<TimePoint>
    next_runs(std::string_view expression, TimePoint base, std::size_t count) const
    {
        const auto parsed = CronExpression{}.parse(expression);
        return parsed ? parsed->next_runs(base, count) : std::vector<TimePoint>{};
    }

    void set_fire_callback(FireCallback callback)
    {
        m_runtime->driver().set_fire_callback(std::move(callback));
    }

    [[nodiscard]] std::optional<CronJob>
    schedule(std::string_view name, std::string_view expression)
    {
        auto job = CronJob{}.create(name, expression);
        if (!job) {
            return std::nullopt;
        }
        m_runtime->driver().upsert(*job);
        return job;
    }

    template<reflect::ReflectedCronJob Job>
    [[nodiscard]] std::optional<CronJob> schedule()
    {
        return schedule(reflect::CronJobMeta<Job>::name, reflect::CronJobMeta<Job>::expression);
    }

    bool remove(std::string_view job_name)
    {
        return m_runtime->driver().remove(job_name);
    }

    [[nodiscard]] std::vector<CronJob> jobs() const
    {
        return m_runtime->driver().jobs();
    }

    [[nodiscard]] CronQuery query() const
    {
        return CronQuery{};
    }

    [[nodiscard]] std::vector<CronJob> execute(const CronQuerySpec& spec) const
    {
        std::vector<CronJob> result;
        for (const CronJob& job: m_runtime->driver().jobs()) {
            if (spec.matches(job)) {
                result.push_back(job);
                if (spec.limit_count && result.size() >= *spec.limit_count) {
                    break;
                }
            }
        }
        return result;
    }

    [[nodiscard]] std::vector<FireEvent> poll_due(TimePoint now = std::chrono::system_clock::now())
    {
        return m_runtime->driver().poll_due(now);
    }

    void close() noexcept
    {
        m_runtime->driver().close();
    }

private:
    std::shared_ptr<CronRuntime> m_runtime{};
};

} // namespace core::cron

#ifdef CONGELADO_TEST
namespace core::cron::tests {

struct FacadeJob
{
    void run() {}
};

CORE_CRON_JOB(FacadeJob, "facade-job", "0 3 * * *");

using namespace boost::ut;

suite<"core_cron_cron"> core_cron_cron_suite = []
{
    "pure helpers work without registration"_test = []
    {
        Cron cron;
        expect(cron.validate("*/5 * * * *"));
        expect(!cron.validate("bogus"));
        const auto base =
            std::chrono::sys_days{std::chrono::year{2'026} / std::chrono::January / 1};
        expect(cron.next_after("0 0 * * *", base).has_value());
        expect(cron.next_runs("0 0 * * *", base, 2).size() == 2);
        expect(reflect::CronJobMeta<FacadeJob>::name == "facade-job");
    };
};

} // namespace core::cron::tests
#endif
