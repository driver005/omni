export module app_handlers_user:runtime_user_merchant_account_create;

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

class UserMerchantAccountCreateRuntime
{
public:
    explicit UserMerchantAccountCreateRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<UserMerchantAccountCreateResult, core::error::AppError>
    user_merchant_account_create(const UserMerchantAccountCreateInput& input) const
    {
        std::string set_cookie{};


        const auto merchant_id_str = m_context.ids().payment_id("merchant_");
        const auto company_name = input.request.get_company_name();
        app::store::merchant_accounts::MerchantAccountRecord merchant{};
        merchant.set_merchant_id(merchant_id_str);
        merchant.set_merchant_name(company_name);
        merchant.set_organization_id(input.request_context.org_id);
        merchant.set_merchant_account_type("orchestration");
        m_context.repository().save_merchant_account(std::move(merchant));

        const auto key = std::string{"user.user_merchant_account_create:"} +
                         std::to_string(input.request_context.body.size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{input.request_context.body})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("user_merchant_account_create");
        m_context.metrics().counter("user.user_merchant_account_create").increment();

        using Response = app::model::user::UserMerchantAccountResponse;
        Response response{};
        app::model::user::MerchantIdValue merchant_id{};
        merchant_id.set_value(merchant_id_str);
        app::model::user::OptionalEncryptableNameValue merchant_name{};
        merchant_name.set_value(company_name);
        app::model::user::MerchantAccountTypeValue merchant_account_type{};
        merchant_account_type.set_value("orchestration");
        app::model::user::ApiVersionValue version{};
        version.set_value("V2");
        response.set_merchant_id(merchant_id);
        response.set_merchant_name(merchant_name);
        response.set_merchant_account_type(merchant_account_type);
        response.set_version(version);
        auto document = ResponseDocument::from(response);
        return UserMerchantAccountCreateResult{document, set_cookie};
    }

private:
    app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
