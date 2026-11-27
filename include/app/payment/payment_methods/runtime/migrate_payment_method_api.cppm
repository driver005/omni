export module app_payment:payment_methods_runtime_migrate_payment_method_api;

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

class MigratePaymentMethodApiRuntime
{
public:
    explicit MigratePaymentMethodApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::
        expected<::app::model::payment_methods::PaymentMethodMigrateResponse, core::error::AppError>
        migrate_payment_method_api(const MigratePaymentMethodApiInput& input) const
    {
        if (auto mca_error = m_support.check_merchant_connector(input.merchant_id)) {
            return std::unexpected{*mca_error};
        }
        const std::string payment_method_id = m_support.global_id(
            "pm_",
            input.merchant_id + input.request.get_merchant_id().get_value()
        );
        std::string last4;
        std::string network;
        if (input.request.get_card()) {
            const auto& number = input.request.get_card()->get_card_number();
            if (number.size() >= 4) {
                last4 = number.substr(number.size() - 4);
            }
            if (input.request.get_card()->get_card_network()) {
                network = input.request.get_card()->get_card_network()->get_value();
            }
        }
        ::app::store::payment_methods::PaymentMethodRecord record{};
        record.set_payment_method_id(payment_method_id);
        record.set_merchant_id(input.merchant_id);
        if (input.request.get_customer_id()) {
            record.set_customer_id(input.request.get_customer_id()->get_value());
        }
        record.set_type(
            input.request.get_payment_method() ? input.request.get_payment_method()->get_value()
                                               : std::string{"card"}
        );
        record.set_network(network);
        record.set_last4(last4);
        // Upstream migrate_payment_method encrypts card data with the key manager and writes it
        // to payment_method_data. Local substitute: key-store-backed encryption in the cache.
        if (input.request.get_card()) {
            const auto encrypted =
                m_support.encrypt_data(serde::Ser::encode_json(*input.request.get_card()));
            m_context.cache().put(
                "payment_methods.card:" + payment_method_id,
                core::cache::to_cache_value(encrypted)
            );
        }
        record.set_vault_id(m_support.local_vault_token(input.merchant_id, payment_method_id));
        record.set_status("active");
        record.set_created_at(m_support.now_value());
        auto stored = m_context.repository().create_payment_method(std::move(record));
        if (!stored) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::ALREADY_EXISTS,
                "payment method already exists"
            )};
        }
        m_context.metrics().record_flow("migrate_payment_method_api");
        m_context.metrics().counter("PAYMENT_METHOD_MIGRATE_PAYMENT_METHOD_API").increment();
        auto migrated = m_support.response_from_record(*stored);
        if (input.request.get_payment_method_type()) {
            migrated.set_payment_method_type(*input.request.get_payment_method_type());
        }
        ::app::model::payment_methods::PaymentMethodMigrateResponse response{};
        response.set_payment_method_response(std::move(migrated));
        response.set_card_migrated(input.request.get_card().has_value());
        response.set_payment_method_migrated(true);
        response.set_network_token_migrated(input.request.get_network_token().has_value());
        response.set_connector_mandate_details_migrated(
            input.request.get_connector_mandate_details().has_value()
        );
        response.set_network_transaction_id_migrated(
            input.request.get_network_transaction_id().has_value()
        );
        return ::app::model::payment_methods::PaymentMethodMigrateResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
