export module app_user:user_runtime_list_users_internal;

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

class ListUsersInternalRuntime
{
public:
    explicit ListUsersInternalRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    list_users_internal(const ::app::model::user::ListUsersInternalRequest& request,
        const ::core::behavior::CallContext& ctx) const
    {
        std::string set_cookie{};


        const auto limit = ctx_query(ctx).get("limit");
        (void)limit;

        m_context.metrics().record_flow("list_users_internal");
        m_context.metrics().counter("user.list_users_internal").increment();

        using Response = ::app::model::user::ListUsersInternalResponse;
        Response response{};
        std::vector<::app::model::user::GetUserInternalDetailsResponse> users{};
        users.reserve(request.get_user_ids().size());
        for (const auto& id: request.get_user_ids()) {
            const auto stored = m_context.repository().find_user_by_string(id);
            ::app::model::user::GetUserInternalDetailsResponse item{};
            item.set_user_id(id);
            item.set_is_active(stored && stored->get_status() == "active");
            users.push_back(std::move(item));
        }
        response.set_users(std::move(users));
        auto document = ResponseDocument::from(response);
        return std::pair{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
