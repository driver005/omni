export module app_handlers_oidc:runtime_token;

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
import app_keystore;
import app_services;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::oidc {

class OidcTokenRuntime
{
public:
    explicit OidcTokenRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<OidcTokenResult, core::error::AppError>
    token(const OidcTokenInput& input) const
    {
        // Upstream applies OIDC_CLIENT_AUTH (HTTP Basic), not a dashboard JWT.
        // The exact Basic client validation is enforced here (see input support).

        m_context.metrics().record_flow("oidc_token");
        m_context.metrics().counter("oidc.oidc_token").increment();
        m_context.metrics().counter("oidc.api_event.Oidc").increment();

        m_support.ensure_oidc_config();

        const auto& request = input.request;
        // auth::OIDC_CLIENT_AUTH: Basic client_id/client_secret vs conf.oidc.client.
        const auto client = m_support.configured_oidc_client(input.basic.first);
        if (!client || client->client_secret != input.basic.second) {
            return OidcTokenResult{oidc_wire::OidcErrorKind::InvalidBasicAuth};
        }
        const auto& client_id = client->client_id;

        // validate_token_request
        if (oidc_support::trim(request.get_redirect_uri()).empty()) {
            return OidcTokenResult{oidc_wire::OidcErrorKind::InvalidTokenRequest};
        }
        if (!oidc_support::validate_redirect_uri_match(
                client->redirect_uri,
                request.get_redirect_uri()
            )) {
            return OidcTokenResult{oidc_wire::OidcErrorKind::InvalidTokenRequest};
        }

        // validate_and_consume_authorization_code
        const auto code_key = std::string{REDIS_AUTH_CODE_PREFIX} + request.get_code();
        const auto cached = m_context.cache().get(code_key);
        if (!cached.hit) {
            return OidcTokenResult{oidc_wire::OidcErrorKind::InvalidGrant};
        }
        const auto stored = cached.value_or<std::string>("");
        m_context.cache().erase(code_key);

        auto auth_data = serde::Ser::deserialize<StoredAuthCode>("application/json", stored);
        if (!auth_data) {
            return OidcTokenResult{oidc_wire::OidcErrorKind::ServerError};
        }
        if (oidc_support::trim(auth_data->get_client_id()) != oidc_support::trim(client_id)) {
            return OidcTokenResult{oidc_wire::OidcErrorKind::InvalidGrant};
        }
        if (!oidc_support::validate_redirect_uri_match(
                auth_data->get_redirect_uri(),
                request.get_redirect_uri()
            )) {
            return OidcTokenResult{oidc_wire::OidcErrorKind::InvalidGrant};
        }

        const auto issuer = m_support.tenant_base_url();
        const auto issued_at =
            static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(
                                           std::chrono::system_clock::now().time_since_epoch()
            )
                                           .count());

        const auto id_claims =
            m_support
                .build_id_claims(issuer, *auth_data, issued_at, issued_at + ID_TOKEN_TTL_IN_SECS);
        const auto access_claims = m_support.build_access_claims(
            issuer,
            *auth_data,
            issued_at,
            issued_at + ACCESS_TOKEN_TTL_IN_SECS
        );

        const auto signer = m_support.token_signer();
        using Response = app::dto::oidc::OidcTokenResponse;
        Response response{};
        response.set_access_token(signer.sign(access_claims));
        response.set_id_token(signer.sign(id_claims));
        response.set_token_type(std::string{TOKEN_TYPE_BEARER});
        response.set_expires_in(ACCESS_TOKEN_TTL_IN_SECS);
        return OidcTokenResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    OidcRuntimeSupport m_support;
};

} // namespace app::handlers::oidc
