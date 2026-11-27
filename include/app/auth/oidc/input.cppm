export module app_auth:oidc_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_dto;
import app_context_core;
import app_http;

import :oidc_runtime_common;

export namespace app::handlers::oidc {

/**
 * @brief Parse/auth/path/query support for the oidc handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input structs
 * consumed by the domain runtime. The oidc_authorize route does not apply a dashboard JWT;
 * the session identity is read here and the RFC 6749 errors are produced by the runtime.
 */
class OidcInputSupport
{
public:
    explicit OidcInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize_operation(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    /** @brief web::Query<OidcAuthorizeQuery>: query only, space-separated scope. */
    [[nodiscard]] std::optional<::app::dto::oidc::OidcAuthorizeQuery>
    parse_authorize_query(interfaces::io::IRequest& req) const
    {
        using Input = ::app::dto::oidc::OidcAuthorizeQuery;
        const auto query = ::app::http::RequestQuery{}(req);
        if (query.get("response_type").value_or("") != "code") {
            return std::nullopt;
        }
        const auto client_id = query.get("client_id");
        const auto redirect_uri = query.get("redirect_uri");
        const auto state = query.get("state");
        const auto nonce = query.get("nonce");
        const auto scope_value = query.get("scope");
        if (!client_id || !redirect_uri || !state || !nonce || !scope_value) {
            return std::nullopt;
        }

        std::vector<::app::dto::oidc::Scope> scopes{};
        std::size_t begin{0};
        while (begin < scope_value->size()) {
            while (begin < scope_value->size() &&
                   std::isspace(static_cast<unsigned char>((*scope_value)[begin]))) {
                ++begin;
            }
            if (begin >= scope_value->size()) {
                break;
            }
            std::size_t end = begin;
            while (end < scope_value->size() &&
                   !std::isspace(static_cast<unsigned char>((*scope_value)[end]))) {
                ++end;
            }
            const std::string_view token{scope_value->data() + begin, end - begin};
            if (token == "openid") {
                scopes.push_back(::app::dto::oidc::Scope::Openid);
            } else if (token == "email") {
                scopes.push_back(::app::dto::oidc::Scope::Email);
            } else if (token == "profile") {
                scopes.push_back(::app::dto::oidc::Scope::Profile);
            } else {
                return std::nullopt;
            }
            begin = end;
        }

        Input payload{};
        payload.set_response_type(::app::dto::oidc::ResponseType::Code);
        payload.set_client_id(*client_id);
        payload.set_redirect_uri(*redirect_uri);
        payload.set_scope(std::move(scopes));
        payload.set_state(*state);
        payload.set_nonce(*nonce);
        return payload;
    }

    /** @brief web::Form<OidcTokenRequest>: form-encoded only. */
    [[nodiscard]] std::optional<::app::dto::oidc::OidcTokenRequest>
    parse_token_request(interfaces::io::IRequest& req) const
    {
        const auto content_type = ::app::http::HeaderReader{}.get(req, "content-type");
        if (content_type.find("x-www-form-urlencoded") == std::string::npos) {
            return std::nullopt;
        }
        const auto body = req.body_as_string();
        const auto form = ::app::http::QueryParams{body};
        if (form.get("grant_type").value_or("") != "authorization_code") {
            return std::nullopt;
        }
        const auto code = form.get("code");
        const auto redirect_uri = form.get("redirect_uri");
        if (!code || !redirect_uri) {
            return std::nullopt;
        }
        ::app::dto::oidc::OidcTokenRequest request{};
        request.set_grant_type(::app::dto::oidc::GrantType::AuthorizationCode);
        request.set_code(*code);
        request.set_redirect_uri(*redirect_uri);
        return request;
    }

    /** @brief HTTP Basic credentials; standard base64, "client_id:client_secret". */
    [[nodiscard]] std::optional<std::pair<std::string, std::string>>
    parse_basic_auth(interfaces::io::IRequest& req) const
    {
        const auto header = ::app::http::HeaderReader{}.get(req, "authorization");
        constexpr std::string_view prefix{"Basic "};
        if (header.size() <= prefix.size()) {
            return std::nullopt;
        }
        const auto lower = [](char ch)
        {
            return ch >= 'A' && ch <= 'Z' ? static_cast<char>(ch + ('a' - 'A')) : ch;
        };
        if (!std::equal(
                prefix.begin(),
                prefix.end(),
                header.begin(),
                [&lower](char expected, char actual)
                {
                    return lower(expected) == lower(actual);
                }
            )) {
            return std::nullopt;
        }
        const auto decoded = oidc_support::decode_standard_base64(header.substr(prefix.size()));
        if (!decoded) {
            return std::nullopt;
        }
        const std::string credentials{decoded->begin(), decoded->end()};
        const auto colon = credentials.find(':');
        if (colon == std::string::npos) {
            return std::nullopt;
        }
        return std::make_pair(credentials.substr(0, colon), credentials.substr(colon + 1));
    }

    [[nodiscard]] std::string authenticated_user_id(interfaces::io::IRequest& req) const
    {
        const auto auth_context = ::app::auth::AuthContextBuilder{}(req);
        if (const auto sub = auth_context.claim("sub"); sub && !sub->empty()) {
            return std::string{*sub};
        }
        const auto read = [&req](std::string_view name)
        {
            return ::app::http::HeaderReader{}.get(req, name);
        };
        for (const auto name: {"x-user-id", "x-user", "x-user-name"}) {
            if (const auto value = read(name); !value.empty()) {
                return value;
            }
        }
        return std::string{::app::auth::TokenParser{}(req).value()};
    }

private:
    ::app::context::AppContext& m_context;
};

/** @brief Domain policy: registry-backed authorization for the JWT-gated OIDC routes. */
class OidcPolicy
{
public:
    explicit OidcPolicy(::app::context::AppContext& context) noexcept :
        m_support{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)spec;
        return m_support.authorize_operation(req, operation);
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        (void)req;
        return {};
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope& scope) const
    {
        (void)scope;
        return std::nullopt;
    }

private:
    OidcInputSupport m_support;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old core call received.

struct OidcDiscoveryInput
{};

struct JwksEndpointInput
{};

struct OidcAuthorizeInput
{
    ::app::dto::oidc::OidcAuthorizeQuery payload{};
    std::string user_id{};
};

struct OidcTokenInput
{
    ::app::dto::oidc::OidcTokenRequest request{};
    std::pair<std::string, std::string> basic{};
};

} // namespace app::handlers::oidc
