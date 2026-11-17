export module core_bridge:runtime;

import std;
import :status;
import :service_key;
import :service_registry;
import :plugin_module;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::bridge {
class Bridge; // facade passes itself as plugin context
}

export namespace core::bridge {
class BridgeRuntime;
}

namespace core::bridge {

/** @brief Internal state owned by the facade: registry + loaded plugins. */
class BridgeRuntime
{
public:
    BridgeRuntime() = default;
    BridgeRuntime(const BridgeRuntime&) = delete;
    BridgeRuntime& operator=(const BridgeRuntime&) = delete;
    BridgeRuntime(BridgeRuntime&&) noexcept = default;
    BridgeRuntime& operator=(BridgeRuntime&&) noexcept = default;

    void set_owner(Bridge* owner) noexcept
    {
        m_owner = owner;
    }

    ServiceRegistry& registry() noexcept
    {
        return m_registry;
    }

    [[nodiscard]] const ServiceRegistry& registry() const noexcept
    {
        return m_registry;
    }

    [[nodiscard]] BridgeStatus load_plugin(std::string path)
    {
        if (m_plugins.contains(path)) {
            return BridgeStatus::ALREADY_LOADED;
        }
        auto module = std::make_unique<PluginModule>(std::move(path));
        const BridgeStatus status = module->load();
        if (status != BridgeStatus::OK) {
            return status;
        }
        const std::string key = module->path();
        module->register_plugin(m_owner); // plugin receives the public Bridge* context
        m_plugins.emplace(key, std::move(module));
        return BridgeStatus::OK;
    }

    bool unload_plugin(std::string_view path)
    {
        const auto it = m_plugins.find(std::string{path});
        if (it == m_plugins.end()) {
            return false;
        }
        it->second->unload();
        m_plugins.erase(it);
        return true;
    }

    [[nodiscard]] std::size_t plugin_count() const noexcept
    {
        return m_plugins.size();
    }

    void unload_all() noexcept
    {
        for (auto& [path, module]: m_plugins) {
            module->unload();
        }
        m_plugins.clear();
    }

private:
    ServiceRegistry m_registry{};
    std::map<std::string, std::unique_ptr<PluginModule>> m_plugins{};
    Bridge* m_owner{nullptr};
};

} // namespace core::bridge

#ifdef CONGELADO_TEST
namespace core::bridge::tests {
using namespace boost::ut;

suite<"core_bridge_runtime"> core_bridge_runtime_suite = []
{
    "owns a registry and rejects missing plugins"_test = []
    {
        BridgeRuntime runtime;
        expect(runtime.plugin_count() == 0);
        expect(runtime.load_plugin("/nonexistent/x.so") == BridgeStatus::LOAD_FAILED);
        expect(runtime.plugin_count() == 0);
        expect(!runtime.unload_plugin("missing"));
        runtime.unload_all();
        expect(runtime.plugin_count() == 0);
    };
};

} // namespace core::bridge::tests
#endif
