module;
#include <Poco/Any.h>
#include <Poco/Exception.h>

export module core_bridge:service_registry;

import std;
import :bridge_status;
import :service_key;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::bridge {

/**
 * @brief Service registry backed by Poco::Any — the library does the type-erased
 * storage, so the bridge never hand-rolls type erasure. Hosts register core
 * facades; plugins resolve them (and may register their own) bidirectionally.
 */
class ServiceRegistry
{
public:
    template<typename T>
    BridgeStatus register_service(const ServiceKey& key, T value)
    {
        if (!key.matches(typeid(T))) {
            return BridgeStatus::TYPE_MISMATCH;
        }
        m_services[key.name()] = Poco::Any{std::move(value)};
        return BridgeStatus::OK;
    }

    template<typename T>
    [[nodiscard]] std::optional<T> resolve(const ServiceKey& key) const
    {
        if (!key.matches(typeid(T))) {
            return std::nullopt;
        }
        const auto it = m_services.find(key.name());
        if (it == m_services.end()) {
            return std::nullopt;
        }
        try {
            return Poco::AnyCast<T>(it->second);
        } catch (const Poco::BadCastException&) {
            return std::nullopt;
        }
    }

    [[nodiscard]] bool contains(const ServiceKey& key) const noexcept
    {
        return m_services.contains(key.name());
    }

    bool remove(const ServiceKey& key) noexcept
    {
        return m_services.erase(key.name()) != 0;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_services.size();
    }

    void clear() noexcept
    {
        m_services.clear();
    }

private:
    std::map<std::string, Poco::Any> m_services{};
};

} // namespace core::bridge

#ifdef CONGELADO_TEST
namespace core::bridge::tests {
using namespace boost::ut;

suite<"core_bridge_service_registry"> core_bridge_service_registry_suite = []
{
    "registers and resolves typed services"_test = []
    {
        ServiceRegistry registry;
        const auto key = ServiceKey::make<std::string>("greeting");
        expect(registry.register_service(key, std::string{"hello"}) == BridgeStatus::OK);
        expect(registry.contains(key));
        expect(registry.size() == 1);
        const auto value = registry.resolve<std::string>(key);
        expect(value.has_value());
        expect(*value == "hello");
        expect(!registry.resolve<int>(key).has_value());
    };

    "rejects mismatched key type and removes services"_test = []
    {
        ServiceRegistry registry;
        const auto int_key = ServiceKey::make<int>("n");
        expect(registry.register_service(int_key, std::string{"bad"}) ==
               BridgeStatus::TYPE_MISMATCH);
        expect(registry.register_service(int_key, 42) == BridgeStatus::OK);
        expect(registry.remove(int_key));
        expect(!registry.contains(int_key));
        registry.clear();
        expect(registry.size() == 0);
    };
};

} // namespace core::bridge::tests
#endif
