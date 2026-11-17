export module core_cron:cron_query;

import std;
import :cron_expression;
import :cron_job;
import :cron_job_meta;
import :cron_query_spec;
import :cron_predicate;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cron {

/** @brief Fluent, compile-time-checked builder for registered-job queries. */
class CronQuery
{
public:
    CronQuery& where_name(CronCompare comparison, std::string value)
    {
        m_spec.predicates.emplace_back("name", comparison, std::move(value));
        return *this;
    }

    CronQuery& where_expression(CronCompare comparison, std::string value)
    {
        m_spec.predicates.emplace_back("expression", comparison, std::move(value));
        return *this;
    }

    /**
     * @brief Restricts to reflected cron jobs whose compile-time expression
     * matches the given string.
     */
    template<reflect::ReflectedCronJob Job>
    CronQuery& where_job()
    {
        return where_name(CronCompare::EQ, std::string{reflect::CronJobMeta<Job>::name});
    }

    CronQuery& limit(std::size_t count)
    {
        m_spec.limit_count = count;
        return *this;
    }

    [[nodiscard]] const CronQuerySpec& spec() const noexcept
    {
        return m_spec;
    }

private:
    CronQuerySpec m_spec{};
};

} // namespace core::cron

#ifdef CONGELADO_TEST
namespace core::cron::tests {

struct ScheduledJob
{
    static void run() {}
};
CORE_CRON_JOB(ScheduledJob, "scheduled", "*/5 * * * *");

using namespace boost::ut;

suite<"core_cron_query"> core_cron_query_suite = []
{
    "composes name, expression and reflected-job predicates"_test = []
    {
        CronQuery query;
        query.where_name(CronCompare::CONTAINS, "night")
            .where_expression(CronCompare::EQ, "0 2 * * *")
            .limit(3);
        expect(query.spec().predicates.size() == 2);
        expect(query.spec().limit_count == 3);

        CronQuery reflected;
        reflected.where_job<ScheduledJob>();
        expect(reflected.spec().predicates[0].matches(
            *CronJob::create("scheduled", "*/5 * * * *")
        ));
    };
};

} // namespace core::cron::tests
#endif
