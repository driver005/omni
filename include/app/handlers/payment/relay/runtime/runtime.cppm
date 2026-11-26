export module app_payment:relay_runtime;

import std;
import core_error;
import app_context;

import :relay_input;
import :relay_result;
import :relay_runtime_common;
import :relay_runtime_unreferenced_refund;
import :relay_runtime_relay;
import :relay_runtime_relay_retrieve;
import :relay_runtime_incoming_relay_webhook;

export namespace app::handlers::relay {

class RelayRuntime
{
public:
    explicit RelayRuntime(::app::context::AppContext& context) noexcept :
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
    ::app::context::AppContext& m_context;
    RelayUnreferencedRefundRuntime m_unreferenced_refund;
    RelayCreateRuntime m_relay;
    RelayRetrieveRuntime m_retrieve;
    RelayIncomingWebhookRuntime m_incoming_webhook;
};

} // namespace app::handlers::relay
