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

    [[nodiscard]] std::
        expected<::app::model::payments::RetrievePaymentLinkResponse, core::error::AppError>
        retrieve(const PaymentLinkRetrieveInput& input) const
    {
        return m_retrieve.retrieve(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    initiate(const PaymentLinkInitiateInput& input) const
    {
        return m_initiate.initiate(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    initiate_secure(const PaymentLinkInitiateSecureInput& input) const
    {
        return m_initiate_secure.initiate_secure(input);
    }

    [[nodiscard]] std::
        expected<::app::model::payments::PaymentLinkListResponse, core::error::AppError>
        list(const PaymentLinkListInput& input) const
    {
        return m_list.list(input);
    }

    [[nodiscard]] std::
        expected<::app::model::payments::PaymentLinkListResponse, core::error::AppError>
        profile_list(const PaymentLinkListInput& input) const
    {
        return m_profile_list.profile_list(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    status(const PaymentLinkStatusInput& input) const
    {
        return m_status.status(input);
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
