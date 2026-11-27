export module app_webhook:webhooks_runtime_receive_incoming_webhook_v2;

import std;
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
    [[nodiscard]] std::expected<WebhookAck, core::error::AppError>
    receive_incoming_webhook_v2(const ReceiveIncomingWebhookV2Input& input) const
    {
        (void)m_context;
        auto ack = m_support.incoming_webhooks_wrapper(
            input.ids,
            input.body,
            input.header,
            input.operation,
            input.flow,
            input.counter,
            false,
            true
        );
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
