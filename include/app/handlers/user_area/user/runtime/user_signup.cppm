export module app_user:user_runtime_user_signup;

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

import :user_input;
import :user_result;
import :user_runtime_common;

export namespace app::handlers::user {

class UserSignupRuntime
{
public:
    explicit UserSignupRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<UserSignupResult, core::error::AppError>
    user_signup(const UserSignupInput& input) const
    {
        std::string set_cookie{};


        const auto user_id = m_context.ids().uuid_v4();
        const auto email = input.request.get_email().get_value();

        // First-run setup: create the default organization and merchant for this new
        // user so the Control Center can list/select them and enable setup actions.
        // Upstream does this in insert_user_and_merchant_in_db; the local substitute
        // persists the same records in the in-memory store.
        const auto org_id_str = m_context.ids().payment_id("org_");
        const auto merchant_id_str = m_context.ids().payment_id("merchant_");

        ::app::store::organizations::OrganizationRecord org{};
        org.set_organization_id(org_id_str);
        org.set_organization_name(email);
        org.set_organization_type("orchestration");
        m_context.repository().save_organization(std::move(org));

        ::app::store::merchant_accounts::MerchantAccountRecord merchant{};
        merchant.set_merchant_id(merchant_id_str);
        merchant.set_merchant_name(email);
        merchant.set_organization_id(org_id_str);
        merchant.set_merchant_account_type("orchestration");
        m_context.repository().save_merchant_account(std::move(merchant));

        // Local substitute for validate_email_domain_auth_type_using_db: an
        // optional config allowlist gates signup by email domain.
        const auto allowlist = m_context.repository().get_config("user.email_domain_allowlist");
        if (allowlist && !allowlist->empty() && !email.empty()) {
            const auto at = email.rfind('@');
            const auto domain = at == std::string::npos ? std::string{} : email.substr(at + 1);
            bool allowed{false};
            std::size_t cursor{0};
            while (cursor <= allowlist->size()) {
                const auto comma = allowlist->find(',', cursor);
                const auto entry = allowlist->substr(
                    cursor,
                    comma == std::string::npos ? std::string::npos : comma - cursor
                );
                if (!domain.empty() && entry == domain) {
                    allowed = true;
                    break;
                }
                if (comma == std::string::npos) {
                    break;
                }
                cursor = comma + 1;
            }
            if (!allowed) {
                return std::unexpected{m_support.wire_error(
                    "UR_16",
                    "Invalid Email",
                    core::error::AppErrorKind::INVALID_REQUEST
                )};
            }
        }

        const ::app::crypto::SecretString password_secret{input.request.get_password()};
        const auto password_hash =
            ::app::crypto::KeyedHasher{"user-password-pepper"}.hash(password_secret.value());

        m_support.store_password(email, password_hash);
        m_support.persist_new_user(user_id, email, email, "active", merchant_id_str, "admin");
        // The local role record ("role_admin") isn't one of the predefined role
        // seeds, so seed its permission groups explicitly. The Control Center
        // gates setup actions on GET /user/role/v2, which reads this cache.
        m_context.cache().put(
            "user_role.groups.role_admin",
            core::cache::to_cache_value(
                std::string{
                    "connectors_view,connectors_manage,operations_view,operations_manage,"
                    "workflows_view,workflows_manage,analytics_view,users_view,users_manage,"
                    "account_view,account_manage,configurations_view,configurations_manage,"
                    "theme_view,theme_manage,offers_view,offers_manage,clone_connector_manage"
                }
            )
        );
        m_support.publish_identity_event("user.signed_up", user_id);

        m_context.metrics().record_flow("user_signup");
        m_context.metrics().counter("user.user_signup").increment();

        const auto flow = input.request_context.query.get("flow").value_or(
            input.request_context.query.get("origin").value_or(std::string{"SIGNIN"})
        );
        const auto token_payload =
            std::format(R"({{"sub":"{}","email":"{}","flow":"{}"}})", user_id, email, flow);
        const auto token = m_support.user_signer().sign(token_payload);

        using Response = ::app::model::user::TokenResponse;
        Response response{};
        ::app::model::user::TokenPurposeValue purpose{};
        purpose.set_value("user_info");
        response.set_token(token);
        response.set_token_type(purpose);
        auto document = ResponseDocument::from(response);
        return UserSignupResult{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
