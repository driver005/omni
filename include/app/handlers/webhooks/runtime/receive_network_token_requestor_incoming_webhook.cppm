export module app_handlers_webhooks:runtime_receive_network_token_requestor_incoming_webhook;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::webhooks {

class ReceiveNetworkTokenRuntime
{
public:
    explicit ReceiveNetworkTokenRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream POST /webhooks/network_token_requestor/ref, NoAuth
    // (routes/webhooks.rs:172-194, app.rs:2346-2351).
    [[nodiscard]] std::expected<ReceiveNetworkTokenResult, core::error::AppError>
    receive_network_token(const ReceiveNetworkTokenInput& input) const
    {
        (void)m_context;
        auto ack = m_support.network_token_incoming_webhooks_wrapper(
            input.ids,
            input.body,
            input.header,
            input.operation,
            input.flow,
            input.counter
        );
        if (!ack) {
            return std::unexpected{ack.error()};
        }
        return ReceiveNetworkTokenResult{std::move(*ack)};
    }

private:
    app::context::AppContext& m_context;
    WebhooksRuntimeSupport m_support;
};

} // namespace app::handlers::webhooks
