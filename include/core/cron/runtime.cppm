export module core_cron:runtime;

import std;
import :engine;
import :driver;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cron {
class CronRuntime;
}

namespace core::cron {

/** @brief Internal state owned by the facade: the single cron driver. */
class CronRuntime
{
public:
    explicit CronRuntime(CronEngine engine) :
        m_driver{engine}
    {
        m_driver.open();
    }

    CronRuntime(const CronRuntime&) = delete;
    CronRuntime& operator=(const CronRuntime&) = delete;
    CronRuntime(CronRuntime&&) noexcept = default;
    CronRuntime& operator=(CronRuntime&&) noexcept = default;

    [[nodiscard]] bool is_open() const noexcept
    {
        return m_driver.is_open();
    }

    [[nodiscard]] std::string_view name() const noexcept
    {
        return m_driver.name();
    }

    [[nodiscard]] CronDriver& driver() noexcept
    {
        return m_driver;
    }

private:
    CronDriver m_driver;
};

} // namespace core::cron

#ifdef CONGELADO_TEST
namespace core::cron::tests {
using namespace boost::ut;

suite<"core_cron_runtime"> core_cron_runtime_suite = []
{
    "owns an open driver"_test = []
    {
        CronRuntime runtime{CronEngine::LOCAL};
        expect(runtime.is_open());
        expect(runtime.name() == "local");
    };
};

} // namespace core::cron::tests
#endif
