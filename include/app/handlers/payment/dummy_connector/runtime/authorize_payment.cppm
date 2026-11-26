export module app_payment:dummy_connector_runtime_authorize_payment;

import std;
import interfaces;
import core_error;
import core_cache;
import app_context;
import app_metrics;

import :dummy_connector_input;
import :dummy_connector_result;
import :dummy_connector_runtime_common;

export namespace app::handlers::dummy_connector {

class DummyConnectorAuthorizePaymentRuntime
{
public:
    explicit DummyConnectorAuthorizePaymentRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // routes/dummy_connector.rs:18-36 -> core.rs:59-85 (payment_authorize).
    [[nodiscard]] std::expected<DummyConnectorAuthorizePaymentResult, core::error::AppError>
    authorize_payment(const AuthorizePaymentInput& input) const
    {
        // core.rs:63 - get_payment_data_by_attempt_id. No mock sleep in this flow.
        if (input.attempt_id.empty()) {
            return DummyConnectorAuthorizePaymentResult{m_support.expired_page()};
        }
        const auto attempt = m_context.cache().get(input.attempt_id);
        const std::string payment_id =
            attempt.hit ? attempt.value_or<std::string>("") : std::string{};
        const auto payment =
            payment_id.empty() ? std::optional<PaymentData>{} : m_support.load_payment(payment_id);
        if (!payment) {
            // core.rs:78-84 - FileData(get_expired_page, TEXT_HTML)
            m_context.metrics().record_flow("DummyPaymentAuthorize");
            return DummyConnectorAuthorizePaymentResult{m_support.expired_page()};
        }

        // core.rs:67-76 - return_url uses state.base_url, page via get_authorize_page.
        const std::string return_url =
            m_support.base_url() + "/dummy-connector/complete/" + input.attempt_id;
        m_context.metrics().record_flow("DummyPaymentAuthorize");
        m_context.metrics().counter("DUMMY_CONNECTOR_AUTHORIZE").increment();
        return DummyConnectorAuthorizePaymentResult{m_support.authorize_page(*payment, return_url)};
    }

private:
    ::app::context::AppContext& m_context;
    DummyConnectorRuntimeSupport m_support;
};

} // namespace app::handlers::dummy_connector
