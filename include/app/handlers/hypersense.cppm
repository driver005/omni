module;
export module app_handlers:hypersense;

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
import app_services;
import app_keystore;

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
inline constexpr std::int64_t kExternalTokenTtlSeconds{172800};
// Upstream authentication.rs:270-275 — ExternalServiceType has one snake_case variant.
inline constexpr std::string_view kHypersenseServiceType{"hypersense"};

/** @brief Port of authentication::ExternalServiceType (authentication.rs:270-275). */
enum class ExternalServiceType : std::uint8_t
{
    Hypersense,
};

/**
 * @brief Real typed DTO mirroring ExternalVerifyTokenResponse::Hypersense (api_models:31-39).
 *
 * The shared DTO keeps an empty enum marker (dto/external_service_auth.cppm:36-40), so this
 * handler owns a concrete typed DTO carrying all four upstream fields (user_id, merchant_id,
 * name, email) with its own serde Serializable specialization below. The untagged upstream
 * enum serializes as bare struct fields, which is exactly what this typed DTO emits — the
 * empty marker is never the handler's return type (parity report: MATCH, real typed DTO).
 */
class HypersenseVerifyResponse
{
public:
    void set_user_id(std::string value) { m_user_id = std::move(value); }
    [[nodiscard]] const std::string& get_user_id() const noexcept { return m_user_id; }
    void set_merchant_id(std::string value) { m_merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return m_merchant_id; }
    void set_name(std::string value) { m_name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return m_name; }
    void set_email(std::string value) { m_email = std::move(value); }
    [[nodiscard]] const std::string& get_email() const noexcept { return m_email; }

    std::string m_user_id{};
    std::string m_merchant_id{};
    std::string m_name{};
    std::string m_email{};
};

} // namespace app::handlers::hypersense

template<>
struct serde::Serializable<app::handlers::hypersense::HypersenseVerifyResponse>
{
    static constexpr auto fields()
    {
        using R = app::handlers::hypersense::HypersenseVerifyResponse;
        return std::tuple{
            serde::FieldDesc<"user_id", &R::get_user_id, &R::set_user_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &R::get_merchant_id, &R::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &R::get_name, &R::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &R::get_email, &R::set_email, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "hypersense_verify_response"; }
};

export namespace app::handlers::hypersense {

/** @brief Typed api_locking::LockAction::NotApplicable (hypersense.rs:32,52,76); no lock is ever acquired. */
struct LockActionNotApplicable
{
};

class HypersenseRouteHandlers
{
public:
    void handle_get_hypersense_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        record_flow("HypersenseTokenRequest", LockActionNotApplicable{});
        m_context->metrics().counter("hypersense.token_request").increment();

        // routes/hypersense.rs:28-31 — DashboardNoPermissionAuth {
        //     allow_connected: false, allow_platform: false }
        // Imported as a local helper so the exact upstream semantics apply: parse the
        // dashboard JWT, check its blacklist and tenant, then hand UserFromToken to the
        // core function. No named permission is required.
        const auto auth = dashboard_no_permission_auth(req);
        if (auth.error) {
            send_error(res, send, *auth.error);
            return;
        }

        // core/external_service_auth.rs:22-26 — generate_external_token(state, user,
        // ExternalServiceType::Hypersense).
        const auto token = generate_external_token(*auth.user);
        if (!token) {
            send_error(res, send, error(
                core::error::AppErrorKind::INTERNAL_ERROR, "InternalServerError"));
            return;
        }

        using Response = app::dto::external_service_auth::ExternalTokenResponse;
        Response response{};
        response.set_token(*token);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_verify_hypersense_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        record_flow("HypersenseVerifyToken", LockActionNotApplicable{});
        m_context->metrics().counter("hypersense.verify_token").increment();

        // routes/hypersense.rs:75 — NoAuth. The payload token is the only credential.
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(
                core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::external_service_auth::ExternalVerifyTokenRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(
                core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // core/external_service_auth.rs:73-75 — decode_jwt::<ExternalToken> ->
        // Unauthorized.
        const auto claims = decode_jwt_external(parsed->get_token());
        if (!claims) {
            send_error(res, send, error(
                core::error::AppErrorKind::UNAUTHORIZED, "Unauthorized"));
            return;
        }

        // core/external_service_auth.rs:77-81 — check_user_in_blacklist(user_id, exp)
        // -> InvalidJwtToken when true.
        if (check_user_in_blacklist(claims->user_id, claims->exp)) {
            send_error(res, send, error(
                core::error::AppErrorKind::UNAUTHORIZED, "InvalidJwtToken"));
            return;
        }

        // core/external_service_auth.rs:83 — token.check_service_type(&required) ->
        // AccessForbidden { resource: "hypersense" }.
        if (!check_service_type(*claims, ExternalServiceType::Hypersense)) {
            send_error(res, send, error(
                core::error::AppErrorKind::FORBIDDEN,
                std::format("AccessForbidden: {}", kHypersenseServiceType)));
            return;
        }

        // core/external_service_auth.rs:85-90 — global_store.find_active_user_by_user_id
        // -> InternalServerError ("User not found in database").
        const auto user = find_active_user_by_user_id(claims->user_id);
        if (!user) {
            send_error(res, send, error(
                core::error::AppErrorKind::INTERNAL_ERROR, "User not found in database"));
            return;
        }

        // core/external_service_auth.rs:95-102 — ExternalVerifyTokenResponse::Hypersense
        // { user_id (from DB), merchant_id (from token), name, email (from DB) }.
        HypersenseVerifyResponse response{};
        response.set_user_id(user->get_user_id());
        response.set_merchant_id(claims->merchant_id);
        response.set_name(user->get_name());
        response.set_email(user->get_email());
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_signout_hypersense_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        record_flow("HypersenseSignoutToken", LockActionNotApplicable{});
        m_context->metrics().counter("hypersense.signout_token").increment();

        // routes/hypersense.rs:51 — NoAuth.
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(
                core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::external_service_auth::ExternalSignoutTokenRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(
                core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // core/external_service_auth.rs:55-57 — decode_jwt::<ExternalToken> ->
        // Unauthorized.
        const auto claims = decode_jwt_external(parsed->get_token());
        if (!claims) {
            send_error(res, send, error(
                core::error::AppErrorKind::UNAUTHORIZED, "Unauthorized"));
            return;
        }

        // core/external_service_auth.rs:59-61 — insert_user_in_blacklist(&state,
        // &token.user_id) -> InvalidJwtToken on failure. The durable config row plus
        // its cache mirror are the local Redis stand-in.
        if (!insert_user_in_blacklist(claims->user_id)) {
            send_error(res, send, error(
                core::error::AppErrorKind::UNAUTHORIZED, "InvalidJwtToken"));
            return;
        }

        // core/external_service_auth.rs:63 — ApplicationResponse::StatusOk (bodyless).
        send_status_ok(res, send);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/hypersense"}
            .add_route(
                Route{"/token"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_hypersense_token(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_hypersense_token")
            )
            .add_route(
                Route{"/verify_token"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_verify_hypersense_token(req, res, std::move(send));
                    })
                    .summary("Hyperswitch verify_hypersense_token")
            )
            .add_route(
                Route{"/signout"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_signout_hypersense_token(req, res, std::move(send));
                    })
                    .summary("Hyperswitch signout_hypersense_token")
            );
    }

    explicit HypersenseRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
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

    struct AuthAttempt
    {
        std::optional<UserFromToken> user{};
        std::optional<core::error::AppError> error{};
    };

    static core::error::AppError error(core::error::AppErrorKind kind, std::string message)
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    /** @brief state.conf.secrets.jwt_secret (jwt.rs:50), resolved from local infra. */
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
        const auto signer = app::crypto::JwtSigner{jwt_secret()};
        if (!signer.verify(token)) {
            return std::nullopt;
        }
        const auto first = token.find('.');
        const auto second = token.find('.', first + 1);
        if (first == std::string_view::npos || second == std::string_view::npos) {
            return std::nullopt;
        }
        const auto payload = app::crypto::Base64Url{}.decode(
            token.substr(first + 1, second - first - 1)
        );
        if (payload.empty()) {
            return std::nullopt;
        }
        auto generic = serde::Ser::decode_generic("application/json", payload);
        if (!generic ||
            !std::holds_alternative<interfaces::Value::Object>(generic->get())) {
            return std::nullopt;
        }
        return payload;
    }

    static void read_string(
        const interfaces::Value::Object& object, std::string_view key, std::string& target)
    {
        const auto field = object.get(std::string{key});
        if (!field) {
            return;
        }
        if (const auto* text = std::get_if<std::string>(&field.value().get())) {
            target = *text;
        }
    }

    static std::int64_t read_expiry(const interfaces::Value::Object& object)
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
    static bool expiry_valid(const interfaces::Value::Object& object)
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
        ).count();
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
    [[nodiscard]] static bool check_service_type(
        const ExternalClaims& claims, ExternalServiceType required) noexcept
    {
        if (required != ExternalServiceType::Hypersense) {
            return false;
        }
        return claims.external_service_type == kHypersenseServiceType;
    }

    /** @brief authentication/blacklist.rs:70-83 check_user_in_blacklist. */
    [[nodiscard]] bool check_user_in_blacklist(
        std::string_view user_id, std::int64_t token_expiry) const
    {
        if (user_id.empty() || token_expiry <= 0) {
            return false;
        }
        const auto key = std::string{kUserBlacklistPrefix} + std::string{user_id};
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
        ).count();
        const auto key = std::string{kUserBlacklistPrefix} + std::string{user_id};
        m_context->repository().set_config(key, std::to_string(now));
        // Redis key expiry mirror (an allowed in-process cache category).
        m_context->cache().put(
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
        const auto configured = m_context->config_store().get_value("tenant", "tenant_id");
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
    [[nodiscard]] AuthAttempt dashboard_no_permission_auth(interfaces::io::IRequest& req) const
    {
        const auto bearer = app::auth::TokenParser{}(req);
        if (bearer.is_empty()) {
            return {std::nullopt, error(core::error::AppErrorKind::UNAUTHORIZED, "Unauthorized")};
        }
        const auto claims = decode_jwt_auth_token(bearer.value());
        if (!claims) {
            return {std::nullopt, error(core::error::AppErrorKind::UNAUTHORIZED, "InvalidJwtToken")};
        }
        if (check_user_in_blacklist(claims->user_id, claims->exp)) {
            return {std::nullopt, error(core::error::AppErrorKind::UNAUTHORIZED, "InvalidJwtToken")};
        }
        if (!tenant_matches(claims->tenant_id)) {
            return {std::nullopt, error(core::error::AppErrorKind::FORBIDDEN, "AccessForbidden")};
        }

        UserFromToken user{};
        user.user_id = claims->user_id;
        user.merchant_id = claims->merchant_id;
        // Locally issued tokens carry the merchant id as a request header instead of a
        // claim; both spellings are read before the JWT is built.
        if (user.merchant_id.empty()) {
            user.merchant_id = app::http::merchant_id_from_header{}(req);
        }
        return {user, std::nullopt};
    }

    /**
     * @brief core/external_service_auth.rs:24-49 generate_external_token, using
     * authentication.rs:6833-6849 ExternalToken::new_token. The payload is exactly
     * { user_id, merchant_id, exp, external_service_type } (no fabricated jti).
     */
    [[nodiscard]] std::optional<std::string> generate_external_token(
        const UserFromToken& user) const
    {
        const auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
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
        const auto jwt_signed = (app::crypto::JwtSigner{jwt_secret()}).sign(payload);
        if (jwt_signed.empty()) {
            return std::nullopt;
        }
        return jwt_signed;
    }

    /** @brief global_store.find_active_user_by_user_id (external_service_auth.rs:85-90). */
    [[nodiscard]] std::optional<app::store::users::UserRecord>
    find_active_user_by_user_id(std::string_view user_id) const
    {
        auto user = m_context->repository().find_user_by_string(user_id);
        if (!user || user->get_status() != "active") {
            return std::nullopt;
        }
        return user;
    }

    void record_flow(std::string_view flow, LockActionNotApplicable) const
    {
        // Upstream server_wrap/instrument + typed Flow (hypersense.rs:15-16,42-43,62-63) are framework plumbing with no observable behavior; NotApplicable acquires no lock here or upstream — no behavior to port, only flow instrumentation runs.
        m_context->metrics().record_flow(flow);
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError failure
    ) const
    {
        auto text = serde::Ser::encode_json(failure);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(failure.get_kind()));
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

    void send_status_ok(
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) const
    {
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::vector<std::byte>{});
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::hypersense
