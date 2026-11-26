export module app_user:user_runtime_list_orgs_for_user;

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

class ListOrgsForUserRuntime
{
public:
    explicit ListOrgsForUserRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ListOrgsForUserResult, core::error::AppError>
    list_orgs_for_user(const ListOrgsForUserInput& input) const
    {
        std::string set_cookie{};


        m_context.metrics().record_flow("list_orgs_for_user");
        m_context.metrics().counter("user.list_orgs_for_user").increment();

        using Response = std::vector<::app::model::user::ListOrgsForUserResponse>;
        Response response{};
        for (const auto& org: m_context.repository().list_organizations()) {
            ::app::model::user::ListOrgsForUserResponse item{};
            ::app::model::user::OrganizationIdValue id{};
            id.set_value(org.get_organization_id());
            ::app::model::user::OrganizationTypeValue type{};
            type.set_value(org.get_organization_type());
            item.set_org_id(id);
            item.set_org_name(org.get_organization_name());
            item.set_org_type(type);
            response.push_back(std::move(item));
        }
        auto document = ResponseDocument::from(response);
        return ListOrgsForUserResult{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
