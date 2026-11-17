export module core_cron:cron_predicate;

import std;
import :cron_job;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cron {

/** @brief Comparison operators for cron job queries. */
enum class CronCompare : std::uint8_t
{
    EQ,
    NE,
    CONTAINS
};

/** @brief One predicate evaluated against a registered job. */
class CronPredicate
{
public:
    CronPredicate() = default;

    CronPredicate(std::string field, CronCompare comparison, std::string value) :
        m_field{std::move(field)},
        m_comparison{comparison},
        m_value{std::move(value)}
    {
    }

    [[nodiscard]] bool matches(const CronJob& job) const noexcept
    {
        std::string_view actual;
        if (m_field == "name") {
            actual = job.name();
        } else if (m_field == "expression") {
            actual = job.expression();
        } else {
            return false;
        }
        switch (m_comparison) {
            case CronCompare::EQ:
                return actual == m_value;
            case CronCompare::NE:
                return actual != m_value;
            case CronCompare::CONTAINS:
                return actual.find(m_value) != std::string_view::npos;
        }
        return false;
    }

private:
    std::string m_field{};
    CronCompare m_comparison{CronCompare::EQ};
    std::string m_value{};
};

} // namespace core::cron

#ifdef CONGELADO_TEST
namespace core::cron::tests {
using namespace boost::ut;

suite<"core_cron_predicate"> core_cron_predicate_suite = []
{
    "matches job name and expression"_test = []
    {
        const auto job = CronJob::create("nightly-backup", "0 2 * * *");
        expect(job.has_value());
        expect(CronPredicate{"name", CronCompare::EQ, "nightly-backup"}.matches(*job));
        expect(CronPredicate{"name", CronCompare::CONTAINS, "backup"}.matches(*job));
        expect(!CronPredicate{"expression", CronCompare::EQ, "* * * * *"}.matches(*job));
        expect(!CronPredicate{"bogus", CronCompare::EQ, "x"}.matches(*job));
    };
};

} // namespace core::cron::tests
#endif
