export module app_auth:oidc_runtime_authorize;

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
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_keystore;
import app_services;

import :oidc_input;
import :oidc_result;
import :oidc_runtime_common;

export namespace app::handlers::oidc {

class OidcAuthorizeRuntime
{
public:
    explicit OidcAuthorizeRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<std::pair<serde::Value, std::string>>
    authorize(
        const ::app::dto::oidc::OidcAuthorizeQuery& payload,
        const core::behavior::CallContext& ctx
    ) const
    {
        m_context.metrics().record_flow("oidc_authorize");
        m_context.metrics().counter("oidc.oidc_authorize").increment();
        m_context.metrics().counter("oidc.api_event.Oidc").increment();

        m_support.ensure_oidc_config();

        const auto& scopes = payload.get_scope();
        if (std::find(scopes.begin(), scopes.end(), ::app::dto::oidc::Scope::Openid) ==
            scopes.end()) {
            return std::unexpected{oidc_wire::oidc_error(oidc_wire::OidcErrorKind::InvalidScope)};
        }

        const auto client_id = payload.get_client_id();
        const auto registered_client = m_support.configured_oidc_client(client_id);
        if (!registered_client) {
            return std::unexpected{
                oidc_wire::oidc_error(oidc_wire::OidcErrorKind::UnauthorizedClient)
            };
        }
        if (!oidc_support::validate_redirect_uri_match(
                registered_client->redirect_uri,
                payload.get_redirect_uri()
            )) {
            return std::unexpected{oidc_wire::oidc_error(oidc_wire::OidcErrorKind::InvalidRequest)};
        }

        // Upstream: Option<UserFromToken>; missing session means AccessDenied.
        static const std::string no_user{};
        const auto user_it = ctx.claims.find("x-user-id");
        const auto& user_id = user_it == ctx.claims.end() ? no_user : user_it->second;
        if (user_id.empty()) {
            return std::unexpected{oidc_wire::oidc_error(oidc_wire::OidcErrorKind::AccessDenied)};
        }
        const auto user = m_context.repository().find_user_by_string(user_id);
        if (!user) {
            return std::unexpected{oidc_wire::oidc_error(oidc_wire::OidcErrorKind::ServerError)};
        }

        // Upstream: generate_cryptographically_secure_random_string(32).
        const auto code = oidc_support::make_alphanumeric_code(
            client_id + "|" + payload.get_redirect_uri() + "|" + payload.get_state() + "|" +
                m_context.ids().uuid_v4(),
            32
        );
        StoredAuthCode stored{};
        stored.set_sub(user_id);
        stored.set_client_id(client_id);
        stored.set_redirect_uri(payload.get_redirect_uri());
        stored.set_scope(payload.get_scope());
        stored.set_nonce(payload.get_nonce());
        stored.set_email(user->get_email());
        stored.set_is_verified(user->get_status() == "active");
        m_context.cache().put(
            std::string{REDIS_AUTH_CODE_PREFIX} + code,
            core::cache::to_cache_value(serde::Ser::encode_json(stored)),
            core::cache::Ttl{}.seconds(AUTH_CODE_TTL_IN_SECS)
        );

        const auto redirect_url = oidc_support::build_oidc_redirect_url(
            payload.get_redirect_uri(),
            code,
            payload.get_state()
        );
        if (!redirect_url) {
            return std::unexpected{oidc_wire::oidc_error(oidc_wire::OidcErrorKind::InvalidRequest)};
        }

        // ApplicationResponse::JsonForRedirection wire shape (api_models::payments v1),
        // built as a typed dynamic document: return_url, params, redirect, method, headers.
        serde::Value::Object body{};
        body.insert(std::string{"return_url"}, serde::Value{std::string{}});
        body.insert(std::string{"params"}, serde::Value{serde::Value::Array{}});
        body.insert(
            std::string{"return_url_with_query_params"},
            serde::Value{std::string{*redirect_url}}
        );
        body.insert(std::string{"http_method"}, serde::Value{std::string{}});
        body.insert(std::string{"headers"}, serde::Value{serde::Value::Array{}});
        return std::pair{serde::Value{std::move(body)}, std::string{*redirect_url}};
    }

private:
    ::app::context::AppContext& m_context;
    OidcRuntimeSupport m_support;
};

} // namespace app::handlers::oidc
