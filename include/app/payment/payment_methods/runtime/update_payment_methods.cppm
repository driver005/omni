export module app_payment:payment_methods_runtime_update_payment_methods;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :payment_methods_input;
import :payment_methods_result;
import :payment_methods_runtime_common;

export namespace app::handlers::payment_methods {

class UpdatePaymentMethodsRuntime
{
public:
    explicit UpdatePaymentMethodsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<
        ::app::model::payment_methods::PaymentMethodRecordUpdateResponse,
        core::error::AppError>
    update_payment_methods(const UpdatePaymentMethodsInput& input) const
    {
        m_context.metrics().record_flow("update_payment_methods");
        m_context.metrics().counter("PAYMENT_METHOD_UPDATE_PAYMENT_METHODS").increment();
        auto repository = m_context.repository();
        for (const auto& update: input.rows) {
            auto record = repository.find_payment_method_by_string(update.get_payment_method_id());
            if (!record) {
                continue;
            }
            if (update.get_status()) {
                record->set_status(update.get_status()->get_value());
            }
            repository.update_payment_method(*record);
        }
        const auto& first = input.rows.front();
        ::app::model::payment_methods::PaymentMethodRecordUpdateResponse response{};
        response.set_payment_method_id(first.get_payment_method_id());
        ::app::model::payment_methods::PaymentMethodStatusValue status{};
        status.set_value(first.get_status() ? first.get_status()->get_value() : "active");
        response.set_status(std::move(status));
        if (first.get_network_transaction_id()) {
            response.set_network_transaction_id(*first.get_network_transaction_id());
        }
        response.set_updated_payment_method_data(true);
        return ::app::model::payment_methods::PaymentMethodRecordUpdateResponse{
            std::move(response)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
