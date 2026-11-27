export module app_webhook:webhooks_runtime_receive_incoming_webhook;

import std;
import core_behavior;
import core_error;
import app_context;

import :webhooks_input;
import :webhooks_result;
import :webhooks_runtime_common;

export namespace app::handlers::webhooks {

class ReceiveIncomingWebhookRuntime
{
public:
    explicit ReceiveIncomingWebhookRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream POST/GET/PUT /webhooks/{merchant_id}/{connector_id_or_name},
    // MerchantIdAuth(path) (routes/webhooks.rs:11-44, app.rs:2352-2357).
    [[nodiscard]] core::behavior::Outcome<WebhookAck>
    receive_incoming_webhook(
        const std::string& body,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        (void)m_context;
        auto ack = m_support.incoming_webhooks_wrapper(body, ctx, false, false);
        if (!ack) {
            return std::unexpected{ack.error()};
        }
        return std::move(*ack);
    }

private:
    ::app::context::AppContext& m_context;
    WebhooksRuntimeSupport m_support;
};

} // namespace app::handlers::webhooks
