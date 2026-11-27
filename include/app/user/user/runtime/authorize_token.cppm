export module app_user:user_runtime_authorize_token;

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

class AuthorizeTokenRuntime
{
public:
    explicit AuthorizeTokenRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    authorize_token(const AuthorizeTokenInput& input) const
    {
        std::string set_cookie{};


        // Port of core/user.rs:4220-4242 (authorize_token):
        //   1. parse `permission` as an upstream Permission; invalid -> InvalidRequestData.
        //   2. decode the JWT; a blacklisted token -> InvalidJwtToken.
        //   3. resolve role info from the token and check the permission.
        //   4. StatusOk.
        const auto permission = input.request.get_permission();
        const auto valid_permission =
            !permission.empty() &&
            std::all_of(
                permission.begin(),
                permission.end(),
                [](char c)
                {
                    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
                }
            ) &&
            (permission.ends_with("Read") || permission.ends_with("Write"));
        if (!valid_permission) {
            return std::unexpected{m_support.wire_error(
                "IR_06",
                "Invalid permission",
                core::error::AppErrorKind::VALIDATION_FAILED
            )};
        }

        const auto token = input.request.get_token();
        if (token.empty() || !m_support.user_signer().verify(token) ||
            m_support.is_blacklisted(token)) {
            return std::unexpected{m_support.wire_error(
                "IR_17",
                "Access forbidden, invalid JWT token was used",
                core::error::AppErrorKind::UNAUTHORIZED
            )};
        }
        const auto grants = m_support.jwt_claim(token, "permissions");
        const auto single = grants.empty() ? m_support.jwt_claim(token, "permission") : grants;
        if (!single.empty() && !m_support.grant_contains(single, permission)) {
            return std::unexpected{m_support.wire_error(
                "IR_18",
                "Permission denied",
                core::error::AppErrorKind::FORBIDDEN
            )};
        }

        m_context.metrics().record_flow("authorize_token");
        m_context.metrics().counter("user.authorize_token").increment();

        // Upstream StatusOk -> bare empty 200 body (services/api.rs:499,730-732).
        return std::pair{ResponseDocument::empty(), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
