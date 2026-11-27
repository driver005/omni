export module app_user:user_runtime_terminate_auth_select;

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

class TerminateAuthSelectRuntime
{
public:
    explicit TerminateAuthSelectRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    terminate_auth_select(const TerminateAuthSelectInput& input) const
    {
        std::string set_cookie{};


        // Port of core/user.rs:3142-3188 (terminate_auth_select): the selected
        // auth method id must resolve to a known method, then a purpose-bound
        // token for the next login flow is issued together with a cookie.
        const auto owner = input.request_context.token.value();
        const auto selected = input.request.get_id();
        if (selected && !selected->empty() &&
            !m_context.repository().find_authentication_record_by_string(*selected)) {
            return std::unexpected{m_support.wire_error(
                "UR_44",
                "Invalid user auth method operation",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }
        const auto method = selected && !selected->empty() ? *selected : std::string{"password"};
        const auto payload = std::format(
            R"({{"sub":"{}","purpose":"AUTH_SELECT","auth_method":"{}"}})",
            owner,
            method
        );
        const auto token = m_support.user_signer().sign(payload);
        set_cookie = "user_token=" + token + "; HttpOnly; Path=/";
        m_context.cache().put("user.token_email:" + token, core::cache::to_cache_value(owner));

        m_context.metrics().record_flow("terminate_auth_select");
        m_context.metrics().counter("user.terminate_auth_select").increment();

        using Response = ::app::model::user::TokenResponse;
        Response response{};
        ::app::model::user::TokenPurposeValue purpose{};
        purpose.set_value("LOGIN");
        response.set_token(token);
        response.set_token_type(purpose);
        auto document = ResponseDocument::from(response);
        return std::pair{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
