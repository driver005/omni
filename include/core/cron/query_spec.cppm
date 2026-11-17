export module core_cron:query_spec;

import std;
import :job;
import :predicate;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cron {

/** @brief Plain, resource-free description of a registered-job lookup. */
class CronQuerySpec
{
public:
    std::vector<CronPredicate> predicates{};
    std::optional<std::size_t> limit_count{};

    [[nodiscard]] bool matches(const CronJob& job) const noexcept
    {
        return std::ranges::all_of(
            predicates,
            [&](const CronPredicate& predicate)
            {
                return predicate.matches(job);
            }
        );
    }
};

} // namespace core::cron

#ifdef CONGELADO_TEST
namespace core::cron::tests {
using namespace boost::ut;

suite<"core_cron_query_spec"> core_cron_query_spec_suite = []
{
    "collects predicates and matches jobs"_test = []
    {
        const auto job = CronJob{}.create("nightly", "0 2 * * *");
        expect(job.has_value());
        CronQuerySpec spec;
        spec.predicates.emplace_back("name", CronCompare::CONTAINS, "night");
        expect(spec.matches(*job));
        spec.limit_count = 5;
        expect(spec.limit_count == 5);
    };
};

} // namespace core::cron::tests
#endif
