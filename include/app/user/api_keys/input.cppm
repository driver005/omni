export module app_user:api_keys_input;

import std;
import interfaces;
import core_behavior;
import core_error;
import app_context;
import app_dto;
import app_auth;
import app_http;

export namespace app::handlers::api_keys {

// Local equivalents of the upstream Cargo features that shape this module
// (routes/api_keys.rs, core/api_keys.rs, app.rs:2483-2515).
inline constexpr bool kV1 = true;    // /api_keys/{merchant_id}
inline constexpr bool kV2 = true;    // /v2/api-keys
inline constexpr bool kEmail = true; // expiry-reminder process-tracker workflow

inline constexpr std::size_t kApiKeyLength = 64; // consts.rs:53 API_KEY_LENGTH
inline constexpr std::size_t kHashKeyLen = 32;   // PlaintextApiKey::HASH_KEY_LEN
inline constexpr std::size_t kPrefixLen = 12;    // PlaintextApiKey::PREFIX_LEN
inline constexpr std::string_view kDecisionAdd = "ADD";
inline constexpr std::string_view kDecisionRevoke = "REVOKE";
inline constexpr std::string_view kExpiryRunner = "ApiKeyExpiryWorkflow";
inline constexpr std::string_view kExpiryName = "API_KEY_EXPIRY";
inline constexpr std::string_view kExpiryTag = "API_KEY";
inline constexpr std::string_view kNotFoundMessage = "API Key does not exist in our records";

/**
 * @brief Parse/auth/path/query support for the api_keys handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime.
 */
class ApiKeysInputSupport
{
public:
    explicit ApiKeysInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] bool is_v2(interfaces::io::IRequest& req) const
    {
        return ::app::http::RequestPath{}(req).at(0).value_or("") == "v2";
    }

    /**
     * @brief Port of the per-operation Permission from routes/api_keys.rs.
     * v2 update intentionally uses read permission (routes/api_keys.rs:235).
     */
    [[nodiscard]] ::app::auth::AppPermission
    permission_for(std::string_view operation, interfaces::io::IRequest& req) const
    {
        const bool write = operation == "api_key_create" || operation == "api_key_revoke" ||
                           (operation == "api_key_update" && !is_v2(req));
        return write ? ::app::auth::AppPermission::API_KEY_WRITE
                     : ::app::auth::AppPermission::API_KEY_READ;
    }

    /**
     * @brief Port of auth_type() (services/authentication.rs:6464-6475): a JWT
     * bearer token selects the JWT merchant guard; otherwise the default
     * platform-admin (v1) / header-merchant (v2) guard is used. Guard flags are
     * the upstream `allow_connected`/`allow_platform` values.
     */
    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation,
        ::app::auth::AppPermission permission
    ) const
    {
        const auto registered = m_context.auth_rules().rule_for(operation);
        if (registered.scheme() == ::app::auth::AuthScheme::FORBIDDEN) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "operation is not registered in the auth rule registry"
            );
        }

        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        const bool jwt = !auth.token().is_empty();
        const bool v2 = is_v2(req);

        ::app::auth::RouteAuthRule rule;
        if (jwt) {
            rule = ::app::auth::RouteAuthRuleBuilder{}
                       .scheme(::app::auth::AuthScheme::JWT)
                       .permission(permission)
                       .allow_connected(true)
                       .allow_platform(true)
                       .build();
        } else if (v2) {
            rule = ::app::auth::RouteAuthRuleBuilder{}
                       .scheme(::app::auth::AuthScheme::HEADER_MERCHANT)
                       .permission(permission)
                       .allow_connected(true)
                       .allow_platform(false)
                       .build();
        } else {
            rule = ::app::auth::RouteAuthRuleBuilder{}
                       .scheme(::app::auth::AuthScheme::PLATFORM_ADMIN)
                       .permission(permission)
                       .allow_connected(true)
                       .allow_platform(true)
                       .build();
        }
        return m_context.authorizer().authorize_route(auth, rule);
    }

    /** @brief Exact limit/skip parse of the original list handler. */
    [[nodiscard]] ::app::dto::api_keys::ListApiKeyConstraints
    parse_constraints(interfaces::io::IRequest& req) const
    {
        const auto query = ::app::http::RequestQuery{}(req);
        ::app::dto::api_keys::ListApiKeyConstraints constraints;
        if (const auto raw = query.get("limit")) {
            std::int64_t limit{};
            const auto converted = std::from_chars(raw->data(), raw->data() + raw->size(), limit);
            if (converted.ec == std::errc{} && converted.ptr == raw->data() + raw->size()) {
                constraints.set_limit(limit);
            }
        }
        if (const auto raw = query.get("skip")) {
            std::int64_t skip{};
            const auto converted = std::from_chars(raw->data(), raw->data() + raw->size(), skip);
            if (converted.ec == std::errc{} && converted.ptr == raw->data() + raw->size()) {
                constraints.set_skip(skip);
            }
        }
        return constraints;
    }

private:
    ::app::context::AppContext& m_context;
};

/** @brief Request snapshot shared by every api_keys runtime method. */
[[nodiscard]] inline ::core::behavior::CallContext make_call_context(interfaces::io::IRequest& req)
{
    ::core::behavior::CallContext ctx{};
    const auto auth = ::app::auth::AuthContextBuilder{}(req);
    ctx.merchant_id = std::string{::app::http::merchant_id_from_header{}(req)};
    if (ctx.merchant_id.empty()) {
        ctx.merchant_id = auth.merchant_id();
    }
    ctx.org_id = std::string{::app::http::org_id_from_header{}(req)};
    if (ctx.org_id.empty()) {
        ctx.org_id = auth.org_id();
    }
    ctx.profile_id = auth.profile_id();
    if (const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId)) {
        ctx.connected_merchant_id = std::string{*connected};
    }
    if (const auto secret = auth.claim(::app::auth::kClaimClientSecret)) {
        ctx.client_secret = std::string{*secret};
    }
    const auto segments = ::app::http::RequestPath{}(req);
    for (std::size_t i{0}; i < segments.count(); ++i) {
        ctx.path_segments.push_back(std::string{segments.at(i).value_or("")});
    }
    const auto raw_path = req.get_path();
    const auto question = raw_path.find('?');
    if (question != std::string_view::npos) {
        const std::string_view tail = raw_path.substr(question + 1);
        std::size_t start{0};
        while (start <= tail.size()) {
            const auto amp = tail.find('&', start);
            const auto end = amp == std::string_view::npos ? tail.size() : amp;
            const auto pair = tail.substr(start, end - start);
            const auto eq = pair.find('=');
            if (!pair.empty()) {
                const std::string key{eq == std::string_view::npos ? pair : pair.substr(0, eq)};
                const std::string value{
                    eq == std::string_view::npos ? std::string_view{} : pair.substr(eq + 1)
                };
                ctx.query.insert_or_assign(key, value);
            }
            if (amp == std::string_view::npos) {
                break;
            }
            start = amp + 1;
        }
    }
    constexpr std::string_view kHeaders[] = {
        "x-merchant-id",
        "x-org-id",
        "x-organization-id",
        "x-profile-id",
        "authorization",
        "x-client-secret",
        "x-api-key",
        "x-connected-merchant-id",
        "x-forwarded-for"
    };
    for (const auto name: kHeaders) {
        const auto value = req.find_header(name);
        if (!value.empty()) {
            ctx.headers.insert_or_assign(std::string{name}, std::string{value});
        }
    }
    for (const auto& [key, value]: auth.claims()) {
        ctx.claims.insert_or_assign(key, value);
    }
    ctx.claims.insert_or_assign("merchant_id", ctx.merchant_id);
    ctx.claims.insert_or_assign("org_id", ctx.org_id);
    ctx.claims.insert_or_assign("profile_id", ctx.profile_id);
    return ctx;
}

/**
 * @brief Domain authorization policy for ApiKeys.
 *
 * The route operation string selects the registered auth rule; the scope is
 * resolved from the request once, for every handler in the domain.
 */
class ApiKeysPolicy
{
public:
    explicit ApiKeysPolicy(ApiKeysInputSupport& input) noexcept :
        m_input{input}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const ::core::behavior::AuthSpec&
    ) const
    {
        return m_input.authorize_operation(req, operation, m_input.permission_for(operation, req));
    }

    [[nodiscard]] ::core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        ::core::behavior::Scope out{};
        out.merchant_id = std::string{::app::http::merchant_id_from_header{}(req)};
        if (out.merchant_id.empty()) {
            out.merchant_id = auth.merchant_id();
        }
        out.org_id = std::string{::app::http::org_id_from_header{}(req)};
        if (out.org_id.empty()) {
            out.org_id = auth.org_id();
        }
        out.profile_id = auth.profile_id();
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        if (connected.has_value()) {
            out.connected_merchant_id = std::string{*connected};
        }
        return out;
    }

    [[nodiscard]] ::core::behavior::CallContext context(interfaces::io::IRequest& req) const
    {
        return make_call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const ::core::behavior::Scope&) const
    {
        return std::nullopt;
    }


private:
    ApiKeysInputSupport& m_input;
};

using ApiKeysRequestSupport = ::core::behavior::RequestSupport<ApiKeysPolicy>;

} // namespace app::handlers::api_keys
