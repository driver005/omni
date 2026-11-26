export module app_payment:payment_methods_runtime_create_payment_method_api;

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

class CreatePaymentMethodApiRuntime
{
public:
    explicit CreatePaymentMethodApiRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<CreatePaymentMethodApiResult, core::error::AppError>
    create_payment_method_api(const CreatePaymentMethodApiInput& input) const
    {
        if (input.request.get_customer_id() &&
            !m_support.customer_exists(input.request.get_customer_id()->get_value())) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::NOT_FOUND,
                "Customer does not exist in our records"
            )};
        }
        std::string last4;
        std::string network;
        std::string card_json = "{}";
        if (input.request.get_card()) {
            const auto& card = *input.request.get_card();
            const auto& number = card.get_card_number().get_value();
            if (number.size() >= 4) {
                last4 = number.substr(number.size() - 4);
            }
            if (card.get_card_network()) {
                network = card.get_card_network()->get_value();
            }
            card_json = "{\"number\":" + serde::Ser::encode_json(number) +
                        ",\"exp_month\":" + serde::Ser::encode_json(card.get_card_exp_month()) +
                        ",\"exp_year\":" + serde::Ser::encode_json(card.get_card_exp_year());
            if (card.get_card_cvc()) {
                card_json += ",\"cvc\":" + serde::Ser::encode_json(*card.get_card_cvc());
            }
            if (card.get_card_holder_name()) {
                card_json += ",\"holder\":" + serde::Ser::encode_json(*card.get_card_holder_name());
            }
            card_json += "}";
        }
        if (input.request.get_billing()) {
            m_context.cache().put(
                "payment_methods.billing:" + m_context.ids().uuid_v4(),
                core::cache::to_cache_value(
                    m_support.encrypt_data(serde::Ser::encode_json(*input.request.get_billing()))
                )
            );
        }
        const auto fingerprint = m_support.merchant_fingerprint(input.merchant_id, last4, network);
        auto repository = m_context.repository();
        const auto duplicate = m_context.cache().get("payment_methods.fingerprint:" + fingerprint);
        if (duplicate.hit) {
            const auto existing = repository.find_payment_method_by_string(
                core::cache::value_as<std::string>(duplicate.value)
            );
            if (existing) {
                auto response = m_support.response_from_record(*existing);
                response.set_client_secret(
                    m_support.issued_token(existing->get_payment_method_id())
                );
                return CreatePaymentMethodApiResult{std::move(response)};
            }
        }
        const std::string payment_method_id = m_context.ids().payment_id("pm_");
        const auto vault_response =
            m_support.call_connector(input.merchant_id, "tokenize", payment_method_id, card_json);
        const std::string vault_id = vault_response.get_reference_id().empty()
                                         ? fingerprint.substr(0, 16)
                                         : vault_response.get_reference_id();
        if (card_json != "{}") {
            m_context.cache().put(
                "payment_methods.card:" + payment_method_id,
                core::cache::to_cache_value(m_support.encrypt_data(card_json))
            );
        }
        ::app::store::payment_methods::PaymentMethodRecord record{};
        record.set_payment_method_id(payment_method_id);
        record.set_merchant_id(input.merchant_id);
        if (input.request.get_customer_id()) {
            record.set_customer_id(input.request.get_customer_id()->get_value());
        }
        record.set_profile_id(input.profile_id);
        record.set_type(
            input.request.get_payment_method() ? input.request.get_payment_method()->get_value()
                                               : std::string{"card"}
        );
        record.set_network(network);
        record.set_last4(last4);
        record.set_vault_id(vault_id);
        record.set_status("active");
        record.set_created_at(m_support.now_value());
        auto stored = repository.create_payment_method(std::move(record));
        if (!stored) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::ALREADY_EXISTS,
                "payment method already exists"
            )};
        }
        m_context.cache().put(
            "payment_methods.fingerprint:" + fingerprint,
            core::cache::to_cache_value(payment_method_id)
        );
        const std::string client_secret = m_support.issued_token(payment_method_id);
        m_support.remember_client_secret(payment_method_id, client_secret);
        m_support.schedule_status_task(payment_method_id, input.merchant_id);
        m_support.publish_pm_event("payment_method.create", payment_method_id);
        m_context.metrics().record_flow("create_payment_method_api");
        m_context.metrics().counter("PAYMENT_METHOD_CREATE_PAYMENT_METHOD_API").increment();
        auto response = m_support.response_from_record(*stored);
        if (input.request.get_payment_method_type()) {
            response.set_payment_method_type(*input.request.get_payment_method_type());
        }
        if (input.request.get_metadata()) {
            response.set_metadata(*input.request.get_metadata());
        }
        response.set_client_secret(client_secret);
        return CreatePaymentMethodApiResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentMethodsRuntimeSupport m_support;
};

} // namespace app::handlers::payment_methods
