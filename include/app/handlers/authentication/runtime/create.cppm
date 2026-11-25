export module app_handlers_authentication:runtime_create;

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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::authentication {

class AuthenticationCreateRuntime
{
public:
    explicit AuthenticationCreateRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: create ─────────────────────────
    // unified_authentication_service.rs:712-875 authentication_create_core.
    [[nodiscard]] std::expected<AuthenticationCreateResult, core::error::AppError>
    create(const AuthenticationCreateInput& input) const
    {
        // :721-732 get_profile_from_business_details + validate_profile_id_from_auth_layer.
        std::string profile_id{};
        if (input.request.get_profile_id().has_value()) {
            profile_id = input.request.get_profile_id()->get_value();
            if (!m_context.repository().find_profile_by_string(profile_id).has_value()) {
                m_context.metrics().counter("authentication_profile_missing").increment();
            }
        } else if (
            const auto claim = input.auth.context.claim("x-profile-id");
            claim.has_value() && !claim->empty()
        ) {
            profile_id = std::string{*claim};
        }
        if (profile_id.empty()) {
            profile_id = std::string{"pro_"} + std::string{input.merchant_id};
        }
        m_context.metrics().counter("authentication_profile_resolved").increment();

        // :734-736 AuthenticationId::generate_authentication_id(AUTHENTICATION_ID_PREFIX).
        const auto authentication_id = m_context.ids().payment_id("authn_");

        // :738-741 force_3ds_challenge = req OR profile default.
        const std::string force_3ds =
            input.request.get_force_3ds_challenge().value_or(false) ? "true" : "false";

        // :743-757 acquirer_details priority (profile_acquirer_id deferred to eligibility_core,
        // where the card network becomes available).
        std::string acquirer_bin{};
        std::string acquirer_merchant_id{};
        std::string acquirer_country_code{};
        if (input.request.get_acquirer_details().has_value()) {
            if (input.request.get_acquirer_details()->get_acquirer_bin()) {
                acquirer_bin = *input.request.get_acquirer_details()->get_acquirer_bin();
            }
            if (input.request.get_acquirer_details()->get_acquirer_merchant_id()) {
                acquirer_merchant_id =
                    *input.request.get_acquirer_details()->get_acquirer_merchant_id();
            }
            if (input.request.get_acquirer_details()->get_merchant_country_code()) {
                acquirer_country_code =
                    *input.request.get_acquirer_details()->get_merchant_country_code();
            }
        }

        // :759-788 crypto_operation EncryptOptional(customer_details) with the merchant key.
        // The local keystore wrap is the documented in-process substitute for the key manager.
        const auto encrypted_customer =
            m_support.encrypted_customer_details(input.request.get_customer_details());

        AuthenticationRecordData record{};
        record.set_authentication_id(authentication_id);
        record.set_merchant_id(std::string{input.merchant_id});
        record.set_profile_id(profile_id);
        if (input.request.get_authentication_connector().has_value()) {
            record.set_authentication_connector(
                input.request.get_authentication_connector()->get_value()
            );
        }
        record.set_amount(input.request.get_amount().get_value());
        record.set_currency(input.request.get_currency().get_value());
        record.set_force_3ds(force_3ds);
        if (input.request.get_psd2_sca_exemption_type().has_value()) {
            record.set_psd2_sca_exemption_type(
                input.request.get_psd2_sca_exemption_type()->get_value()
            );
        }
        record.set_acquirer_bin(acquirer_bin);
        record.set_acquirer_merchant_id(acquirer_merchant_id);
        record.set_acquirer_country_code(acquirer_country_code);
        if (input.request.get_return_url().has_value()) {
            record.set_return_url(*input.request.get_return_url());
        }
        if (input.request.get_profile_acquirer_id().has_value()) {
            record.set_profile_acquirer_id(input.request.get_profile_acquirer_id()->get_value());
        }
        if (encrypted_customer.has_value()) {
            record.set_customer_details(encrypted_customer->get_value());
        }
        // :800 common_enums::AuthenticationStatus::Started (snake_case wire value).
        record.set_status("started");
        record.set_created_at(m_support.created_timestamp(authentication_id));
        // :1265-1276 merchant_details.three_ds_requestor_url from
        // business_profile.authentication_connector_details — local profile-config substitute.
        record.set_three_ds_requestor_url(m_support.three_ds_requestor_url_for(profile_id));

        // :790-818 create_new_authentication -> DB insert (DuplicateAuthentication mapping).
        if (m_support.find_authentication(authentication_id).has_value()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::DUPLICATE_REQUEST,
                "Authentication with authentication_id " + authentication_id + " already exists"
            )};
        }
        const auto client_secret_plain = app::crypto::Base64Url{}.encode(
            app::crypto::KeyedHasher{"authentication_client_secret"}.hash(
                authentication_id + "_secret"
            )
        );
        record.set_client_secret(client_secret_plain);
        m_support.save_authentication(record);
        m_support.remember_client_secret(authentication_id, client_secret_plain);
        m_support.persist_authentication(
            authentication_id,
            input.merchant_id,
            "started",
            app::crypto::KeyedHasher{"authentication_client_secret"}.hash(client_secret_plain)
        );

        // routes/authentication.rs:27-45 wraps the core in Flow::AuthenticationCreate.
        m_context.metrics().record_flow("AuthenticationCreate");
        m_context.metrics().counter("authentication_create").increment();

        // :861-870 AuthenticationResponse::foreign_try_from (round-trip decrypt of
        // customer_details via crypto_operation DecryptOptional, :835-859).
        return AuthenticationCreateResult{m_support.build_create_response(record, input.request)};
    }

private:
    app::context::AppContext& m_context;
    AuthenticationRuntimeSupport m_support;
};

} // namespace app::handlers::authentication
