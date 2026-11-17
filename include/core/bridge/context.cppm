export module core_bridge:context;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::bridge {

/** @brief Accumulates builder configuration for the bridge runtime. */
class BridgeContext
{
public:
    void set_option(std::string key, std::string value)
    {
        m_options.insert_or_assign(std::move(key), std::move(value));
    }

    [[nodiscard]] const std::map<std::string, std::string>& options() const noexcept
    {
        return m_options;
    }

private:
    std::map<std::string, std::string> m_options{};
};

} // namespace core::bridge

#ifdef CONGELADO_TEST
namespace core::bridge::tests {
using namespace boost::ut;

suite<"core_bridge_context"> core_bridge_context_suite = []
{
    "stores options"_test = []
    {
        BridgeContext context;
        context.set_option("base_dir", "./plugins");
        expect(context.options().at("base_dir") == "./plugins");
    };
};

} // namespace core::bridge::tests
#endif
