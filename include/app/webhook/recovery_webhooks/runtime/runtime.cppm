export module app_webhook:recovery_webhooks_runtime;

import std;
import core_behavior;
import core_error;
import app_context;

import :recovery_webhooks_input;
import :recovery_webhooks_result;
import :recovery_webhooks_runtime_recovery_receive_incoming_webhook;

export namespace app::handlers::recovery_webhooks {

class RecoveryWebhooksRuntime
{
public:
    explicit RecoveryWebhooksRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_recovery_receive_incoming_webhook{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<WebhookResponse> recovery_receive_incoming_webhook(
        const IncomingWebhookRequestDetails& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_recovery_receive_incoming_webhook.receive_incoming_webhook(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    RecoveryReceiveIncomingWebhookRuntime m_recovery_receive_incoming_webhook;
};

} // namespace app::handlers::recovery_webhooks
