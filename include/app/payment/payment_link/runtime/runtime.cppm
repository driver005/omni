export module app_payment:payment_link_runtime;

import std;
import core_serde;
import core_error;
import app_context;

import :payment_link_input;
import :payment_link_result;
import :payment_link_runtime_common;
import :payment_link_runtime_retrieve;
import :payment_link_runtime_initiate;
import :payment_link_runtime_initiate_secure;
import :payment_link_runtime_list;
import :payment_link_runtime_profile_list;
import :payment_link_runtime_status;

export namespace app::handlers::payment_link {

class PaymentLinkRuntime
{
public:
    explicit PaymentLinkRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_retrieve{context},
        m_initiate{context},
        m_initiate_secure{context},
        m_list{context},
        m_profile_list{context},
        m_status{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::payments::RetrievePaymentLinkResponse>
        retrieve(const core::behavior::CallContext& ctx) const
    {
        return m_retrieve.retrieve(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    initiate(const core::behavior::CallContext& ctx) const
    {
        return m_initiate.initiate(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    initiate_secure(const core::behavior::CallContext& ctx) const
    {
        return m_initiate_secure.initiate_secure(ctx);
    }

        template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentLinkListResponse>
    list(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_list.list(dto, ctx);
    }

        template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::model::payments::PaymentLinkListResponse>
    profile_list(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_profile_list.profile_list(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    status(const core::behavior::CallContext& ctx) const
    {
        return m_status.status(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    PaymentLinkRetrieveRuntime m_retrieve;
    PaymentLinkInitiateRuntime m_initiate;
    PaymentLinkInitiateSecureRuntime m_initiate_secure;
    PaymentLinkListRuntime m_list;
    PaymentLinkProfileListRuntime m_profile_list;
    PaymentLinkStatusRuntime m_status;
};

} // namespace app::handlers::payment_link
