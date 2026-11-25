export module app_handlers_proxy:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_proxy;

export namespace app::handlers::proxy {

class ProxyRuntime
{
public:
    explicit ProxyRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_forward{context}
    {
    }

    [[nodiscard]] std::expected<ProxyResult, core::error::AppError>
    proxy(const ProxyInput& input) const
    {
        return m_forward.proxy(input);
    }

private:
    app::context::AppContext& m_context;
    ProxyForwardRuntime m_forward;
};

} // namespace app::handlers::proxy
