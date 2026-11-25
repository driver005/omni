export module app_handlers_api_keys:input;

import std;
import interfaces;
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
    explicit ApiKeysInputSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] bool is_v2(interfaces::io::IRequest& req) const
    {
        return app::http::RequestPath{}(req).at(0).value_or("") == "v2";
    }

    [[nodiscard]] std::string merchant_from_request(interfaces::io::IRequest& req) const
    {
        if (is_v2(req)) {
            return app::http::merchant_id_from_header{}(req);
        }
        return std::string{app::http::RequestPath{}(req).at(1).value_or("")};
    }

    [[nodiscard]] std::string key_id_from_request(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() >= 3) {
            return std::string{segments.at(2).value_or("")};
        }
        return {};
    }

    /**
     * @brief Port of the per-operation Permission from routes/api_keys.rs.
     * v2 update intentionally uses read permission (routes/api_keys.rs:235).
     */
    [[nodiscard]] app::auth::AppPermission
    permission_for(std::string_view operation, interfaces::io::IRequest& req) const
    {
        const bool write = operation == "api_key_create" || operation == "api_key_revoke" ||
                           (operation == "api_key_update" && !is_v2(req));
        return write ? app::auth::AppPermission::API_KEY_WRITE
                     : app::auth::AppPermission::API_KEY_READ;
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
        app::auth::AppPermission permission
    ) const
    {
        const auto registered = m_context.auth_rules().rule_for(operation);
        if (registered.scheme() == app::auth::AuthScheme::FORBIDDEN) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "operation is not registered in the auth rule registry"
            );
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const bool jwt = !auth.token().is_empty();
        const bool v2 = is_v2(req);

        app::auth::RouteAuthRule rule;
        if (jwt) {
            rule = app::auth::RouteAuthRuleBuilder{}
                       .scheme(app::auth::AuthScheme::JWT)
                       .permission(permission)
                       .allow_connected(true)
                       .allow_platform(true)
                       .build();
        } else if (v2) {
            rule = app::auth::RouteAuthRuleBuilder{}
                       .scheme(app::auth::AuthScheme::HEADER_MERCHANT)
                       .permission(permission)
                       .allow_connected(true)
                       .allow_platform(false)
                       .build();
        } else {
            rule = app::auth::RouteAuthRuleBuilder{}
                       .scheme(app::auth::AuthScheme::PLATFORM_ADMIN)
                       .permission(permission)
                       .allow_connected(true)
                       .allow_platform(true)
                       .build();
        }
        return m_context.authorizer().authorize_route(auth, rule);
    }

    /** @brief Exact limit/skip parse of the original list handler. */
    [[nodiscard]] app::dto::api_keys::ListApiKeyConstraints
    parse_constraints(interfaces::io::IRequest& req) const
    {
        const auto query = app::http::RequestQuery{}(req);
        app::dto::api_keys::ListApiKeyConstraints constraints;
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
    app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old core call received.

struct ApiKeyCreateInput
{
    app::dto::api_keys::CreateApiKeyRequest request{};
    std::string merchant_id{};
};

struct ApiKeyRetrieveInput
{
    std::string merchant_id{};
    std::string key_id{};
};

struct ApiKeyUpdateInput
{
    app::dto::api_keys::UpdateApiKeyRequest request{};
    std::string merchant_id{};
    std::string key_id{};
};

struct ApiKeyRevokeInput
{
    std::string merchant_id{};
    std::string key_id{};
};

struct ApiKeyListInput
{
    std::string merchant_id{};
    app::dto::api_keys::ListApiKeyConstraints constraints{};
};

} // namespace app::handlers::api_keys
