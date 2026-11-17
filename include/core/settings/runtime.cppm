export module core_settings:runtime;

import std;
import :map;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::settings {
class SettingsRuntime;
}

namespace core::settings {

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

} // namespace core::settings

#ifdef CONGELADO_TEST
namespace core::settings::tests {
using namespace boost::ut;

suite<"core_settings_runtime"> core_settings_runtime_suite = []
{
    "owns an empty settings map"_test = []
    {
        SettingsRuntime runtime;
        expect(runtime.map().size() == 0);
    };
};

} // namespace core::settings::tests
#endif
