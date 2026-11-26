export module app_auth:hypersense_runtime_common;

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
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;
import app_services;

export namespace app::handlers::hypersense {

// Upstream consts.rs:73 — USER_BLACKLIST_PREFIX = "BU_".
inline constexpr std::string_view kUserBlacklistPrefix{"BU_"};
// Upstream services/jwt.rs:50 — settings.secrets.jwt_secret. The local key store /
// config store mirrors the single Settings secret shared by AuthToken and ExternalToken.
inline constexpr std::string_view kJwtPurpose{"jwt"};
inline constexpr std::string_view kJwtSigningKeyId{"user-jwt-v2"};
inline constexpr std::string_view kJwtFallbackKeyId{"user-jwt-secret"};
inline constexpr std::string_view kJwtConfigKey{"jwt.secret"};
inline constexpr std::string_view kDefaultJwtSecret{"user-jwt-secret"};
// Upstream consts.rs:64 — JWT_TOKEN_TIME_IN_SECS = 60 * 60 * 24 * 2.
inline constexpr std::int64_t kExternalTokenTtlSeconds{172'800};
// Upstream authentication.rs:270-275 — ExternalServiceType has one snake_case variant.
inline constexpr std::string_view kHypersenseServiceType{"hypersense"};

/** @brief Port of authentication::ExternalServiceType (authentication.rs:270-275). */
enum class ExternalServiceType : std::uint8_t
{
    Hypersense,
};

/** @brief Typed api_locking::LockAction::NotApplicable (hypersense.rs:32,52,76). */
struct LockActionNotApplicable
{};

/** @brief authentication.rs:359-367 UserFromToken, reduced to the fields used here. */
struct UserFromToken
{
    std::string user_id{};
    std::string merchant_id{};
};

/** @brief authentication.rs:6824-6830 ExternalToken (the signed claim set). */
struct ExternalToken
{
    std::string user_id{};
    std::string merchant_id{};
    std::int64_t exp{0};
    ExternalServiceType external_service_type{ExternalServiceType::Hypersense};
};

/** @brief authentication.rs:322-331 AuthToken plus the locally issued `sub` alias. */
struct AuthToken
{
    std::string user_id{};
    std::string merchant_id{};
    std::string role_id{};
    std::string org_id{};
    std::string profile_id{};
    std::string tenant_id{};
    std::int64_t exp{0};
};

struct ExternalClaims
{
    std::string user_id{};
    std::string merchant_id{};
    std::string external_service_type{};
    std::int64_t exp{0};
};

/**
 * @brief Shared JWT / blacklist / token helpers for the hypersense sub-runtimes.
 */
class HypersenseRuntimeSupport
{
public:
    explicit HypersenseRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    void record_flow(std::string_view flow, LockActionNotApplicable) const
    {
        // Upstream server_wrap/instrument + typed Flow (hypersense.rs:15-16,42-43,62-63)
        // are framework plumbing with no observable behavior; NotApplicable acquires no
        // lock here or upstream — no behavior to port, only flow instrumentation runs.
        m_context.metrics().record_flow(flow);
    }

    /** @brief state.conf.secrets.jwt_secret (jwt.rs:50), resolved from local infra. */
    [[nodiscard]] std::string jwt_secret() const
    {
        if (const auto stored =
                m_context.secret_resolver().resolve(kJwtPurpose, std::string{kJwtSigningKeyId});
            stored && !stored->empty()) {
            return *stored;
        }
        if (const auto stored = m_context.key_store().get(kJwtPurpose, kJwtFallbackKeyId);
            stored && !stored->value().empty()) {
            return stored->value();
        }
        if (const auto configured = m_context.repository().get_config(std::string{kJwtConfigKey});
            configured && !configured->empty()) {
            return *configured;
        }
        return std::string{kDefaultJwtSecret};
    }

    /**
     * @brief authentication.rs:6335-6351 decode_jwt_verified — verify the JWT signature
     * and header before any payload is trusted. The local JwtSigner uses the same
     * HS256 wire shape (crypto/token.cppm:386-397,488-498).
     */
    [[nodiscard]] std::optional<std::string> verified_payload_json(std::string_view token) const
    {
        if (token.empty()) {
            return std::nullopt;
        }
        const auto signer = ::app::crypto::JwtSigner{jwt_secret()};
        if (!signer.verify(token)) {
            return std::nullopt;
        }
        const auto first = token.find('.');
        const auto second = token.find('.', first + 1);
        if (first == std::string_view::npos || second == std::string_view::npos) {
            return std::nullopt;
        }
        const auto payload =
            ::app::crypto::Base64Url{}.decode(token.substr(first + 1, second - first - 1));
        if (payload.empty()) {
            return std::nullopt;
        }
        auto generic = serde::Ser::decode_generic("application/json", payload);
        if (!generic || !std::holds_alternative<interfaces::Value::Object>(generic->get())) {
            return std::nullopt;
        }
        return payload;
    }

    void read_string(
        const interfaces::Value::Object& object,
        std::string_view key,
        std::string& target
    ) const
    {
        const auto field = object.get(std::string{key});
        if (!field) {
            return;
        }
        if (const auto* text = std::get_if<std::string>(&field.value().get())) {
            target = *text;
        }
    }

    [[nodiscard]] std::int64_t read_expiry(const interfaces::Value::Object& object) const
    {
        const auto field = object.get("exp");
        if (!field) {
            return 0;
        }
        if (const auto* number = std::get_if<std::int64_t>(&field.value().get())) {
            return *number;
        }
        if (const auto* number = std::get_if<double>(&field.value().get())) {
            return static_cast<std::int64_t>(*number);
        }
        return 0;
    }

    /** @brief jsonwebtoken Validation::new(HS256): expire when exp is present. */
    [[nodiscard]] bool expiry_valid(const interfaces::Value::Object& object) const
    {
        const auto field = object.get("exp");
        if (!field) {
            return true;
        }
        const auto exp = read_expiry(object);
        if (exp <= 0) {
            return false;
        }
        const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now().time_since_epoch()
        )
                             .count();
        return exp > now;
    }

    /** @brief authentication.rs:6353-6364 decode_jwt::<ExternalToken>. */
    [[nodiscard]] std::optional<ExternalClaims> decode_jwt_external(std::string_view token) const
    {
        const auto payload = verified_payload_json(token);
        if (!payload) {
            return std::nullopt;
        }
        auto generic = serde::Ser::decode_generic("application/json", *payload);
        if (!generic) {
            return std::nullopt;
        }
        const auto* object = std::get_if<interfaces::Value::Object>(&generic->get());
        if (object == nullptr || !expiry_valid(*object)) {
            return std::nullopt;
        }

        ExternalClaims claims{};
        read_string(*object, "user_id", claims.user_id);
        read_string(*object, "merchant_id", claims.merchant_id);
        read_string(*object, "external_service_type", claims.external_service_type);
        claims.exp = read_expiry(*object);

        if (claims.user_id.empty() || claims.exp <= 0) {
            return std::nullopt;
        }
        return claims;
    }

    /** @brief parse_jwt_payload::<AuthToken> (authentication.rs:5394,5432). */
    [[nodiscard]] std::optional<AuthToken> decode_jwt_auth_token(std::string_view token) const
    {
        const auto payload = verified_payload_json(token);
        if (!payload) {
            return std::nullopt;
        }
        auto generic = serde::Ser::decode_generic("application/json", *payload);
        if (!generic) {
            return std::nullopt;
        }
        const auto* object = std::get_if<interfaces::Value::Object>(&generic->get());
        if (object == nullptr || !expiry_valid(*object)) {
            return std::nullopt;
        }

        AuthToken claims{};
        read_string(*object, "user_id", claims.user_id);
        // Locally issued dashboard tokens use `sub` for the same identity.
        if (claims.user_id.empty()) {
            read_string(*object, "sub", claims.user_id);
        }
        read_string(*object, "merchant_id", claims.merchant_id);
        read_string(*object, "role_id", claims.role_id);
        read_string(*object, "org_id", claims.org_id);
        read_string(*object, "profile_id", claims.profile_id);
        read_string(*object, "tenant_id", claims.tenant_id);
        claims.exp = read_expiry(*object);

        if (claims.user_id.empty()) {
            return std::nullopt;
        }
        return claims;
    }

    /** @brief authentication.rs:6851-6863 ExternalToken::check_service_type. */
    [[nodiscard]] bool
    check_service_type(const ExternalClaims& claims, ExternalServiceType required) const noexcept
    {
        if (required != ExternalServiceType::Hypersense) {
            return false;
        }
        return claims.external_service_type == kHypersenseServiceType;
    }

    /** @brief authentication/blacklist.rs:70-83 check_user_in_blacklist. */
    [[nodiscard]] bool
    check_user_in_blacklist(std::string_view user_id, std::int64_t token_expiry) const
    {
        if (user_id.empty() || token_expiry <= 0) {
            return false;
        }
        const auto key = std::string{kUserBlacklistPrefix} + std::string{user_id};
        const auto stored = m_context.repository().get_config(key);
        if (!stored) {
            return false;
        }
        std::int64_t blacklisted_at{0};
        try {
            blacklisted_at = std::stoll(*stored);
        } catch (...) {
            return false;
        }
        return blacklisted_at > (token_expiry - kExternalTokenTtlSeconds);
    }

    /** @brief authentication/blacklist.rs:24-38 insert_user_in_blacklist. */
    bool insert_user_in_blacklist(std::string_view user_id) const
    {
        if (user_id.empty()) {
            return false;
        }
        const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now().time_since_epoch()
        )
                             .count();
        const auto key = std::string{kUserBlacklistPrefix} + std::string{user_id};
        m_context.repository().set_config(key, std::to_string(now));
        // Redis key expiry mirror (an allowed in-process cache category).
        m_context.cache().put(
            key,
            core::cache::to_cache_value(now),
            core::cache::Ttl{}.seconds(kExternalTokenTtlSeconds)
        );
        return true;
    }

    /** @brief authorization::check_tenant (external_service_auth.rs:119). */
    [[nodiscard]] bool tenant_matches(std::string_view tenant_id) const
    {
        if (tenant_id.empty()) {
            return true;
        }
        const auto configured = m_context.config_store().get_value("tenant", "tenant_id");
        if (!configured || configured->empty()) {
            return true;
        }
        return tenant_id == *configured;
    }

    /**
     * @brief authentication.rs:5378-5419 DashboardNoPermissionAuth::authenticate_and_fetch.
     *
     * allow_connected/allow_platform are inert for the dashboard JWT scheme; the
     * authenticating work is parse JWT -> check blacklist -> check tenant -> UserFromToken.
     */
    [[nodiscard]] std::expected<UserFromToken, core::error::AppError>
    authenticate_dashboard(std::string_view bearer, std::string_view header_merchant_id) const
    {
        if (bearer.empty()) {
            return std::unexpected{
                make_error(core::error::AppErrorKind::UNAUTHORIZED, "Unauthorized")
            };
        }
        const auto claims = decode_jwt_auth_token(bearer);
        if (!claims) {
            return std::unexpected{
                make_error(core::error::AppErrorKind::UNAUTHORIZED, "InvalidJwtToken")
            };
        }
        if (check_user_in_blacklist(claims->user_id, claims->exp)) {
            return std::unexpected{
                make_error(core::error::AppErrorKind::UNAUTHORIZED, "InvalidJwtToken")
            };
        }
        if (!tenant_matches(claims->tenant_id)) {
            return std::unexpected{
                make_error(core::error::AppErrorKind::FORBIDDEN, "AccessForbidden")
            };
        }

        UserFromToken user{};
        user.user_id = claims->user_id;
        user.merchant_id = claims->merchant_id;
        // Locally issued tokens carry the merchant id as a request header instead of a
        // claim; both spellings are read before the JWT is built.
        if (user.merchant_id.empty()) {
            user.merchant_id = std::string{header_merchant_id};
        }
        return user;
    }

    /**
     * @brief core/external_service_auth.rs:24-49 generate_external_token, using
     * authentication.rs:6833-6849 ExternalToken::new_token. The payload is exactly
     * { user_id, merchant_id, exp, external_service_type } (no fabricated jti).
     */
    [[nodiscard]] std::optional<std::string>
    generate_external_token(const UserFromToken& user) const
    {
        const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now().time_since_epoch()
        )
                             .count();
        ExternalToken token{};
        token.user_id = user.user_id;
        token.merchant_id = user.merchant_id;
        token.exp = now + kExternalTokenTtlSeconds;
        token.external_service_type = ExternalServiceType::Hypersense;

        const auto payload = std::format(
            R"({{"user_id":{},"merchant_id":{},"exp":{},"external_service_type":{}}})",
            serde::Ser::encode_json(token.user_id),
            serde::Ser::encode_json(token.merchant_id),
            token.exp,
            serde::Ser::encode_json(std::string{kHypersenseServiceType})
        );
        const auto jwt_signed = (::app::crypto::JwtSigner{jwt_secret()}).sign(payload);
        if (jwt_signed.empty()) {
            return std::nullopt;
        }
        return jwt_signed;
    }

    /** @brief global_store.find_active_user_by_user_id (external_service_auth.rs:85-90). */
    [[nodiscard]] std::optional<::app::store::users::UserRecord>
    find_active_user_by_user_id(std::string_view user_id) const
    {
        auto user = m_context.repository().find_user_by_string(user_id);
        if (!user || user->get_status() != "active") {
            return std::nullopt;
        }
        return user;
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::hypersense
