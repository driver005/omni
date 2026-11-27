export module app_store:proxy_runtime;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;

import :proxy_input;
import :proxy_runtime_common;
import :proxy_runtime_proxy;

export namespace app::handlers::proxy {

class ProxyRuntime
{
public:
    explicit ProxyRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_forward{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    proxy(const ProxyRequest& request, const core::behavior::CallContext& ctx) const
    {
        return m_forward.proxy(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    ProxyForwardRuntime m_forward;
};

} // namespace app::handlers::proxy
