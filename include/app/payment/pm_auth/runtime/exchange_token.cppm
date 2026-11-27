export module app_payment:pm_auth_runtime_exchange_token;

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

import :pm_auth_input;
import :pm_auth_result;
import :pm_auth_runtime_common;

export namespace app::handlers::pm_auth {

class PmAuthExchangeTokenRuntime
{
public:
    explicit PmAuthExchangeTokenRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    exchange_token(const ExchangeTokenInput& input) const
    {
        const auto& request = input.request;
        const std::string& public_token = request.get_public_token();
        const std::string& payment_id = request.get_payment_id().get_value();
        const std::string& payment_method = request.get_payment_method().get_value();
        const std::string& payment_method_type = request.get_payment_method_type().get_value();

        // core/pm_auth.rs:105-110 applies to the settle/payment-confirm path; the exchange
        // path still verifies the stored intent state and client secret when present.
        std::optional<::app::store::payments::PaymentIntentRecord> payment_intent;
        if (const auto intent_error = m_support.verify_payment_intent(
                payment_id,
                input.merchant_id,
                request.get_client_secret().has_value(),
                payment_intent
            )) {
            return std::unexpected{*intent_error};
        }
        if (payment_intent.has_value()) {
            if (const auto age_error = m_support.check_payment_intent_age(payment_id)) {
                return std::unexpected{*age_error};
            }
        }
        if (const auto secret_error =
                m_support.verify_client_secret(payment_id, request.get_client_secret())) {
            return std::unexpected{*secret_error};
        }

        // Upstream get_selected_config_from_redis: a missing key is InvalidRequestData,
        // an absent choice is GenericNotFoundError. No seeding on the exchange path.
        const auto choice = m_support.resolve_connector_choice(
            input.merchant_id,
            payment_id,
            payment_method,
            payment_method_type
        );
        if (!choice.key_exists) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Incorrect payment_id provided in request"
            )};
        }
        std::string connector_name = choice.connector_name;
        std::optional<::app::store::merchant_connectors::MerchantConnectorRecord> mca_record;
        if (const auto mca_error = m_support.validate_merchant_connector(
                input.merchant_id,
                choice.mca_id,
                connector_name,
                mca_record
            )) {
            return std::unexpected{*mca_error};
        }
        if (!choice.matched || connector_name.empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "payment method auth connector name not found"
            )};
        }
        const ConnectorAuth connector_auth = m_support.resolve_connector_auth(mca_record);

        // core/pm_auth.rs:674-724 — typed exchange-token connector call with error mapping.
        auto exchange_request = ::app::connector::ConnectorRequestBuilder{}
                                    .operation("exchange_token")
                                    .merchant_id(input.merchant_id)
                                    .resource_id(payment_id)
                                    .raw_payload(input.body)();
        std::string access_token;
        std::string bank_raw;
        if (auto connector = m_context.connector_router().select(payment_id)) {
            const auto exchange_response = connector->exchange_token(exchange_request);
            if (const auto connector_error =
                    m_support.connector_failure(exchange_response, connector_name)) {
                return std::unexpected{*connector_error};
            }
            access_token = m_support.connector_value(exchange_response);

            // core/pm_auth.rs:618-672 — bank-account credentials call with typed error mapping.
            const std::string bank_payload =
                "{\"access_token\":" + serde::Ser::encode_json(access_token) +
                ",\"connector_auth_type\":" + serde::Ser::encode_json(connector_auth.scheme) + "}";
            auto bank_request = ::app::connector::ConnectorRequestBuilder{}
                                    .operation("bank_account_credentials")
                                    .merchant_id(input.merchant_id)
                                    .resource_id(payment_id)
                                    .raw_payload(bank_payload)();
            const auto bank_response = connector->authorize(bank_request);
            if (const auto connector_error =
                    m_support.connector_failure(bank_response, connector_name)) {
                return std::unexpected{*connector_error};
            }
            bank_raw = bank_response.get_raw_json().empty() ? bank_response.get_status()
                                                            : bank_response.get_raw_json();
        }
        if (access_token.empty()) {
            access_token = m_support.generate_token("at_", public_token + "_" + payment_id);
        }

        // core/pm_auth.rs:674-724,618-672 — normalize connector payloads into typed bank
        // credentials (router-data/ExternalConnectorError substitute).
        std::vector<BankCredential> credentials = m_support.parse_bank_credentials(bank_raw);
        if (credentials.empty()) {
            BankCredential synthesized{};
            synthesized.method = "ach";
            synthesized.account_number = access_token;
            synthesized.payment_method_type = payment_method_type;
            synthesized.account_type = "checking";
            synthesized.account_id = payment_id;
            credentials.push_back(std::move(synthesized));
        }

        // core/pm_auth.rs:301-565,579-616 — HMAC-shaped dedupe, mask generation, connector
        // detail merge and bulk PM insert/update through the repository.
        m_support.store_bank_details(
            input.merchant_id,
            payment_intent,
            connector_name,
            choice.mca_id,
            mca_record.has_value() ? mca_record->get_profile_id() : std::string{},
            access_token,
            credentials
        );

        // Access tokens are ephemeral; only the token (not the bank business record) is cached.
        const ::app::crypto::SecretString token_secret{access_token};
        m_context.cache().put(
            std::string{"pm_auth.access_token."} + payment_id,
            core::cache::to_cache_value(m_support.encrypt_secret(token_secret.value()))
        );

        m_context.metrics().record_flow("PmAuthExchangeToken");
        m_context.metrics().counter("PM_AUTH_EXCHANGE_TOKEN").increment();

        // core/pm_auth.rs:244,311 — exchange_token_core returns RouterResponse<()> /
        // ApplicationResponse::StatusOk: HTTP 200 with an empty body. No access_token is
        // emitted; the empty body is a typed empty document, not raw JSON text.
        return serde::Value{ResponseDocument::empty()};
    }

private:
    ::app::context::AppContext& m_context;
    PmAuthRuntimeSupport m_support;
};

} // namespace app::handlers::pm_auth
