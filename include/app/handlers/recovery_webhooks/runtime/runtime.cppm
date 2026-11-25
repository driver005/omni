export module app_handlers_recovery_webhooks:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_recovery_receive_incoming_webhook;

export namespace app::handlers::recovery_webhooks {

class RecoveryWebhooksRuntime
{
public:
    explicit RecoveryWebhooksRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_recovery_receive_incoming_webhook{context}
    {
    }

    [[nodiscard]] std::expected<RecoveryReceiveIncomingWebhookResult, core::error::AppError>
    recovery_receive_incoming_webhook(const RecoveryReceiveIncomingWebhookInput& input) const
    {
        return m_recovery_receive_incoming_webhook.receive_incoming_webhook(input);
    }

private:
    app::context::AppContext& m_context;
    RecoveryReceiveIncomingWebhookRuntime m_recovery_receive_incoming_webhook;
};

} // namespace app::handlers::recovery_webhooks
