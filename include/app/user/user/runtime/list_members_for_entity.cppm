export module app_user:user_runtime_list_members_for_entity;

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

class ListMembersForEntityRuntime
{
public:
    explicit ListMembersForEntityRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    list_members_for_entity(const interfaces::Value& generic,
        const ::core::behavior::CallContext& ctx) const
    {
        std::string set_cookie{};


        const auto org_id = ctx.org_id;
        const auto merchant_id = ctx.merchant_id;
        m_context.metrics().record_flow("list_members_for_entity");
        m_context.metrics().counter("user.list_members_for_entity").increment();

        using Response = ::app::dto::user_role::ListUsersInEntityResponse;
        Response response{};
        for (const auto& role: m_context.repository().list_user_roles()) {
            if (!org_id.empty() && role.get_organization_id() != org_id) {
                continue;
            }
            if (!merchant_id.empty() && role.get_merchant_id() != merchant_id) {
                continue;
            }
            const auto user = m_context.repository().find_user_by_string(role.get_user_id());
            ::app::dto::user_role::EmailValue email{};
            email.set_value(user ? user->get_email() : role.get_user_id());
            std::vector<::app::dto::user_role::MinimalRoleInfoValue> roles{};
            ::app::dto::user_role::MinimalRoleInfoValue role_value{};
            role_value.set_value(role.get_role_id());
            roles.push_back(std::move(role_value));
            response.set_email(email);
            response.set_roles(std::move(roles));
            break;
        }
        auto document = ResponseDocument::from(response);
        return std::pair{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
