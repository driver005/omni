export module app_webhook:webhooks_runtime_receive_incoming_webhook;

import std;
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
    [[nodiscard]] std::expected<ReceiveIncomingWebhookResult, core::error::AppError>
    receive_incoming_webhook(const ReceiveIncomingWebhookInput& input) const
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
            false
        );
        if (!ack) {
            return std::unexpected{ack.error()};
        }
        return ReceiveIncomingWebhookResult{std::move(*ack)};
    }

private:
    ::app::context::AppContext& m_context;
    WebhooksRuntimeSupport m_support;
};

} // namespace app::handlers::webhooks
