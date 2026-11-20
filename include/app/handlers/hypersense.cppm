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

export namespace app::handlers::hypersense {

inline constexpr std::string_view kBlacklistConfigPrefix{"hypersense.blacklist."};
inline constexpr std::string_view kSigningSecretConfigKey{"hypersense.jwt_secret"};
inline constexpr std::string_view kDefaultSigningSecret{"hypersense-jwt-secret"};
// Upstream JWT_TOKEN_TIME_IN_SECS (consts.rs:64): 60 * 60 * 24 * 2.
inline constexpr std::int64_t kExternalTokenTtlSeconds{172800};

/**
 * @brief Local serializable shape for ExternalVerifyTokenResponse::Hypersense.
 *
 * The DTO partition declares the response variant as an empty enum marker
 * (dto/external_service_auth.cppm:36-40), so the four upstream payload fields
 * (core/external_service_auth.rs:95-102) are normalized here and emitted through
 * serde instead of a hand-written JSON string.
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

/** @brief Typed stand-in for ExternalToken::check_service_type (no raw string compare in the handler). */
class ServiceTypeValidator
{
public:
    [[nodiscard]] bool accepts_hypersense(std::string_view value) const noexcept
    {
        return parse(value) ==
               app::dto::external_service_auth::ExternalVerifyTokenResponse::Hypersense;
    }

private:
    using Service =
        app::dto::external_service_auth::ExternalVerifyTokenResponse;

    [[nodiscard]] std::optional<Service> parse(std::string_view value) const noexcept
    {
        if (value == "Hypersense") {
            return Service::Hypersense;
        }
        if (value == "OfferEngine") {
            return Service::OfferEngine;
        }
        return std::nullopt;
    }
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
        // Upstream: DashboardNoPermissionAuth { allow_connected: false, allow_platform: false }
        // (hypersense.rs:28-31). The audited registry row is the single source of truth;
        // its JWT + ADMIN rule is the local equivalent (bearer required, no permission
        // check) and the allow flags are inert for the JWT scheme.
        const auto rule = m_context->auth_rules().rule_for("get_hypersense_token");
        const auto auth = m_context->authorizer().authorize_route(
            app::auth::AuthContextBuilder{}(req), rule
        );
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        // generate_external_token: bind the external token to the calling dashboard
        // identity. user_id comes from the caller's token, merchant_id from the
        // merchant header (with the auth context as a fallback) — never from body size.
        const auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        const auto bearer = app::auth::TokenParser{}(req).value();
        auto user_id = std::string{bearer};
        if (const auto sub = auth_ctx.claim("sub"); sub && !sub->empty()) {
            user_id = std::string{*sub};
        }
        auto merchant_id = app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) {
            merchant_id = auth_ctx.merchant_id();
        }
        if (merchant_id.empty()) {
            merchant_id = "merchant_"
                + app::crypto::KeyedHasher{"hypersense.merchant"}.hash(bearer);
        }

        // ExternalToken payload: { user_id, merchant_id, exp, external_service_type }.
        // Signed with the config-driven secret (state.conf substitute); jti is a real
        // generated id. No fabricated name/email is cached here — the authoritative
        // identity is read from the user store at verify time.
        const auto now = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now()
        );
        const auto exp = now + kExternalTokenTtlSeconds;
        const auto token_payload = std::format(
            R"({{"user_id":{},"merchant_id":{},"exp":{},"external_service_type":"Hypersense","jti":{}}})",
            serde::Ser::encode_json(user_id),
            serde::Ser::encode_json(merchant_id),
            exp,
            serde::Ser::encode_json(m_context->ids().uuid_v4())
        );
        const auto token = app::crypto::JwtSigner{signing_secret()}.sign(token_payload);
        if (token.empty()) {
            send_error(
                res,
                send,
                core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INTERNAL_ERROR,
                    "external token signing failed"
                )
            );
            return;
        }

        // Upstream flow: Flow::HypersenseTokenRequest, api_locking::LockAction::NotApplicable
        // (hypersense.rs:15,32) — no lock is acquired, only flow/counter instrumentation.
        m_context->metrics().record_flow("HypersenseTokenRequest");
        m_context->metrics().counter("hypersense.token_request").increment();

        using Response = app::dto::external_service_auth::ExternalTokenResponse;
        Response response{};
        response.set_token(token);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_verify_hypersense_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: NoAuth — the payload token is the credential.
        const auto rule = m_context->auth_rules().rule_for("verify_hypersense_token");
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
        using Input = app::dto::external_service_auth::ExternalVerifyTokenRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // verify_external_token: decode + signature-check the JWT, reject blacklisted
        // users and wrong service types, then resolve the identity it carries.
        const auto claims = decode_token(parsed->get_token());
        if (!claims) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED, "invalid external token"));
            return;
        }
        if (is_blacklisted(claims->user_id, claims->exp)) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED, "invalid external token"));
            return;
        }
        if (!ServiceTypeValidator{}.accepts_hypersense(claims->external_service_type)) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN, "invalid external service type"));
            return;
        }

        // find_active_user_by_user_id (external_service_auth.rs:85-100): name and email
        // are authoritative from the persisted user record, not fabricated or cached.
        const auto user = m_context->repository().find_user_by_string(claims->user_id);
        if (!user || user->get_status() != "active") {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INTERNAL_ERROR, "user not found for external token"));
            return;
        }

        // Upstream flow: Flow::HypersenseVerifyToken, LockAction::NotApplicable.
        m_context->metrics().record_flow("HypersenseVerifyToken");
        m_context->metrics().counter("hypersense.verify_token").increment();

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
        // Upstream: NoAuth — decode the JWT from the payload, then blacklist its user.
        const auto rule = m_context->auth_rules().rule_for("signout_hypersense_token");
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
        using Input = app::dto::external_service_auth::ExternalSignoutTokenRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto claims = decode_token(parsed->get_token());
        if (!claims) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED, "invalid external token"));
            return;
        }

        // insert_user_in_blacklist (external_service_auth.rs:55-61): persist the
        // blacklist timestamp so it survives a restart. The cache entry is a short
        // TTL mirror (an explicitly allowed cache category) that keeps other local
        // handlers observing the same blacklist marker.
        const auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        const auto key = std::string{kBlacklistConfigPrefix} + claims->user_id;
        m_context->repository().set_config(key, std::to_string(now));
        m_context->cache().put(
            key,
            core::cache::to_cache_value(now),
            core::cache::Ttl{}.seconds(kExternalTokenTtlSeconds)
        );

        // Upstream flow: Flow::HypersenseSignoutToken, LockAction::NotApplicable.
        m_context->metrics().record_flow("HypersenseSignoutToken");
        m_context->metrics().counter("hypersense.signout_token").increment();

        // ApplicationResponse::StatusOk is bodyless (external_service_auth.rs:63).
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
    struct ExternalClaims
    {
        std::string user_id{};
        std::string merchant_id{};
        std::string external_service_type{};
        std::int64_t exp{0};
    };

    /** @brief state.conf signing-secret substitute, resolved from persisted config. */
    [[nodiscard]] std::string signing_secret() const
    {
        if (const auto configured = m_context->repository().get_config(
                std::string{kSigningSecretConfigKey});
            configured && !configured->empty()) {
            return *configured;
        }
        return std::string{kDefaultSigningSecret};
    }

    /** @brief check_user_in_blacklist: blacklisted_at > (exp - JWT_TOKEN_TIME_IN_SECS). */
    [[nodiscard]] bool is_blacklisted(std::string_view user_id, std::int64_t exp) const
    {
        if (exp <= 0) {
            return false;
        }
        const auto key = std::string{kBlacklistConfigPrefix} + std::string{user_id};
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
        return blacklisted_at > (exp - kExternalTokenTtlSeconds);
    }

    [[nodiscard]] std::optional<ExternalClaims> decode_token(std::string_view token) const
    {
        const auto signer = app::crypto::JwtSigner{signing_secret()};
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
        if (!generic) {
            return std::nullopt;
        }
        const auto* object = std::get_if<interfaces::Value::Object>(&generic->get());
        if (object == nullptr) {
            return std::nullopt;
        }

        ExternalClaims claims{};
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
        read("user_id", claims.user_id);
        read("merchant_id", claims.merchant_id);
        read("external_service_type", claims.external_service_type);

        if (const auto field = object->get("exp")) {
            if (const auto* number = std::get_if<std::int64_t>(&field.value().get())) {
                claims.exp = *number;
            } else if (const auto* number = std::get_if<double>(&field.value().get())) {
                claims.exp = static_cast<std::int64_t>(*number);
            }
        }

        if (claims.user_id.empty() || claims.exp <= 0) {
            return std::nullopt;
        }
        return claims;
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
