export module app_user:user_runtime_list_user_authentication_methods;

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

class ListUserAuthenticationMethodsRuntime
{
public:
    explicit ListUserAuthenticationMethodsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ListUserAuthenticationMethodsResult, core::error::AppError>
    list_user_authentication_methods(const ListUserAuthenticationMethodsInput& input) const
    {
        std::string set_cookie{};

        // Port of core/user.rs:2947-3001: exactly one of auth_id / email_domain
        // query parameters selects the method list; both/neither is invalid.
        const auto query = input.request_context.query;
        const auto auth_id = query.get("auth_id");
        const auto email_domain = query.get("email_domain");
        if (auth_id.has_value() == email_domain.has_value()) {
            return std::unexpected{m_support.wire_error(
                "UR_44",
                "Invalid user auth method operation",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }

        m_context.metrics().record_flow("list_user_authentication_methods");
        m_context.metrics().counter("user.list_user_authentication_methods").increment();

        using Response = std::vector<::app::model::user::UserAuthenticationMethodResponse>;
        Response response{};
        for (const auto& record: m_context.repository().list_authentication_records()) {
            if (auth_id && record.get_auth_id() != *auth_id) {
                continue;
            }
            if (email_domain && record.get_user_id() != *email_domain &&
                record.get_merchant_id() != *email_domain) {
                continue;
            }
            ::app::model::user::UserAuthenticationMethodResponse item{};
            item.set_id(record.get_auth_id());
            item.set_auth_id(record.get_auth_id());
            ::app::model::user::AuthMethodDetails details{};
            ::app::model::user::UserAuthTypeValue type{};
            type.set_value(record.get_method());
            details.set_auth_type(type);
            item.set_auth_method(details);
            item.set_allow_signup(record.get_status() == "active");
            response.push_back(std::move(item));
        }
        auto document = ResponseDocument::from(response);
        return ListUserAuthenticationMethodsResult{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
