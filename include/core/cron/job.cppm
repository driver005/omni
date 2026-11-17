export module core_cron:job;

import std;
import :expression;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cron {

/** @brief A named job: an opaque name bound to a parsed schedule. */
class CronJob
{
public:
    using TimePoint = CronExpression::TimePoint;

    CronJob() = default;

    CronJob(std::string name, std::string expression, std::optional<CronExpression> parsed) :
        m_name{std::move(name)},
        m_expression{std::move(expression)},
        m_parsed{std::move(parsed)}
    {
    }

    [[nodiscard]] std::optional<CronJob> create(
        std::string_view name,
        std::string_view expression
    )
    {
        if (name.empty()) {
            return std::nullopt;
        }
        auto parsed = CronExpression{}.parse(expression);
        if (!parsed) {
            return std::nullopt;
        }
        return CronJob{std::string{name}, std::string{expression}, std::move(parsed)};
    }

    [[nodiscard]] const std::string& name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::string& expression() const noexcept
    {
        return m_expression;
    }

    [[nodiscard]] const CronExpression& schedule() const noexcept
    {
        return *m_parsed;
    }

    [[nodiscard]] std::optional<TimePoint> next_after(TimePoint base) const noexcept
    {
        return m_parsed ? m_parsed->next_after(base) : std::nullopt;
    }

private:
    std::string m_name{};
    std::string m_expression{};
    std::optional<CronExpression> m_parsed{};
};

} // namespace core::cron

#ifdef CONGELADO_TEST
namespace core::cron::tests {
using namespace boost::ut;

suite<"core_cron_job"> core_cron_job_suite = []
{
    "creates named jobs from valid expressions"_test = []
    {
        const auto job = CronJob{}.create("nightly", "0 0 * * *");
        expect(job.has_value());
        expect(job->name() == "nightly");
        expect(job->expression() == "0 0 * * *");
        expect(!CronJob{}.create("", "* * * * *").has_value());
        expect(!CronJob{}.create("bad", "nope").has_value());
    };
};

} // namespace core::cron::tests
#endif
