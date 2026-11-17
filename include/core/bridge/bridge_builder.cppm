export module core_bridge:bridge_builder;

import std;
import :bridge_context;
import :bridge;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::bridge {

/** @brief Fluent entry point. Builds a Bridge with no free helper functions. */
class BridgeBuilder
{
public:
    BridgeBuilder& option(std::string key, std::string value)
    {
        m_context.set_option(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] Bridge build() const
    {
        return Bridge{};
    }

private:
    detail::BridgeContext m_context{};
};

} // namespace core::bridge

#ifdef CONGELADO_TEST
namespace core::bridge::tests {
using namespace boost::ut;

struct BridgePayload
{
    int value{0};
};

suite<"core_bridge_bridge_builder"> core_bridge_bridge_builder_suite = []
{
    "builds a bridge and registers/resolves services"_test = []
    {
        Bridge bridge = BridgeBuilder{}.option("base_dir", "./plugins").build();
        const auto key = ServiceKey{}.make<BridgePayload>("payload");
        expect(bridge.register_service(key, BridgePayload{42}) == BridgeStatus::OK);
        expect(bridge.has_service(key));
        expect(bridge.service_count() == 1);
        expect(bridge.resolve<BridgePayload>(key)->value == 42);
        expect(bridge.load_plugin("/nonexistent/p.so") == BridgeStatus::LOAD_FAILED);
        expect(bridge.plugin_count() == 0);
    };
};

} // namespace core::bridge::tests
#endif
