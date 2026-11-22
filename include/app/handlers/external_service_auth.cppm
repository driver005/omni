module;
export module app_handlers:external_service_auth;

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
import app_store;
import app_http;
import app_crypto;
import app_metrics;
import app_keystore;
import app_services;

export namespace app::handlers::external_service_auth {

// ── Upstream constants (exact values) ───────────────────────────────────────
// consts.rs:64 — JWT_TOKEN_TIME_IN_SECS = 60 * 60 * 24 * 2.
inline constexpr std::int64_t kJwtTokenTimeInSecs{172800};
// consts.rs:73,75,77 — Redis key prefixes.
inline constexpr std::string_view kUserBlacklistPrefix{"BU_"};
inline constexpr std::string_view kRoleBlacklistPrefix{"BR_"};
inline constexpr std::string_view kUserIdleSessionPrefix{"IS_"};
// authorization.rs:72-74 — role info cache key prefix.
inline constexpr std::string_view kRoleInfoCachePrefix{"CR_INFO_"};
// jwt.rs:50 + services/authentication.rs:6353-6364 — state.conf.secrets.jwt_secret.
inline constexpr std::string_view kJwtPurpose{"jwt"};
inline constexpr std::string_view kJwtSigningKeyId{"user-jwt-v2"};
inline constexpr std::string_view kJwtFallbackKeyId{"user-jwt-secret"};
inline constexpr std::string_view kJwtConfigKey{"jwt.secret"};
inline constexpr std::string_view kDefaultJwtSecret{"user-jwt-secret"};
// core/external_service_auth.rs:22,130 — OFFER_ENGINE_CONTEXT.
inline constexpr std::string_view kOfferEngineContext{"MERCHANT"};

// Exact upstream error wire values (hyperswitch_domain_models api_error_response.rs:
// 629,768,643-645 + api_models/errors/types.rs:60-70,159-176).
inline constexpr std::string_view kInvalidRequestType{"invalid_request"};
inline constexpr std::string_view kInvalidJwtCode{"IR_17"};
inline constexpr std::string_view kInvalidJwtMessage{
    "Access forbidden, invalid JWT token was used"};
inline constexpr std::string_view kExpiredJwtCode{"IR_48"};
inline constexpr std::string_view kExpiredJwtMessage{
    "Access forbidden, expired JWT token was used"};
inline constexpr std::string_view kAccessForbiddenCode{"IR_22"};
inline constexpr std::string_view kAccessForbiddenMessage{
    "Access forbidden. Not authorized to access this resource offer_engine"};

/*
 * Request/response wire shapes are owned by the typed DTO partition.
 *
 * `app::dto::external_service_auth::ValidateTokenRequest` now carries a real
 * snake_case serde for `ValidatingService` (FieldConverter maps the typed enum to
 * "offer_engine"), so this handler deserializes the upstream request shape
 * directly — no local wire normalizer. Likewise
 * `app::dto::external_service_auth::ExternalVerifyTokenResponse` is now the real
 * typed OfferEngine arm (the upstream enum is `#[serde(untagged)]`), so the
 * response is emitted straight from the DTO with the exact upstream field order.
 * See the parity report for the DTO promotion evidence.
 */

/** @brief Port of authentication::AuthToken (authentication.rs:322-331). */
struct AuthToken
{
    std::string user_id{};
    std::string merchant_id{};
    std::string role_id{};
    std::int64_t exp{0};
    std::string org_id{};
    std::string profile_id{};
    std::optional<std::string> tenant_id{};
};

/** @brief JwtDecodeOutcome mapping (authentication.rs:6353-6364). */
enum class JwtDecodeOutcome
{
    Valid,
    Invalid,
    Expired,
};

/** @brief Outcome of decode_jwt::<AuthToken>: reconstructed claims plus reason. */
struct DecodedToken
{
    AuthToken token{};
    JwtDecodeOutcome outcome{JwtDecodeOutcome::Invalid};
};

/** @brief authorization::permissions::Permission restricted to Resource::Offers.
 *
 *  generate_permissions! declares Offers with scopes [Read, Write]
 *  (authorization/permissions.rs:130-133); Permission::iter() therefore yields
 *  these two in this order and their Display values are the snake_case names.
 */
enum class OffersPermission
{
    Read,
    Write,
};

[[nodiscard]] constexpr std::string_view permission_name(OffersPermission permission) noexcept
{
    return permission == OffersPermission::Read ? "offers_read" : "offers_write";
}

/** @brief offer_engine::OfferEngineCredentialSource (core/offer_engine/types.rs:19).
 *  Reuses the shared DTO enum (dto/offer_engine.cppm:20-25) rather than a local copy. */
using OfferEngineCredentialSource = app::dto::offer_engine::OfferEngineCredentialSource;

/** @brief Local mirror of roles::RoleInfo (authorization/roles.rs:11-90).
 *
 *  `grants` is the local in-process stand-in for the role's permission groups:
 *  upstream `check_permission_exists` expands groups and checks resource/scope;
 *  here the expanded grant names are held directly, so the Offers filter below
 *  is byte-for-byte the upstream iteration result.
 */
struct RoleInfo
{
    std::string role_id{};
    std::vector<std::string> grants{};

    [[nodiscard]] bool check_permission_exists(OffersPermission permission) const noexcept
    {
        const auto wanted = permission_name(permission);
        return std::ranges::any_of(
            grants, [wanted](std::string_view grant) { return grant == wanted; });
    }
};

/** @brief Result of AuthToken::check_in_blacklist (blacklist.rs:143-166). */
enum class BlacklistOutcome
{
    NotBlacklisted,
    Blacklisted,   // InvalidJwtToken
    IdleExpired,   // ExpiredJwtToken
};

} // namespace app::handlers::external_service_auth

export namespace app::handlers::external_service_auth {

class ExternalServiceAuthRouteHandlers
{
public:
    explicit ExternalServiceAuthRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    /** @brief routes/external_service_auth.rs:11-27 — validate_token. */
    void handle_validate_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // routes/external_service_auth.rs:23-24 — authentication::NoAuth with
        // LockAction::NotApplicable. "validate_token" resolves to NO_AUTH
        // (auth/registry.cppm:456); authorize_route is an exact no-op
        // (auth/authorizer.cppm:59-62). No lock is acquired, matching NotApplicable.
        const auto rule = m_context->auth_rules().rule_for("validate_token");
        const auto auth = m_context->authorizer().authorize_route(
            app::auth::AuthContextBuilder{}(req), rule
        );
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        // Deserialize the exact upstream request shape directly. The typed DTO's
        // `ValidatingService` FieldConverter accepts the snake_case wire value
        // "offer_engine" and rejects anything else, matching upstream
        // `#[serde(rename_all = "snake_case")]` (api_models:18-29).
        using Input = app::dto::external_service_auth::ValidateTokenRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const auto& request = *parsed;

        // core/external_service_auth.rs:111 — req.token.expose().
        const app::crypto::SecretString token_secret{request.get_token()};
        const auto token = token_secret.value();

        // core/external_service_auth.rs:113 — decode_jwt::<AuthToken>(&token, &state).
        const auto decoded = decode_jwt(token);
        if (decoded.outcome == JwtDecodeOutcome::Expired) {
            send_api_error(res, send, kInvalidRequestType, kExpiredJwtCode,
                           kExpiredJwtMessage, interfaces::io::types::Status::UNAUTHORIZED);
            return;
        }
        if (decoded.outcome != JwtDecodeOutcome::Valid) {
            send_api_error(res, send, kInvalidRequestType, kInvalidJwtCode,
                           kInvalidJwtMessage, interfaces::io::types::Status::UNAUTHORIZED);
            return;
        }
        const auto& payload = decoded.token;

        // core/external_service_auth.rs:115-117 — payload.check_in_blacklist(&state).
        const auto blacklist = check_in_blacklist(payload);
        if (blacklist == BlacklistOutcome::Blacklisted) {
            send_api_error(res, send, kInvalidRequestType, kInvalidJwtCode,
                           kInvalidJwtMessage, interfaces::io::types::Status::UNAUTHORIZED);
            return;
        }
        if (blacklist == BlacklistOutcome::IdleExpired) {
            send_api_error(res, send, kInvalidRequestType, kExpiredJwtCode,
                           kExpiredJwtMessage, interfaces::io::types::Status::UNAUTHORIZED);
            return;
        }

        // core/external_service_auth.rs:119 — check_tenant(payload.tenant_id, state.tenant).
        if (!check_tenant(payload.tenant_id)) {
            send_api_error(res, send, kInvalidRequestType, kInvalidJwtCode,
                           kInvalidJwtMessage, interfaces::io::types::Status::UNAUTHORIZED);
            return;
        }

        // core/external_service_auth.rs:121 — get_role_info(&state, &payload).
        const auto role_info = get_role_info(payload);
        if (!role_info) {
            send_api_error(res, send, kInvalidRequestType, kInvalidJwtCode,
                           kInvalidJwtMessage, interfaces::io::types::Status::UNAUTHORIZED);
            return;
        }

        // core/external_service_auth.rs:123-136 — match ValidatingService::OfferEngine.
        switch (request.get_service()) {
        case app::dto::external_service_auth::ValidatingService::OfferEngine: {
            // core:125 — resolve_offer_engine_merchant_id.
            const auto merchant_id = resolve_offer_engine_merchant_id(payload);
            if (!merchant_id) {
                send_api_error(res, send, kInvalidRequestType, kAccessForbiddenCode,
                               kAccessForbiddenMessage,
                               interfaces::io::types::Status::FORBIDDEN);
                return;
            }

            // routes/external_service_auth.rs:16-26 — Flow::ExternalServiceValidateToken,
            // LockAction::NotApplicable (no lock acquired). Local metrics substitute.
            m_context->metrics().record_flow("ExternalServiceValidateToken");
            m_context->metrics().counter("EXTERNAL_SERVICE_TOKEN_VALIDATED").increment();

            // Typed OfferEngine arm of the untagged upstream response enum; the
            // DTO's Serializable emits the exact field order from api_models:40-45.
            using Response = app::dto::external_service_auth::ExternalVerifyTokenResponse;
            Response response{};
            response.set_merchant_id(*merchant_id);
            response.set_context(std::string{kOfferEngineContext});
            response.set_token(
                token_secret.masked_as(app::crypto::SecretFieldKind::ClientSecret));
            response.set_permissions(offer_engine_permissions(*role_info));
            send_json(res, send, serde::Ser::encode_json(response), true);
            return;
        }
        }

        send_error(res, send, core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INVALID_REQUEST, "unsupported validating service"));
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Upstream mounts exactly POST /external-service/validate-token
        // (app.rs:2004-2009; routes/external_service_auth.rs:11-27). No other
        // route exists in this domain upstream; the old C++-only prefixed alias
        // was removed for strict parity.
        Router{router, "external-service"}
            .add_route(
                Route{"validate-token"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_validate_token(req, res, std::move(send));
                    })
                    .summary("Hyperswitch validate_token (upstream path)")
            );
    }

private:
    // ── decode_jwt::<AuthToken> ───────────────────────────────────────────────

    /** @brief state.conf.secrets.jwt_secret resolved from local infra.
     *  Mirrors hypersense.cppm:342-360 and supports the same local token issuer
     *  (user.cppm:3822-3830 signs with "jwt"/"user-jwt-v2"). */
    [[nodiscard]] std::string jwt_secret() const
    {
        if (const auto stored = m_context->secret_resolver().resolve(
                kJwtPurpose, std::string{kJwtSigningKeyId});
            stored && !stored->empty()) {
            return *stored;
        }
        if (const auto stored = m_context->key_store().get(
                kJwtPurpose, kJwtFallbackKeyId);
            stored && !stored->value().empty()) {
            return stored->value();
        }
        if (const auto configured = m_context->repository().get_config(
                std::string{kJwtConfigKey});
            configured && !configured->empty()) {
            return *configured;
        }
        return std::string{kDefaultJwtSecret};
    }

    /** @brief authentication.rs:6334-6364 decode_jwt_verified + decode_jwt error map. */
    [[nodiscard]] DecodedToken decode_jwt(std::string_view token) const
    {
        DecodedToken result{};
        if (token.empty()) {
            result.outcome = JwtDecodeOutcome::Invalid;
            return result;
        }

        const auto signer = app::crypto::JwtSigner{jwt_secret()};
        const auto verification = signer.verify_with_options(
            token, app::crypto::JwtVerificationOptions{}.require_expiry(false));
        if (!verification.ok()) {
            // The local JwtSigner reports "expired" only when require_expiry is
            // enabled; with it disabled, an expired exp is still read below and
            // mapped to ExpiredJwtToken exactly as jsonwebtoken would.
            result.outcome = JwtDecodeOutcome::Invalid;
            return result;
        }

        const std::size_t first = token.find('.');
        const std::size_t second = token.find('.', first + 1);
        if (first == std::string_view::npos || second == std::string_view::npos) {
            result.outcome = JwtDecodeOutcome::Invalid;
            return result;
        }
        const auto payload = app::crypto::Base64Url{}.decode(
            token.substr(first + 1, second - first - 1)
        );
        if (payload.empty()) {
            result.outcome = JwtDecodeOutcome::Invalid;
            return result;
        }
        auto generic = serde::Ser::decode_generic("application/json", payload);
        if (!generic) {
            result.outcome = JwtDecodeOutcome::Invalid;
            return result;
        }
        const auto* object = std::get_if<interfaces::Value::Object>(&generic->get());
        if (object == nullptr) {
            result.outcome = JwtDecodeOutcome::Invalid;
            return result;
        }

        const auto read = [object](std::string_view key, std::string& target)
        {
            const auto field = object->get(std::string{key});
            if (!field) {
                return;
            }
            if (const auto* text = std::get_if<std::string>(&field.value().get())) {
                target = *text;
            }
        };
        read("user_id", result.token.user_id);
        // Locally issued dashboard tokens also carry `sub` (hypersense.cppm:486-489).
        if (result.token.user_id.empty()) {
            read("sub", result.token.user_id);
        }
        read("merchant_id", result.token.merchant_id);
        read("role_id", result.token.role_id);
        read("org_id", result.token.org_id);
        read("profile_id", result.token.profile_id);

        if (const auto field = object->get("tenant_id")) {
            if (const auto* text = std::get_if<std::string>(&field.value().get())) {
                result.token.tenant_id = *text;
            }
        }

        if (const auto field = object->get("exp")) {
            if (const auto* number = std::get_if<std::int64_t>(&field.value().get())) {
                result.token.exp = *number;
            } else if (const auto* number = std::get_if<double>(&field.value().get())) {
                result.token.exp = static_cast<std::int64_t>(*number);
            }
        }

        // Validation::new(HS256) validates `exp` when present.
        if (result.token.exp > 0) {
            const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
            if (result.token.exp <= now) {
                result.outcome = JwtDecodeOutcome::Expired;
                return result;
            }
        }
        // jsonwebtoken Validation::new(HS256) requires an `exp` claim, and
        // AuthToken's non-Option fields must deserialize (user_id, merchant_id,
        // role_id, org_id, profile_id). Missing values are InvalidJwtToken.
        if (result.token.exp <= 0 || result.token.user_id.empty() ||
            result.token.merchant_id.empty() || result.token.role_id.empty()) {
            result.outcome = JwtDecodeOutcome::Invalid;
            return result;
        }
        result.outcome = JwtDecodeOutcome::Valid;
        return result;
    }

    // ── AuthToken::check_in_blacklist ─────────────────────────────────────────

    /** @brief blacklist.rs:70-83 check_user_in_blacklist. */
    [[nodiscard]] bool check_user_in_blacklist(
        std::string_view user_id, std::int64_t token_expiry) const
    {
        return check_principal_in_blacklist(kUserBlacklistPrefix, user_id, token_expiry);
    }

    /** @brief blacklist.rs:85-98 check_role_in_blacklist. */
    [[nodiscard]] bool check_role_in_blacklist(
        std::string_view role_id, std::int64_t token_expiry) const
    {
        return check_principal_in_blacklist(kRoleBlacklistPrefix, role_id, token_expiry);
    }

    [[nodiscard]] bool check_principal_in_blacklist(
        std::string_view prefix, std::string_view id, std::int64_t token_expiry) const
    {
        if (id.empty() || token_expiry <= 0) {
            return false;
        }
        const auto key = std::string{prefix} + std::string{id};
        const auto stored = m_context->repository().get_config(key);
        if (!stored) {
            return false;
        }
        std::int64_t blacklisted_at{0};
        try {
            blacklisted_at = std::stoll(*stored);
        } catch (...) {
            return false;
        }
        return blacklisted_at > (token_expiry - kJwtTokenTimeInSecs);
    }

    /** @brief idle_session.rs:40-74 check_idle_session. */
    [[nodiscard]] BlacklistOutcome check_idle_session(std::string_view user_id) const
    {
        if (user_id.empty()) {
            return BlacklistOutcome::NotBlacklisted;
        }
        const auto timeout = m_context->config_store().get_value(
            "user", "idle_session_timeout_in_secs");
        if (!timeout || timeout->empty()) {
            return BlacklistOutcome::NotBlacklisted;
        }
        std::int64_t seconds{0};
        try {
            seconds = std::stoll(*timeout);
        } catch (...) {
            return BlacklistOutcome::NotBlacklisted;
        }
        if (seconds <= 0) {
            return BlacklistOutcome::NotBlacklisted;
        }
        const auto key = std::string{kUserIdleSessionPrefix} + std::string{user_id};
        if (!m_context->cache().get(key).hit) {
            return BlacklistOutcome::IdleExpired;
        }
        m_context->cache().put(
            key, core::cache::to_cache_value(now_seconds()),
            core::cache::Ttl{}.seconds(seconds));
        return BlacklistOutcome::NotBlacklisted;
    }

    /** @brief blacklist.rs:151-166 BlackList for AuthToken. */
    [[nodiscard]] BlacklistOutcome check_in_blacklist(const AuthToken& token) const
    {
        const bool blacklisted =
            check_user_in_blacklist(token.user_id, token.exp) ||
            check_role_in_blacklist(token.role_id, token.exp);
        if (blacklisted) {
            return BlacklistOutcome::Blacklisted;
        }
        return check_idle_session(token.user_id);
    }

    // ── authorization::check_tenant / get_role_info ───────────────────────────

    /** @brief authorization.rs:130-144 check_tenant. */
    [[nodiscard]] bool check_tenant(const std::optional<std::string>& token_tenant_id) const
    {
        if (!token_tenant_id.has_value()) {
            return true;
        }
        const auto configured = m_context->config_store().get_value("tenant", "tenant_id");
        if (!configured || configured->empty()) {
            return true;
        }
        return *token_tenant_id == *configured;
    }

    /** @brief authorization.rs:19-56 get_role_info (predefined → cache → DB). */
    [[nodiscard]] std::optional<RoleInfo> get_role_info(const AuthToken& token) const
    {
        if (token.role_id.empty()) {
            return std::nullopt;
        }
        // authorization.rs:23-25 — PREDEFINED_ROLES.get(role_id).
        if (const auto predefined = predefined_role_grants(token.role_id)) {
            RoleInfo info{};
            info.role_id = token.role_id;
            info.grants = *predefined;
            return info;
        }
        // authorization.rs:27-32 — role info cache (CR_INFO_<role_id>).
        const auto cache_key = std::string{kRoleInfoCachePrefix} + token.role_id;
        const auto cached = m_context->cache().get(cache_key);
        if (cached.hit) {
            RoleInfo info{};
            info.role_id = token.role_id;
            info.grants = parse_grants(cached.value_or<std::string>({}));
            return info;
        }
        // authorization.rs:34-43 — find_by_role_id_org_id_tenant_id -> InvalidJwtToken.
        const auto record = m_context->repository().find_role_by_string(token.role_id);
        if (!record) {
            return std::nullopt;
        }
        RoleInfo info{};
        info.role_id = token.role_id;
        const auto config_key = std::string{"role_permissions."} + token.role_id;
        const auto configured = m_context->config_store().get_value("offer_engine", config_key);
        if (configured && !configured->empty()) {
            info.grants = parse_grants(*configured);
        }
        // authorization.rs:45-54 — cache with TTL = exp - now when positive.
        if (token.exp > 0) {
            const auto now = now_seconds();
            if (token.exp > now) {
                m_context->cache().put(
                    cache_key,
                    core::cache::to_cache_value(join_grants(info.grants)),
                    core::cache::Ttl{}.seconds(token.exp - now));
            }
        }
        return info;
    }

    /** @brief Local PREDEFINED_ROLES table (predefined_roles.rs:8+): the Offers
     *  grants each predefined role carries via its permission groups. */
    [[nodiscard]] std::optional<std::vector<std::string>> predefined_role_grants(
        std::string_view role_id) const noexcept
    {
        constexpr std::string_view kReadWrite[] = {
            "internal_admin", "internal_operator", "tenant_admin", "org_admin",
            "merchant_admin", "merchant_developer", "merchant_iam_admin",
            "merchant_operator", "profile_admin", "profile_developer",
            "profile_iam_admin", "profile_operator",
        };
        constexpr std::string_view kViewOnly[] = {
            "internal_view_only", "merchant_view_only", "profile_view_only",
            "merchant_customer_support", "profile_customer_support",
        };
        for (const auto id : kReadWrite) {
            if (role_id == id) {
                return std::vector<std::string>{"offers_read", "offers_write"};
            }
        }
        for (const auto id : kViewOnly) {
            if (role_id == id) {
                return std::vector<std::string>{"offers_read"};
            }
        }
        return std::nullopt;
    }

    // ── offer_engine_permissions ──────────────────────────────────────────────

    /** @brief core/external_service_auth.rs:189-196 offer_engine_permissions.
     *  Permission::iter().filter(resource() == Resource::Offers)
     *    .filter(check_permission_exists).map(to_string()).collect(). */
    [[nodiscard]] std::vector<std::string>
    offer_engine_permissions(const RoleInfo& role_info) const noexcept
    {
        std::vector<std::string> permissions{};
        for (const auto permission : {OffersPermission::Read, OffersPermission::Write}) {
            if (role_info.check_permission_exists(permission)) {
                permissions.emplace_back(permission_name(permission));
            }
        }
        return permissions;
    }

    // ── resolve_offer_engine_merchant_id ──────────────────────────────────────

    /** @brief Dimension chain from AuthToken (core/external_service_auth.rs:145-148). */
    [[nodiscard]] std::string dimensions(const AuthToken& payload) const
    {
        std::string scope{};
        if (!payload.merchant_id.empty()) {
            scope += "merchant:" + payload.merchant_id;
        }
        if (!payload.org_id.empty()) {
            if (!scope.empty()) {
                scope += '/';
            }
            scope += "org:" + payload.org_id;
        }
        if (!payload.profile_id.empty()) {
            if (!scope.empty()) {
                scope += '/';
            }
            scope += "profile:" + payload.profile_id;
        }
        return scope;
    }

    /** @brief Superposition-backed config read with namespace/scoped/base fallback. */
    [[nodiscard]] std::optional<std::string> config_value(
        std::string_view scope, std::string_view key) const
    {
        constexpr std::string_view kNamespace{"offer_engine"};
        if (const auto resolved = m_context->superposition().resolve(kNamespace, key, scope)) {
            return resolved->get_value();
        }
        const std::string scoped_key{std::string{key} + "@" + std::string{scope}};
        if (const auto scoped = m_context->config_store().get_value(kNamespace, scoped_key)) {
            return scoped;
        }
        return m_context->config_store().get_value(kNamespace, key);
    }

    /** @brief core/offer_engine/config.rs:12-47 resolve_offer_engine_credential_source. */
    [[nodiscard]] OfferEngineCredentialSource
    resolve_offer_engine_credential_source(const AuthToken& payload) const
    {
        const auto scope = dimensions(payload);
        const auto enabled = config_value(scope, "enabled");
        if (!enabled || *enabled != "true") {
            return OfferEngineCredentialSource::None;
        }
        const auto raw = config_value(scope, "credential_source");
        if (raw && *raw == "application") {
            return OfferEngineCredentialSource::Application;
        }
        if (raw && *raw == "merchant") {
            return OfferEngineCredentialSource::Merchant;
        }
        return OfferEngineCredentialSource::None;
    }

    /** @brief core/offer_engine/config.rs:75-108 resolve_application_offer_config.
     *  Same base keys as the offer_engine module (offer_engine.cppm:652-670). */
    [[nodiscard]] std::optional<std::string> resolve_application_offer_config() const
    {
        const auto api_key = config_value("", "api_key");
        if (!api_key || api_key->empty()) {
            return std::nullopt;
        }
        const auto merchant_id = config_value("", "merchant_id");
        if (!merchant_id || merchant_id->empty()) {
            return std::nullopt;
        }
        return *merchant_id;
    }

    /** @brief core/offer_engine/config.rs:111-147 resolve_merchant_offer_config.
     *
     *  Upstream reads merchant_account.get_offer_engine_config(). The shared
     *  offer_engine module persists that same JSON under ConfigStore namespace
     *  `offer_engine_merchant_config`, keyed by merchant id
     *  (offer_engine.cppm:673-705). Reading it here keeps both routes on one
     *  local persistence contract. */
    [[nodiscard]] std::optional<std::string>
    resolve_merchant_offer_config(const AuthToken& payload) const
    {
        if (payload.merchant_id.empty()) {
            return std::nullopt;
        }
        // core/external_service_auth.rs:164-175 — merchant key store + account lookup.
        // Local accounts_store is AppRepository; master key resolution is represented by
        // the merchant-key record when present.
        (void)m_context->key_store().get("merchant_key", payload.merchant_id);
        const auto account = m_context->repository().find_merchant_account_by_string(
            payload.merchant_id
        );
        if (!account) {
            return std::nullopt;
        }
        // resolve_merchant_offer_config also requires the application base_url.
        const auto base_url = config_value("", "base_url");
        if (!base_url || base_url->empty()) {
            return std::nullopt;
        }
        const auto raw = m_context->config_store().get_value(
            "offer_engine_merchant_config", payload.merchant_id);
        if (!raw || raw->empty()) {
            return std::nullopt;
        }
        auto generic = serde::Ser::decode_generic("application/json", *raw);
        if (!generic) {
            return std::nullopt;
        }
        const auto* object = std::get_if<interfaces::Value::Object>(&generic->get());
        if (object == nullptr) {
            return std::nullopt;
        }
        std::string api_key{};
        std::string merchant_id{};
        const auto read = [object](std::string_view key, std::string& target)
        {
            const auto field = object->get(std::string{key});
            if (field) {
                if (const auto* text = std::get_if<std::string>(&field.value().get())) {
                    target = *text;
                }
            }
        };
        read("api_key", api_key);
        read("merchant_id", merchant_id);
        if (api_key.empty() || merchant_id.empty()) {
            return std::nullopt;
        }
        return merchant_id;
    }

    /** @brief core/external_service_auth.rs:139-187 resolve_offer_engine_merchant_id. */
    [[nodiscard]] std::optional<std::string>
    resolve_offer_engine_merchant_id(const AuthToken& payload) const
    {
        const auto source = resolve_offer_engine_credential_source(payload);
        switch (source) {
        case OfferEngineCredentialSource::None:
            return std::nullopt;  // AccessForbidden { resource: "offer_engine" }
        case OfferEngineCredentialSource::Application:
            return resolve_application_offer_config();
        case OfferEngineCredentialSource::Merchant:
            return resolve_merchant_offer_config(payload);
        }
        return std::nullopt;
    }

    // ── small utilities ───────────────────────────────────────────────────────

    [[nodiscard]] std::int64_t now_seconds() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    [[nodiscard]] static std::vector<std::string> parse_grants(std::string_view value)
    {
        std::vector<std::string> grants{};
        while (!value.empty()) {
            const auto comma = value.find(',');
            std::string grant{value.substr(0, comma)};
            const auto first = grant.find_first_not_of(" \t");
            if (first != std::string::npos) {
                grant.erase(0, first);
            }
            while (!grant.empty() && (grant.back() == ' ' || grant.back() == '\t')) {
                grant.pop_back();
            }
            if (!grant.empty()) {
                grants.push_back(std::move(grant));
            }
            if (comma == std::string_view::npos) {
                break;
            }
            value.remove_prefix(comma + 1);
        }
        return grants;
    }

    [[nodiscard]] static std::string join_grants(const std::vector<std::string>& grants)
    {
        std::string out{};
        for (const auto& grant : grants) {
            if (!out.empty()) {
                out += ',';
            }
            out += grant;
        }
        return out;
    }

    [[nodiscard]] static std::string json_escape(std::string_view value)
    {
        std::string out{};
        out.reserve(value.size());
        for (const char ch : value) {
            switch (ch) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(ch); break;
            }
        }
        return out;
    }

    /** @brief Exact ApiErrorResponse wire shape (api_models/errors/types.rs:60-70,159-176). */
    void send_api_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view type,
        std::string_view code,
        std::string_view message,
        interfaces::io::types::Status status
    ) const
    {
        const auto body = std::format(
            R"({{"type":"{}","message":"{}","code":"{}"}})",
            json_escape(type), json_escape(message), json_escape(code));
        auto bytes = serde::Ser::serialize_raw("application/json", body);
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::external_service_auth
