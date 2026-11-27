export module app_payment:payment_methods_runtime_save_payment_method_api;

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

class SavePaymentMethodApiRuntime
{
public:
    explicit SavePaymentMethodApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::
        expected<::app::model::payment_methods::PaymentMethodResponse, core::error::AppError>
        save_payment_method_api(const SavePaymentMethodApiInput& input) const
    {
        auto repository = m_context.repository();
        auto record = repository.find_payment_method_by_string(input.payment_method_id);
        if (!record) {
            if (input.merchant_id.empty()) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "missing merchant id"
                )};
            }
            ::app::store::payment_methods::PaymentMethodRecord created{};
            created.set_payment_method_id(input.payment_method_id);
            created.set_merchant_id(input.merchant_id);
            if (input.request.get_customer_id()) {
                created.set_customer_id(input.request.get_customer_id()->get_value());
            }
            created.set_type(
                input.request.get_payment_method() ? input.request.get_payment_method()->get_value()
                                                   : std::string{"card"}
            );
            created.set_status("active");
            created.set_created_at(m_support.now_value());
            auto stored = repository.create_payment_method(std::move(created));
            if (!stored) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::ALREADY_EXISTS,
                    "payment method exists"
                )};
            }
            record = *stored;
        }
        if (input.request.get_card()) {
            const auto vault = m_support.call_connector(
                record->get_merchant_id(),
                "tokenize",
                input.payment_method_id,
                serde::Ser::encode_json(*input.request.get_card())
            );
            if (!vault.get_reference_id().empty()) {
                record->set_vault_id(vault.get_reference_id());
            }
            m_context.cache().put(
                "payment_methods.card:" + input.payment_method_id,
                core::cache::to_cache_value(
                    m_support.encrypt_data(serde::Ser::encode_json(*input.request.get_card()))
                )
            );
        }
        record->set_status("active");
        repository.update_payment_method(*record);
        m_support.publish_pm_event("payment_method.save", input.payment_method_id);
        m_context.metrics().record_flow("save_payment_method_api");
        m_context.metrics().counter("PAYMENT_METHOD_SAVE_PAYMENT_METHOD_API").increment();
        return ::app::model::payment_methods::PaymentMethodResponse{
            m_support.response_from_record(*record)
        };
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
