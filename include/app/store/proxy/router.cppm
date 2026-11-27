export module app_store:proxy_router;

import std;
import interfaces;
import core_router;
import core_behavior;
import app_context;

import :proxy_input;
import :proxy_runtime;
import :proxy_handlers_v1;
import :proxy_handlers_v2;
import :proxy_handlers_prefix;

export namespace app::handlers::proxy {

/**
 * Handler of handlers: owns the single shared ProxyRuntime and ProxyInputSupport,
 * then delegates registration to the version and prefix partitions. Public shape
 * is unchanged (class name, ctor, install), so all.cppm / routes.cppm need no edits.
 */
class ProxyRouteHandlers
{
public:
    explicit ProxyRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context},
        m_policy{m_input_support},
        m_request_support{m_policy},
        m_v1{context, m_runtime, m_input_support, m_request_support},
        m_v2{context, m_runtime, m_input_support, m_request_support},
        m_prefix{context, m_runtime, m_input_support, m_request_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        // Upstream v2 scope is "/{payment_methods_prefix}/proxy" (default "v2",
        // app.rs:1043-1052, configs.rs:85-89); docs add /v2/proxy and /v1/proxy
        // (openapi/routes/proxy.rs:7,61). The bare "/proxy" has no upstream counterpart.
        // Original registration order preserved: v2, v1, then config prefix alias.
        m_v2.install(router);
        m_v1.install(router);
        m_prefix.install(router);
    }

private:
    ProxyRuntime m_runtime;            // SINGLE runtime instance — shared by every family.
    ProxyInputSupport m_input_support; // SINGLE input helper — shared by every family.
    ProxyPolicy m_policy;              // SINGLE auth/scope policy.
    core::behavior::RequestSupport<ProxyPolicy> m_request_support;
    ProxyHandlersV1
        m_v1; // holds AppContext&, ProxyRuntime&, ProxyInputSupport&, RequestSupport<ProxyPolicy>&.
    ProxyHandlersV2 m_v2;
    ProxyHandlersPrefix m_prefix;
};

} // namespace app::handlers::proxy
