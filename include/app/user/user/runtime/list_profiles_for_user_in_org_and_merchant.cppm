export module app_user:user_runtime_list_profiles_for_user_in_org_and_merchant;

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

class ListProfilesForUserInOrgAndMerchantRuntime
{
public:
    explicit ListProfilesForUserInOrgAndMerchantRuntime(
        ::app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ListProfilesForUserInOrgAndMerchantResult, core::error::AppError>
    list_profiles_for_user_in_org_and_merchant(
        const ListProfilesForUserInOrgAndMerchantInput& input
    ) const
    {
        std::string set_cookie{};


        const auto merchant_id = input.request_context.merchant_id;
        m_context.metrics().record_flow("list_profiles_for_user_in_org_and_merchant");
        m_context.metrics().counter("user.list_profiles_for_user_in_org_and_merchant").increment();

        using Response =
            std::vector<::app::model::user::ListProfilesForUserInOrgAndMerchantAccountResponse>;
        Response response{};
        for (const auto& connector: m_context.repository().list_merchant_connectors()) {
            if (!merchant_id.empty() && connector.get_merchant_id() != merchant_id) {
                continue;
            }
            const auto& profile_id = connector.get_profile_id();
            if (profile_id.empty()) {
                continue;
            }
            if (std::any_of(
                    response.begin(),
                    response.end(),
                    [&profile_id](const auto& item)
                    {
                        return item.get_profile_id().get_value() == profile_id;
                    }
                )) {
                continue;
            }
            ::app::model::user::ListProfilesForUserInOrgAndMerchantAccountResponse item{};
            ::app::model::user::ProfileIdValue id{};
            id.set_value(profile_id);
            item.set_profile_id(id);
            item.set_profile_name(profile_id);
            response.push_back(std::move(item));
        }
        auto document = ResponseDocument::from(response);
        return ListProfilesForUserInOrgAndMerchantResult{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
