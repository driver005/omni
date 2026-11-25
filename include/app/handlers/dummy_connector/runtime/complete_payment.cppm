export module app_handlers_dummy_connector:runtime_complete_payment;

import std;
import interfaces;
import core_error;
import core_cache;
import app_context;
import app_metrics;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::dummy_connector {

class DummyConnectorCompletePaymentRuntime
{
public:
    explicit DummyConnectorCompletePaymentRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // routes/dummy_connector.rs:40-62 -> core.rs:88-153 (payment_complete).
    [[nodiscard]] std::expected<DummyConnectorCompletePaymentResult, core::error::AppError>
    complete_payment(const CompletePaymentInput& input) const
    {
        // core.rs:98 - get_payment_data_by_attempt_id (result kept for the branch).
        const auto attempt = m_context.cache().get(input.attempt_id);
        const std::string payment_id =
            attempt.hit ? attempt.value_or<std::string>("") : std::string{};
        auto payment =
            payment_id.empty() ? std::optional<PaymentData>{} : m_support.load_payment(payment_id);

        // core.rs:112 - delete the attempt key unconditionally.
        m_context.cache().erase(input.attempt_id);

        const std::string default_return_url =
            m_context.config_store()
                .get_value("dummy_connector", "default_return_url")
                .value_or(std::string{"https://app.hyperswitch.io/"});
        std::string return_url = default_return_url;
        if (payment) {
            // core.rs:114-126 - status + next_action=None, store with payment_ttl.
            payment->status = input.confirm ? Status::Succeeded : Status::Failed;
            payment->next_action.reset();
            m_support.save_payment(*payment);
            return_url = payment->return_url.value_or(default_return_url);
        }

        // core.rs:127-152 - JsonForRedirection(RedirectionResponse): 302 + Location.
        m_context.metrics().record_flow("DummyPaymentComplete");
        m_context.metrics().counter("DUMMY_CONNECTOR_COMPLETE").increment();
        return DummyConnectorCompletePaymentResult{
            m_support.redirect_response(return_url),
            return_url
        };
    }

private:
    app::context::AppContext& m_context;
    DummyConnectorRuntimeSupport m_support;
};

} // namespace app::handlers::dummy_connector
