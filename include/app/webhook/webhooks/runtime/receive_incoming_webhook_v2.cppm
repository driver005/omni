export module app_webhook:webhooks_runtime_receive_incoming_webhook_v2;

import std;
import core_behavior;
import core_error;
import app_context;

import :webhooks_input;
import :webhooks_result;
import :webhooks_runtime_common;

export namespace app::handlers::webhooks {

class ReceiveIncomingWebhookV2Runtime
{
public:
    explicit ReceiveIncomingWebhookV2Runtime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream POST/GET/PUT /v2/webhooks/{merchant_id}/{profile_id}/{connector_id},
    // MerchantIdAndProfileIdAuth(path) (routes/webhooks.rs:129-170, app.rs:2391-2395).
    [[nodiscard]] core::behavior::Outcome<WebhookAck>
    receive_incoming_webhook_v2(
        const std::string& body,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        (void)m_context;
        auto ack = m_support.incoming_webhooks_wrapper(body, ctx, false, true);
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
