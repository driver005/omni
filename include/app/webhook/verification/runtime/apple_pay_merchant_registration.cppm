export module app_webhook:verification_runtime_apple_pay_merchant_registration;

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

import :verification_input;
import :verification_result;
import :verification_runtime_common;

export namespace app::handlers::verification {

class ApplePayMerchantRegistrationRuntime
{
public:
    explicit ApplePayMerchantRegistrationRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief Port of `verify_merchant_creds_for_applepay`
     *  (core/verification.rs:11-93). */
    [[nodiscard]] std::expected<ApplePayMerchantRegistrationResult, core::error::AppError>
    verify_merchant_creds_for_applepay(const ApplePayMerchantRegistrationInput& input) const
    {
        const auto& domain_names = input.request.get_domain_names();
        const auto& merchant_connector_id = input.request.get_merchant_connector_account_id();

        // core/verification.rs:18-24 — resolve key store + MCA first.
        auto account = m_support.get_apple_pay_merchant_connector_account(
            input.merchant_id,
            merchant_connector_id,
            input.profile_id
        );
        if (!account.has_value()) {
            return std::unexpected(account.error());
        }

        // core/verification.rs:26-41. APPLEPAY_INTERNAL_MERCHANT_NAME is
        // "Applepay_merchant" (:9); both identifier fields share the configured
        // common_merchant_identifier (:28-31).
        const auto common_identifier =
            m_context.config_store()
                .get_value("applepay_merchant_configs", "common_merchant_identifier")
                .value_or(std::string{"Applepay_merchant"});
        ::app::dto::verifications::ApplepayMerchantVerificationConfigs configs{};
        configs.set_domain_names(domain_names);
        configs.set_encrypt_to(common_identifier);
        configs.set_partner_internal_merchant_identifier(common_identifier);
        configs.set_partner_merchant_name("Applepay_merchant");

        // core/verification.rs:43-54 — POST, JSON content type, cert + cert key.
        // The in-process transport is a local substitute for the Apple HTTPS
        // gateway; the wire body and certificate references are upstream-exact.
        const auto request = m_support.request_builder(configs);

        // core/verification.rs:56-63.
        const auto response = m_support.call_connector_api(input.merchant_id, request);
        m_support.log_applepay_verification_response_if_error(response);

        // core/verification.rs:65-66 — outer client error maps to
        // InternalServerError (the original error is already logged above).
        if (response.outcome() == AppleCallOutcome::TRANSPORT_ERROR) {
            return std::unexpected(m_support.make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "InternalServerError"
            ));
        }

        // core/verification.rs:69-92 — Ok persists the domains; Err returns
        // InvalidRequestData{"Applepay verification Failed"}.
        if (response.outcome() == AppleCallOutcome::CONNECTOR_ERROR) {
            return std::unexpected(m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Applepay verification Failed"
            ));
        }

        auto added =
            m_support.add_domain_to_apple_pay_merchant_connector_account(*account, domain_names);
        if (!added.has_value()) {
            return std::unexpected(added.error());
        }

        ::app::dto::verifications::ApplepayMerchantResponse response_dto{};
        response_dto.set_status_message("Applepay verification Completed");

        // #[instrument(fields(flow = ?Flow::Verification))] on the route.
        m_context.metrics().record_flow("Verification");
        return ApplePayMerchantRegistrationResult{response_dto};
    }

private:
    ::app::context::AppContext& m_context;
    VerificationRuntimeSupport m_support;
};

} // namespace app::handlers::verification
