export module app_payment:dummy_connector_runtime_payment;
import core_behavior;

import std;
import core_serde;
import interfaces;
import core_error;
import core_cache;
import app_context;
import app_metrics;

import :dummy_connector_input;
import :dummy_connector_result;
import :dummy_connector_runtime_common;

export namespace app::handlers::dummy_connector {

class DummyConnectorPaymentRuntime
{
public:
    explicit DummyConnectorPaymentRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // routes/dummy_connector.rs:66-83 -> core.rs:13-42 (payment).
    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    payment(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = PaymentInputData{dto, ctx};
        const PaymentRequest& request = input.request;

        // types.rs:77-88 - PaymentId::default() (pay_*) and
        // generate_id_with_default_len("dummy_attempt").
        const std::string payment_id = m_context.ids().payment_id("pay_");
        const std::string attempt_id =
            m_context.ids().payment_id(std::string{ATTEMPT_ID_PREFIX} + "_");

        // types.rs:405-419 - process_payment_attempt: redirect URL + PM flow.
        const std::string redirect_url =
            m_support.base_url() + "/dummy-connector/authorize/" + attempt_id;
        auto flow = m_support.process_payment_attempt(request, redirect_url);
        if (!flow) {
            return std::unexpected{flow.error()};
        }

        PaymentData data{};
        data.attempt_id = attempt_id;
        data.payment_id = payment_id;
        data.status = flow->status;
        data.amount = request.amount;
        data.eligible_amount = request.amount;
        data.currency = request.currency;
        data.created = m_support.now_iso8601();
        data.pm_type = m_support.to_payment_method_type(request.pm);
        data.connector = request.connector;
        data.next_action = std::move(flow->next_action);
        data.return_url = std::move(flow->return_url);

        // core.rs:27-40 - two Redis writes with authorize_ttl / payment_ttl.
        m_context.cache()
            .put(attempt_id, core::cache::to_cache_value(payment_id), m_support.ttl("authorize"));
        m_support.save_payment(data);

        m_context.logger().record_flow("DummyPaymentCreate");
        m_context.logger().counter("DUMMY_CONNECTOR_PAYMENT").increment();
        return serde::Value{m_support.payment_response(data)};
    }

private:
    ::app::context::AppContext& m_context;
    DummyConnectorRuntimeSupport m_support;
};

} // namespace app::handlers::dummy_connector
