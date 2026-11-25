export module app_handlers_relay:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_unreferenced_refund;
import :runtime_relay;
import :runtime_relay_retrieve;
import :runtime_incoming_relay_webhook;

export namespace app::handlers::relay {

class RelayRuntime
{
public:
    explicit RelayRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_unreferenced_refund{context},
        m_relay{context},
        m_retrieve{context},
        m_incoming_webhook{context}
    {
    }

    [[nodiscard]] std::expected<UnreferencedRefundResult, core::error::AppError>
    unreferenced_refund(const RelayUnreferencedRefundInput& input) const
    {
        return m_unreferenced_refund.unreferenced_refund(input);
    }

    [[nodiscard]] std::expected<RelayResult, core::error::AppError>
    relay(const RelayCreateInput& input) const
    {
        return m_relay.relay(input);
    }

    [[nodiscard]] std::expected<RelayRetrieveResult, core::error::AppError>
    retrieve(const RelayRetrieveInput& input) const
    {
        return m_retrieve.retrieve(input);
    }

    [[nodiscard]] std::expected<RelayWebhookResult, core::error::AppError>
    incoming_webhook(const RelayIncomingWebhookInput& input) const
    {
        return m_incoming_webhook.incoming_webhook(input);
    }

private:
    app::context::AppContext& m_context;
    RelayUnreferencedRefundRuntime m_unreferenced_refund;
    RelayCreateRuntime m_relay;
    RelayRetrieveRuntime m_retrieve;
    RelayIncomingWebhookRuntime m_incoming_webhook;
};

} // namespace app::handlers::relay
