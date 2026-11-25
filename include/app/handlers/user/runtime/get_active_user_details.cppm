export module app_handlers_user:runtime_get_active_user_details;

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
import app_locking;
import app_services;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::user {

class GetActiveUserDetailsRuntime
{
public:
    explicit GetActiveUserDetailsRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<GetActiveUserDetailsResult, core::error::AppError>
    get_active_user_details(const GetActiveUserDetailsInput& input) const
    {
        std::string set_cookie{};

        const auto merchant_id = input.request_context.merchant_id;
        const auto org_id = input.request_context.org_id;
        const auto auth_ctx = input.request_context.auth;

        // The bearer token is a signed JWT, not a user id. Decode its `sub` claim
        // and use the persisted user/merchant so a freshly signed-up account has a
        // selected merchant/org even on its very first request (no headers yet).
        const auto owner = m_support.token_subject(auth_ctx.token().value());
        std::optional<app::store::users::UserRecord> user{};
        if (!owner.empty()) {
            user = m_context.repository().find_user_by_string(owner);
        }

        std::string effective_merchant_id =
            merchant_id.empty() ? auth_ctx.merchant_id() : merchant_id;
        if (effective_merchant_id.empty() && user) {
            effective_merchant_id = user->get_merchant_id();
        }
        std::string effective_org_id = org_id.empty() ? auth_ctx.org_id() : org_id;


        m_context.metrics().record_flow("get_active_user_details");
        m_context.metrics().counter("user.get_active_user_details").increment();

        std::string merchant_name{};
        if (!effective_merchant_id.empty()) {
            const auto account =
                m_context.repository().find_merchant_account_by_string(effective_merchant_id);
            if (!account) {
                return std::unexpected{m_support.wire_error(
                    "UR_18",
                    "Invalid Merchant ID",
                    core::error::AppErrorKind::INVALID_REQUEST
                )};
            }
            merchant_name = account->get_merchant_name();
            if (effective_org_id.empty()) {
                effective_org_id = account->get_organization_id();
            }
        }

        // Derive verification/theme data from persisted config and identity records
        // instead of fixed placeholders (parity: core/user.rs:139-226).
        bool two_factor_setup{false};
        for (const auto& record: m_context.repository().list_authentication_records()) {
            if (record.get_merchant_id() == effective_merchant_id &&
                record.get_method() == "TOTP" && record.get_status() == "active") {
                two_factor_setup = true;
                break;
            }
        }
        const auto verification_days =
            m_context.repository().get_config("user.verification_days." + effective_merchant_id);
        const auto theme_id =
            m_context.repository().get_config("user.theme." + effective_merchant_id);
        const auto credential_source =
            m_context.repository().get_config("user.offer_engine_credential_source");

        using Response = app::model::user::GetUserDetailsResponse;
        Response response{};
        app::model::user::MerchantIdValue mid{};
        mid.set_value(effective_merchant_id);
        response.set_merchant_id(mid);
        app::model::user::OrganizationIdValue oid{};
        oid.set_value(effective_org_id);
        response.set_name(merchant_name);
        response.set_org_id(oid);
        response.set_is_two_factor_auth_setup(two_factor_setup);

        // Port of core/user.rs:211-227: identity fields come from the active
        // user row; role/profile/entity come from the verified token context.
        if (user) {
            response.set_user_id(user->get_user_id());
            app::model::user::EmailValue email{};
            email.set_value(user->get_email());
            response.set_email(email);
            if (!user->get_name().empty()) {
                response.set_name(user->get_name());
            }
            response.set_role_id(user->get_role());
        } else {
            response.set_user_id(owner);
            const auto email_entry = m_context.cache().get("user.token_email:" + owner);
            app::model::user::EmailValue email{};
            email.set_value(
                email_entry.hit ? core::cache::value_as<std::string>(email_entry.value, owner)
                                : owner
            );
            response.set_email(email);
            response.set_role_id(m_context.repository()
                                     .get_config("user.role." + owner)
                                     .value_or(std::string{"role_org_admin"}));
        }
        app::model::user::ProfileIdValue pid{};
        pid.set_value(auth_ctx.profile_id());
        response.set_profile_id(pid);
        app::model::user::EntityTypeValue entity{};
        entity.set_value(m_context.repository()
                             .get_config("user.entity_type." + effective_merchant_id)
                             .value_or(std::string{"merchant"}));
        response.set_entity_type(entity);

        const auto recovery = m_context.repository().get_config("user.recovery_codes." + owner);
        if (recovery) {
            std::size_t count{0};
            std::from_chars(recovery->data(), recovery->data() + recovery->size(), count);
            response.set_recovery_codes_left(count);
        }

        if (verification_days) {
            std::int64_t days{0};
            std::from_chars(
                verification_days->data(),
                verification_days->data() + verification_days->size(),
                days
            );
            response.set_verification_days_left(days);
        }
        if (theme_id) {
            response.set_theme_id(*theme_id);
        }
        app::model::user::ApiVersionValue version{};
        version.set_value(m_context.key_store().get("jwt", "user-jwt-v2") ? "V2" : "V1");
        response.set_version(version);
        app::model::user::OfferEngineCredentialSourceValue source{};
        source.set_value(credential_source.value_or(std::string{"config"}));
        response.set_offer_engine_credential_source(source);
        auto document = ResponseDocument::from(response);
        return GetActiveUserDetailsResult{document, set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
