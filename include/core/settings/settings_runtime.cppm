export module core_settings:settings_runtime;

import std;
import :settings_map;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::settings::detail {
class SettingsRuntime;
}

namespace core::settings::detail {

/** @brief Internal state owned by the facade. */
class SettingsRuntime
{
public:
    SettingsMap& map() noexcept
    {
        return m_map;
    }

    [[nodiscard]] const SettingsMap& map() const noexcept
    {
        return m_map;
    }

private:
    SettingsMap m_map{};
};

} // namespace core::settings::detail

#ifdef CONGELADO_TEST
namespace core::settings::detail::tests {
using namespace boost::ut;

suite<"core_settings_runtime"> core_settings_runtime_suite = []
{
    "owns an empty settings map"_test = []
    {
        SettingsRuntime runtime;
        expect(runtime.map().size() == 0);
    };
};

} // namespace core::settings::detail::tests
#endif
