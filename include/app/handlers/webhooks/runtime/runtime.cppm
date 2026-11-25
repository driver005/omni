export module app_handlers_webhooks:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_receive_incoming_webhook;
import :runtime_receive_incoming_webhook_v2;
import :runtime_receive_network_token_requestor_incoming_webhook;

export namespace app::handlers::webhooks {

class WebhooksRuntime
{
public:
    explicit WebhooksRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_receive_incoming_webhook{context},
        m_receive_incoming_webhook_v2{context},
        m_receive_network_token{context}
    {
    }

    [[nodiscard]] std::expected<ReceiveIncomingWebhookResult, core::error::AppError>
    receive_incoming_webhook(const ReceiveIncomingWebhookInput& input) const
    {
        return m_receive_incoming_webhook.receive_incoming_webhook(input);
    }

    [[nodiscard]] std::expected<ReceiveIncomingWebhookV2Result, core::error::AppError>
    receive_incoming_webhook_v2(const ReceiveIncomingWebhookV2Input& input) const
    {
        return m_receive_incoming_webhook_v2.receive_incoming_webhook_v2(input);
    }

    [[nodiscard]] std::expected<ReceiveNetworkTokenResult, core::error::AppError>
    receive_network_token(const ReceiveNetworkTokenInput& input) const
    {
        return m_receive_network_token.receive_network_token(input);
    }

private:
    app::context::AppContext& m_context;
    ReceiveIncomingWebhookRuntime m_receive_incoming_webhook;
    ReceiveIncomingWebhookV2Runtime m_receive_incoming_webhook_v2;
    ReceiveNetworkTokenRuntime m_receive_network_token;
};

} // namespace app::handlers::webhooks
