export module app_user:user_runtime_create_platform;

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

class CreatePlatformRuntime
{
public:
    explicit CreatePlatformRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    create_platform(const CreatePlatformInput& input) const
    {
        std::string set_cookie{};


        const auto org_name = input.request.get_organization_name();
        for (const auto& existing: m_context.repository().list_organizations()) {
            if (existing.get_organization_name() == org_name) {
                return std::unexpected{m_support.wire_error(
                    "api:UR_21",
                    "An Organization with the id already exists",
                    core::error::AppErrorKind::INTERNAL_ERROR
                )};
            }
        }

        const auto org_id_str = m_context.ids().payment_id("org_");
        const auto merchant_id_str = m_context.ids().payment_id("merchant_");
        ::app::store::organizations::OrganizationRecord org{};
        org.set_organization_id(org_id_str);
        org.set_organization_name(org_name);
        org.set_organization_type("platform");
        org.set_platform_merchant_id(merchant_id_str);
        const auto saved_org = m_context.repository().save_organization(std::move(org));

        ::app::store::merchant_accounts::MerchantAccountRecord merchant{};
        merchant.set_merchant_id(merchant_id_str);
        merchant.set_merchant_name(org_name);
        merchant.set_organization_id(saved_org.get_organization_id());
        merchant.set_merchant_account_type("platform");
        m_context.repository().save_merchant_account(std::move(merchant));

        const auto key = std::string{"user.create_platform:"} +
                         std::to_string(input.request_context.body.size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{input.request_context.body})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("create_platform");
        m_context.metrics().counter("user.create_platform").increment();

        using Response = ::app::model::user::PlatformAccountCreateResponse;
        Response response{};
        ::app::model::user::OrganizationIdValue org_id{};
        org_id.set_value(saved_org.get_organization_id());
        ::app::model::user::OrganizationTypeValue org_type{};
        org_type.set_value("platform");
        ::app::model::user::MerchantIdValue merchant_id{};
        merchant_id.set_value(merchant_id_str);
        ::app::model::user::MerchantAccountTypeValue merchant_account_type{};
        merchant_account_type.set_value("platform");
        response.set_org_id(org_id);
        response.set_org_name(input.request.get_organization_name());
        response.set_org_type(org_type);
        response.set_merchant_id(merchant_id);
        response.set_merchant_account_type(merchant_account_type);
        auto document = ResponseDocument::from(response);
        return std::pair{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
