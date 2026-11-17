export module core_cron:engine;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cron {

/** @brief Selectable cron engines; the concrete timing loop stays internal. */
enum class CronEngine : std::uint8_t
{
    LOCAL,
    /** @brief Reserved for an external cron service wired behind the facade. */
    EXTERNAL
};

class CronEngineName
{
public:
    [[nodiscard]] constexpr std::string_view operator()(CronEngine engine) const noexcept
    {
        return engine == CronEngine::EXTERNAL ? "external" : "local";
    }
};

} // namespace core::cron

#ifdef CONGELADO_TEST
namespace core::cron::tests {
using namespace boost::ut;

suite<"core_cron_engine"> core_cron_engine_suite = []
{
    "exposes engine names only"_test = []
    {
        expect(CronEngineName{}(CronEngine::LOCAL) == "local");
        expect(CronEngineName{}(CronEngine::EXTERNAL) == "external");
    };
};

} // namespace core::cron::tests
#endif
