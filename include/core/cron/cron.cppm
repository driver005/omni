export module core_cron:cron;

import std;
import :cron_engine;
import :cron_expression;
import :cron_job;
import :fire_event;
import :cron_job_meta;
import :cron_query;
import :cron_query_spec;
import :cron_runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cron {

/**
 * @brief The only handle a consumer ever holds. It hides the timing engine and
 * every job table: callers get typed scheduling, queries and polling, and
 * nothing that can reach the backend directly. Built by CronBuilder.
 */
class Cron
{
public:
    using TimePoint = CronExpression::TimePoint;
    using FireCallback = std::function<void(std::string_view)>;

    Cron(const Cron&) = delete;
    Cron& operator=(const Cron&) = delete;
    Cron(Cron&&) noexcept;
    Cron& operator=(Cron&&) noexcept;
    ~Cron();

    [[nodiscard]] bool is_open() const noexcept;
    [[nodiscard]] std::string_view engine_name() const noexcept;

    /** @brief Pure validation — no backend access. */
    [[nodiscard]] static bool validate(std::string_view expression) noexcept
    {
        return CronExpression::parse(expression).has_value();
    }

    /** @brief Pure next-fire computation — no backend access. */
    [[nodiscard]] static std::optional<TimePoint> next_after(
        std::string_view expression,
        TimePoint base
    ) noexcept
    {
        const auto parsed = CronExpression::parse(expression);
        return parsed ? parsed->next_after(base) : std::nullopt;
    }

    /** @brief Pure next-N computation — no backend access. */
    [[nodiscard]] static std::vector<TimePoint> next_runs(
        std::string_view expression,
        TimePoint base,
        std::size_t count
    )
    {
        const auto parsed = CronExpression::parse(expression);
        return parsed ? parsed->next_runs(base, count) : std::vector<TimePoint>{};
    }

    void set_fire_callback(FireCallback callback);

    [[nodiscard]] std::optional<CronJob> schedule(std::string_view name, std::string_view expression);

    template<reflect::ReflectedCronJob Job>
    [[nodiscard]] std::optional<CronJob> schedule()
    {
        return schedule(reflect::CronJobMeta<Job>::name, reflect::CronJobMeta<Job>::expression);
    }

    bool remove(std::string_view job_name);
    [[nodiscard]] std::vector<CronJob> jobs() const;

    [[nodiscard]] CronQuery query() const;
    [[nodiscard]] std::vector<CronJob> execute(const CronQuerySpec& spec) const;

    [[nodiscard]] std::vector<FireEvent> poll_due(TimePoint now = std::chrono::system_clock::now());

    void close() noexcept;

private:
    struct Runtime;
    std::unique_ptr<Runtime> m_runtime{};

    explicit Cron(std::unique_ptr<Runtime> runtime) noexcept;

    friend class CronBuilder;
};

} // namespace core::cron

// ─── Implementation (module purview — not reachable by importers) ────────────

namespace core::cron {

struct Cron::Runtime
{
    explicit Runtime(std::unique_ptr<detail::CronRuntime> value) noexcept :
        impl{std::move(value)}
    {
    }

    std::unique_ptr<detail::CronRuntime> impl{};
};

Cron::Cron(std::unique_ptr<Runtime> runtime) noexcept :
    m_runtime{std::move(runtime)}
{
}

Cron::Cron(Cron&&) noexcept = default;
Cron& Cron::operator=(Cron&&) noexcept = default;
Cron::~Cron() = default;

bool Cron::is_open() const noexcept
{
    return m_runtime && m_runtime->impl && m_runtime->impl->is_open();
}

std::string_view Cron::engine_name() const noexcept
{
    return m_runtime && m_runtime->impl ? m_runtime->impl->name() : "none";
}

void Cron::set_fire_callback(FireCallback callback)
{
    m_runtime->impl->driver().set_fire_callback(std::move(callback));
}

std::optional<CronJob> Cron::schedule(std::string_view name, std::string_view expression)
{
    auto job = CronJob::create(name, expression);
    if (!job) {
        return std::nullopt;
    }
    m_runtime->impl->driver().upsert(*job);
    return job;
}

bool Cron::remove(std::string_view job_name)
{
    return m_runtime->impl->driver().remove(job_name);
}

std::vector<CronJob> Cron::jobs() const
{
    return m_runtime->impl->driver().jobs();
}

CronQuery Cron::query() const
{
    return CronQuery{};
}

std::vector<CronJob> Cron::execute(const CronQuerySpec& spec) const
{
    std::vector<CronJob> result;
    for (const CronJob& job: m_runtime->impl->driver().jobs()) {
        if (spec.matches(job)) {
            result.push_back(job);
            if (spec.limit_count && result.size() >= *spec.limit_count) {
                break;
            }
        }
    }
    return result;
}

std::vector<FireEvent> Cron::poll_due(TimePoint now)
{
    return m_runtime->impl->driver().poll_due(now);
}

void Cron::close() noexcept
{
    if (m_runtime && m_runtime->impl) {
        m_runtime->impl->driver().close();
    }
}

} // namespace core::cron

#ifdef CONGELADO_TEST
namespace core::cron::tests {

struct FacadeJob
{
    static void run() {}
};
CORE_CRON_JOB(FacadeJob, "facade-job", "0 3 * * *");

using namespace boost::ut;

suite<"core_cron_cron"> core_cron_cron_suite = []
{
    "pure helpers work without a backend"_test = []
    {
        expect(Cron::validate("*/5 * * * *"));
        expect(!Cron::validate("bogus"));
        const auto base = std::chrono::sys_days{std::chrono::year{2026} / std::chrono::January / 1};
        expect(Cron::next_after("0 0 * * *", base).has_value());
        expect(Cron::next_runs("0 0 * * *", base, 2).size() == 2);
        expect(reflect::CronJobMeta<FacadeJob>::name == "facade-job");
    };
};

} // namespace core::cron::tests
#endif
