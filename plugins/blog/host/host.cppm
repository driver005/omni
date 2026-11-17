export module blog_host:host;

import std;
import core_bridge;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::host {

/** @brief Ghost host service: owns the bridge that wires core facades to plugins. */
class HostService
{
public:
    HostService() :
        m_bridge{core::bridge::BridgeBuilder{}.build()}
    {
    }

    [[nodiscard]] core::bridge::Bridge& bridge() noexcept
    {
        return m_bridge;
    }

    [[nodiscard]] bool load(const std::string& path)
    {
        return m_bridge.load_plugin(path) == core::bridge::BridgeStatus::OK;
    }

private:
    core::bridge::Bridge m_bridge{};
};

} // namespace blog::host
#ifdef CONGELADO_TEST
namespace blog::host::tests {
using namespace boost::ut;
suite<"blog_host_host"> blog_host_host_suite = []
{
    "owns a bridge"_test = []
    {
        HostService host;
        expect(!host.load("/nonexistent/p.so"));
    };
};
} // namespace blog::host::tests
#endif
