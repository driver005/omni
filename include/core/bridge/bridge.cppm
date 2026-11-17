export module core_bridge:bridge;

import std;
import :bridge_status;
import :service_key;
import :service_registry;
import :bridge_runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

// detail::BridgeRuntime is forward-declared (exported) by :bridge_runtime;
// a shared_ptr member needs only that incomplete declaration.
export namespace core::bridge {

/**
 * @brief The only handle a consumer holds. It owns a POCO-backed service
 * registry and plugin loader; neither Poco::SharedLibrary nor the registry
 * implementation is reachable from outside this module. Every member is
 * defined inline — no out-of-line definitions, no free helper functions.
 */
class Bridge
{
public:
    Bridge() :
        m_runtime{std::make_shared<detail::BridgeRuntime>()}
    {
        m_runtime->set_owner(this);
    }

    Bridge(const Bridge&) = delete;
    Bridge& operator=(const Bridge&) = delete;
    Bridge(Bridge&&) noexcept = default;
    Bridge& operator=(Bridge&&) noexcept = default;
    ~Bridge() = default;

    template<typename T>
    BridgeStatus register_service(const ServiceKey& key, T value)
    {
        return m_runtime->registry().register_service(key, std::move(value));
    }

    template<typename T>
    [[nodiscard]] std::optional<T> resolve(const ServiceKey& key) const
    {
        return m_runtime->registry().template resolve<T>(key);
    }

    [[nodiscard]] bool has_service(const ServiceKey& key) const noexcept
    {
        return m_runtime->registry().contains(key);
    }

    [[nodiscard]] std::size_t service_count() const noexcept
    {
        return m_runtime->registry().size();
    }

    [[nodiscard]] BridgeStatus load_plugin(std::string path)
    {
        return m_runtime->load_plugin(std::move(path));
    }

    bool unload_plugin(std::string_view path)
    {
        return m_runtime->unload_plugin(path);
    }

    [[nodiscard]] std::size_t plugin_count() const noexcept
    {
        return m_runtime->plugin_count();
    }

    void unload_all() noexcept
    {
        m_runtime->unload_all();
    }

private:
    std::shared_ptr<detail::BridgeRuntime> m_runtime{};
};

} // namespace core::bridge

#ifdef CONGELADO_TEST
namespace core::bridge::tests {
using namespace boost::ut;

suite<"core_bridge_bridge"> core_bridge_bridge_suite = []
{
    "service registry works through the handle"_test = []
    {
        Bridge bridge;
        const auto key = ServiceKey{}.make<int>("n");
        expect(bridge.register_service(key, 7) == BridgeStatus::OK);
        expect(bridge.resolve<int>(key).value_or(0) == 7);
        expect(bridge.service_count() == 1);
    };
};

} // namespace core::bridge::tests
#endif
