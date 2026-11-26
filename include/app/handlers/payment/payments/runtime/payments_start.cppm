export module app_payment:payments_runtime_payments_start;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_model;
import app_dto;
import app_store;
import app_connector;
import app_crypto;
import core_cache;
import app_http;

import :payments_input;
import :payments_result;
import :payments_runtime_common;

export namespace app::handlers::payments {

class PaymentsStartRuntime
{
public:
    explicit PaymentsStartRuntime(
        ::app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] std::expected<PaymentsStartResult, core::error::AppError>
    payments_start(const PaymentsStartInput& input) const
    {
        std::string service_status{"pending"};
        auto connector_request = ::app::connector::ConnectorRequestBuilder{}
                                     .operation("authorize")
                                     .merchant_id(input.data.merchant_id)
                                     .resource_id(input.data.payment_id)
                                     .raw_payload(input.data.body)();
        if (auto connector = m_context.connector_router().select(input.data.merchant_id)) {
            const auto connector_response = connector->authorize(connector_request);
            if (!connector_response.get_status().empty()) {
                service_status = connector_response.get_reference_id();
            }
        }
        if (const auto err = m_support.ensure_payment_exists(input.data)) {
            return std::unexpected{*err};
        }
        ::app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(m_support.local_redirect_url(input.data, service_status));
        return PaymentsStartResult{response};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
