export module app_payment:relay_runtime;

import std;
import core_serde;
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

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::dto::unreferenced_refund::UnreferencedRefundResponse>
        unreferenced_refund(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_unreferenced_refund.unreferenced_refund(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::relay::RelayResponse>
    relay(const core::behavior::CallContext& ctx) const
    {
        return m_relay.relay(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::relay::RelayResponse>
    retrieve(const core::behavior::CallContext& ctx) const
    {
        return m_retrieve.retrieve(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    incoming_webhook(const core::behavior::CallContext& ctx) const
    {
        return m_incoming_webhook.incoming_webhook(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    RelayUnreferencedRefundRuntime m_unreferenced_refund;
    RelayCreateRuntime m_relay;
    RelayRetrieveRuntime m_retrieve;
    RelayIncomingWebhookRuntime m_incoming_webhook;
};

} // namespace app::handlers::relay
