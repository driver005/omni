export module app_user:user_runtime_user_signup_with_merchant_id;

import std;
import core_behavior;
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

import :user_input;
import :user_result;
import :user_runtime_common;

export namespace app::handlers::user {

class UserSignupWithMerchantIdRuntime
{
public:
    explicit UserSignupWithMerchantIdRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    user_signup_with_merchant_id(
        const ::app::model::user::SignUpWithMerchantIdRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        std::string set_cookie{};


        const auto user_id = m_context.ids().uuid_v4();
        const auto email = request.get_email().get_value();
        const ::app::crypto::SecretString password_secret{request.get_password()};
        const auto password_hash =
            ::app::crypto::KeyedHasher{"user-password-pepper"}.hash(password_secret.value());

        m_support.store_password(email, password_hash);

        m_context.logger().record_flow("user_signup_with_merchant_id");
        m_context.logger().counter("user.user_signup_with_merchant_id").increment();

        const auto company_name = request.get_company_name();
        if (!company_name.empty()) {
            const auto org_id = "org_" + company_name;
            const auto merchant_id = "merchant_" + company_name;
            if (m_context.repository().find_organization_by_string(org_id)) {
                return std::unexpected{m_support.wire_error(
                    "api:UR_21",
                    "An Organization with the id already exists",
                    core::error::AppErrorKind::INTERNAL_ERROR
                )};
            }
            ::app::store::organizations::OrganizationRecord org{};
            org.set_organization_id(org_id);
            org.set_organization_name(company_name);
            org.set_organization_type("orchestration");
            const auto saved_org = m_context.repository().save_organization(std::move(org));

            ::app::store::merchant_accounts::MerchantAccountRecord merchant{};
            merchant.set_merchant_id(merchant_id);
            merchant.set_merchant_name(company_name);
            merchant.set_organization_id(saved_org.get_organization_id());
            merchant.set_merchant_account_type("orchestration");
            m_context.repository().save_merchant_account(std::move(merchant));
        }

        const auto effective_merchant =
            company_name.empty() ? std::string{} : std::string{"merchant_"} + company_name;
        m_support
            .persist_new_user(user_id, email, company_name, "active", effective_merchant, "admin");
        m_support.publish_identity_event("user.signed_up", user_id);

        using Response = ::app::model::user::AuthorizeResponse;
        Response response{};
        response.set_is_email_sent(false);
        response.set_user_id(user_id);
        auto document = ResponseDocument::from(response);
        return std::pair{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
