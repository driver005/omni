export module app_user:user_runtime_list_merchants_for_user_in_org;

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

class ListMerchantsForUserInOrgRuntime
{
public:
    explicit ListMerchantsForUserInOrgRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    list_merchants_for_user_in_org(const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx) const
    {
        std::string set_cookie{};


        const auto org_id = ctx.org_id;
        m_context.metrics().record_flow("list_merchants_for_user_in_org");
        m_context.metrics().counter("user.list_merchants_for_user_in_org").increment();

        using Response = std::vector<::app::model::user::UserMerchantAccountResponse>;
        Response response{};
        for (const auto& merchant: m_context.repository().list_merchant_accounts()) {
            if (!org_id.empty() && merchant.get_organization_id() != org_id) {
                continue;
            }
            ::app::model::user::UserMerchantAccountResponse item{};
            ::app::model::user::MerchantIdValue id{};
            id.set_value(merchant.get_merchant_id());
            ::app::model::user::OptionalEncryptableNameValue name{};
            name.set_value(merchant.get_merchant_name());
            ::app::model::user::MerchantAccountTypeValue type{};
            type.set_value(merchant.get_merchant_account_type());
            ::app::model::user::ApiVersionValue version{};
            version.set_value("V2");
            item.set_merchant_id(id);
            item.set_merchant_name(name);
            item.set_merchant_account_type(type);
            item.set_version(version);
            response.push_back(std::move(item));
        }
        auto document = ResponseDocument::from(response);
        return std::pair{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
