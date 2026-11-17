export module core_cron:cron_context;

import std;
import :cron_engine;
import :cron_runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::cron::detail {

/** @brief Accumulates builder configuration and compiles it into a runtime. */
class CronContext
{
public:
    void set_engine(CronEngine engine) noexcept
    {
        m_engine = engine;
    }

    void set_option(std::string key, std::string value)
    {
        m_options.insert_or_assign(std::move(key), std::move(value));
    }

    [[nodiscard]] std::unique_ptr<CronRuntime> build() const
    {
        return std::make_unique<CronRuntime>(m_engine);
    }

    [[nodiscard]] CronEngine engine() const noexcept
    {
        return m_engine;
    }

    [[nodiscard]] const std::map<std::string, std::string>& options() const noexcept
    {
        return m_options;
    }

private:
    CronEngine m_engine{CronEngine::LOCAL};
    std::map<std::string, std::string> m_options{};
};

} // namespace core::cron::detail

#ifdef CONGELADO_TEST
namespace core::cron::detail::tests {
using namespace boost::ut;

suite<"core_cron_context"> core_cron_context_suite = []
{
    "accumulates config and builds a runtime"_test = []
    {
        CronContext context;
        context.set_engine(CronEngine::LOCAL);
        context.set_option("timezone", "UTC");
        expect(context.engine() == CronEngine::LOCAL);
        expect(context.options().at("timezone") == "UTC");
        auto runtime = context.build();
        expect(runtime->is_open());
    };
};

} // namespace core::cron::detail::tests
#endif
