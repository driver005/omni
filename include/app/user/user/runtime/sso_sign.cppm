export module app_user:user_runtime_sso_sign;

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

class SsoSignRuntime
{
public:
    explicit SsoSignRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    sso_sign(const SsoSignInput& input) const
    {
        std::string set_cookie{};


        const auto state_entry =
            m_context.repository().get_config("user.sso_state." + input.request.get_state());
        if (!state_entry) {
            return std::unexpected{m_support.wire_error(
                "UR_46",
                "Invalid SSO request",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }
        const auto sso_payload = std::format(
            R"({{"code":"{}","state":"{}"}})",
            input.request.get_code(),
            input.request.get_state()
        );
        const auto sso_token = m_support.user_signer().sign(sso_payload);

        m_context.metrics().record_flow("sso_sign");
        m_context.metrics().counter("user.sso_sign").increment();

        using Response = ::app::model::user::TokenResponse;
        Response response{};
        ::app::model::user::TokenPurposeValue purpose{};
        purpose.set_value("LOGIN");
        response.set_token(sso_token);
        response.set_token_type(purpose);
        auto document = ResponseDocument::from(response);
        return std::pair{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
