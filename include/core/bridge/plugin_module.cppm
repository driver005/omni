module;
#include <Poco/SharedLibrary.h>

export module core_bridge:plugin_module;

import std;
import :bridge_status;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::bridge::detail {

/** @brief Signature every plugin's C registration entrypoint must expose. */
using RegisterFn = void (*)(void*);

/** @brief Instance functor exposing the plugin registration symbol name. */
export class RegisterSymbol
{
public:
    [[nodiscard]] constexpr std::string_view operator()() const noexcept
    {
        return "congelado_bridge_register";
    }
};

/**
 * @brief One loaded plugin shared library. POCO's SharedLibrary does the
 * dlopen/dlsym work — no hand-written platform loader here.
 */
class PluginModule
{
public:
    explicit PluginModule(std::string path) :
        m_path{std::move(path)}
    {
    }

    PluginModule(const PluginModule&) = delete;
    PluginModule& operator=(const PluginModule&) = delete;
    PluginModule(PluginModule&&) noexcept = default;
    PluginModule& operator=(PluginModule&&) noexcept = default;

    [[nodiscard]] BridgeStatus load() noexcept
    {
        try {
            m_library = std::make_unique<Poco::SharedLibrary>(m_path);
            auto* symbol = m_library->getSymbol(std::string{RegisterSymbol{}()});
            if (symbol == nullptr) {
                m_library.reset();
                return BridgeStatus::SYMBOL_MISSING;
            }
            m_register = reinterpret_cast<RegisterFn>(symbol);
            return BridgeStatus::OK;
        } catch (const Poco::Exception&) {
            m_library.reset();
            m_register = nullptr;
            return BridgeStatus::LOAD_FAILED;
        }
    }

    [[nodiscard]] bool loaded() const noexcept
    {
        return m_library != nullptr && m_library->isLoaded();
    }

    /** @brief Invokes the plugin's registration entrypoint with `context`. */
    void register_plugin(void* context)
    {
        if (m_register != nullptr) {
            m_register(context);
        }
    }

    void unload() noexcept
    {
        m_register = nullptr;
        if (m_library) {
            m_library->unload();
            m_library.reset();
        }
    }

    [[nodiscard]] const std::string& path() const noexcept
    {
        return m_path;
    }

private:
    std::string m_path{};
    std::unique_ptr<Poco::SharedLibrary> m_library{};
    RegisterFn m_register{nullptr};
};

} // namespace core::bridge::detail

#ifdef CONGELADO_TEST
namespace core::bridge::detail::tests {
using namespace boost::ut;

suite<"core_bridge_plugin_module"> core_bridge_plugin_module_suite = []
{
    "missing library reports load failure"_test = []
    {
        PluginModule module{"/nonexistent/libmissing.so"};
        expect(!module.loaded());
        expect(module.load() == BridgeStatus::LOAD_FAILED);
        expect(!module.loaded());
        module.unload(); // no-op when not loaded
    };

    "registration symbol name is stable"_test = []
    {
        expect(RegisterSymbol{}() == "congelado_bridge_register");
    };
};

} // namespace core::bridge::detail::tests
#endif
