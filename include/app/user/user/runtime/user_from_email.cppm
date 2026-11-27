export module app_user:user_runtime_user_from_email;

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

class UserFromEmailRuntime
{
public:
    explicit UserFromEmailRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    user_from_email(const UserFromEmailInput& input) const
    {
        std::string set_cookie{};


        const auto key = std::string{"user.user_from_email:"} +
                         std::to_string(input.request_context.body.size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{input.request_context.body})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("user_from_email");
        m_context.metrics().counter("user.user_from_email").increment();

        using Response = ::app::model::user::GetUserInternalDetailsResponse;
        Response response{};
        const auto token = input.request.get_token();
        const auto cached_user = m_context.cache().get("user.internal_user:" + token);
        response.set_user_id(
            cached_user.hit ? core::cache::value_as<std::string>(cached_user.value, token) : token
        );
        response.set_is_active(true);
        auto document = ResponseDocument::from(response);
        return std::pair{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
