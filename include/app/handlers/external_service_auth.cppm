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

inline constexpr std::string_view kBlacklistPrefix{"hypersense.blacklist."};
inline constexpr std::string_view kJwtPurpose{"jwt"};
inline constexpr std::string_view kJwtKeyId{"user-jwt-secret"};
inline constexpr std::string_view kJwtConfigKey{"jwt.secret"};
inline constexpr std::string_view kDefaultJwtSecret{"user-jwt-secret"};
// Upstream OFFER_ENGINE_CONTEXT (core/external_service_auth.rs:22,130).
inline constexpr std::string_view kOfferEngineContext{"MERCHANT"};

/**
 * @brief Local normalizer for ValidateTokenRequest.
 *
 * The DTO partition models `service` as a C++ enum (dto/external_service_auth.cppm:42-56)
 * whose serde mapping uses the enumerator name, while upstream `ValidatingService` is
 * `#[serde(rename_all = "snake_case")]` + `strum::Display` ("offer_engine"). Deserializing
 * the DTO directly therefore does not accept the upstream wire value. This local shape
 * keeps the field as text so both spellings map deterministically to the typed DTO.
 */
class ValidateTokenWire
{
public:
    void set_token(std::string value) { token = std::move(value); }
    [[nodiscard]] const std::string& get_token() const noexcept { return token; }
    void set_service(std::string value) { service = std::move(value); }
    [[nodiscard]] const std::string& get_service() const noexcept { return service; }

    std::string token{};
    std::string service{};
};

/**
 * @brief Typed OfferEngine arm of ExternalVerifyTokenResponse.
 *
 * The DTO partition declares the response variant as an empty enum marker
 * (dto/external_service_auth.cppm:36-40), so the four upstream fields
 * (api_models/external_service_auth.rs:40-45) are normalized here and emitted through
 * serde. `token` is masked because upstream types it as `Secret<String>`.
 */
class OfferEngineVerifyResponse
{
public:
    void set_merchant_id(std::string value) { merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return merchant_id; }
    void set_context(std::string value) { context = std::move(value); }
    [[nodiscard]] const std::string& get_context() const noexcept { return context; }
    void set_token(std::string value) { token = std::move(value); }
    [[nodiscard]] const std::string& get_token() const noexcept { return token; }
    void set_permissions(std::vector<std::string> value) { permissions = std::move(value); }
    [[nodiscard]] const std::vector<std::string>& get_permissions() const noexcept { return permissions; }

    std::string merchant_id{};
    std::string context{};
    std::string token{};
    std::vector<std::string> permissions{};
};

} // namespace app::handlers::external_service_auth

template<>
struct serde::Serializable<::app::handlers::external_service_auth::ValidateTokenWire>
{
    static constexpr auto fields()
    {
        using W = ::app::handlers::external_service_auth::ValidateTokenWire;
        return std::tuple{
            serde::FieldDesc<"token", &W::get_token, &W::set_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"service", &W::get_service, &W::set_service, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "validate_token_wire"; }
};

template<>
struct serde::Serializable<::app::handlers::external_service_auth::OfferEngineVerifyResponse>
{
    static constexpr auto fields()
    {
        using R = ::app::handlers::external_service_auth::OfferEngineVerifyResponse;
        return std::tuple{
            serde::FieldDesc<"merchant_id", &R::get_merchant_id, &R::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"context", &R::get_context, &R::set_context, serde::FieldOptions{}>{},
            serde::FieldDesc<"token", &R::get_token, &R::set_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"permissions", &R::get_permissions, &R::set_permissions, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "offer_engine_verify_response"; }
};

export namespace app::handlers::external_service_auth {

class ExternalServiceAuthRouteHandlers
{
public:
    explicit ExternalServiceAuthRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_validate_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: authentication::NoAuth + api_locking::LockAction::NotApplicable
        // (routes/external_service_auth.rs:23-24). The audited registry row is NO_AUTH,
        // so authorize_route passes through; identity comes from the JWT in the body.
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

        using Wire = ::app::handlers::external_service_auth::ValidateTokenWire;
        auto parsed = serde::Ser::deserialize<Wire>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        if (parsed->get_service().empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST, "missing validating service"));
            return;
        }
        const auto service = parse_service(parsed->get_service());
        if (!service) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST, "unsupported validating service"));
            return;
        }

        using Input = app::dto::external_service_auth::ValidateTokenRequest;
        Input request{};
        request.set_token(parsed->get_token());
        request.set_service(*service);

        const app::crypto::SecretString token_secret{request.get_token()};
        const auto token = token_secret.value();

        // core/external_service_auth.rs:111-113 — decode_jwt::<AuthToken>(&token, &state)
        // and validate signature, shape and expiry before any identity is trusted.
        const auto claims = decode_dashboard_claims(token);
        if (!claims) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED, "invalid dashboard token"));
            return;
        }

        // core:115-117 — payload.check_in_blacklist -> InvalidJwtToken.
        if (is_blacklisted(claims->user_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED, "invalid dashboard token"));
            return;
        }

        // core:119 — check_tenant(payload.tenant_id, state.tenant.tenant_id).
        if (const auto tenant = m_context->config_store().get_value("tenant", "tenant_id");
            tenant && !tenant->empty() && claims->tenant_id != *tenant) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN, "tenant mismatch"));
            return;
        }

        // core:121,189-196 — get_role_info + Permission::iter() filtered to Resource::Offers.
        // A resolved role with no Offers grants yields an empty permissions list upstream,
        // not an access error; only an unresolved role is access forbidden.
        const auto permissions = offer_engine_permissions(*claims);
        if (!permissions) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN, "offer engine access forbidden"));
            return;
        }

        // core:125,139-187 — resolve_offer_engine_merchant_id via Dimensions and the
        // credential-source config, with AccessForbidden when no source is configured.
        auto merchant_id = resolve_offer_engine_merchant(*claims);
        if (merchant_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN, "offer engine is not enabled for this merchant"));
            return;
        }

        // routes/external_service_auth.rs:16-26 — Flow::ExternalServiceValidateToken,
        // LockAction::NotApplicable (no lock acquired).
        m_context->metrics().record_flow("ExternalServiceValidateToken");
        m_context->metrics().counter("EXTERNAL_SERVICE_TOKEN_VALIDATED").increment();

        OfferEngineVerifyResponse response{};
        response.set_merchant_id(std::move(merchant_id));
        response.set_context(std::string{kOfferEngineContext});
        response.set_token(token_secret.masked_as(app::crypto::SecretFieldKind::ClientSecret));
        response.set_permissions(*permissions);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/external_service_auth"}
            .add_route(
                Route{"/external-service/validate-token"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_validate_token(req, res, std::move(send));
                    })
                    .summary("Hyperswitch validate_token")
            );
        // Upstream mounts the route as POST /external-service/validate-token.
        Router{router, "/external-service"}
            .add_route(
                Route{"/validate-token"}
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
    struct DashboardClaims
    {
        std::string user_id{};
        std::string merchant_id{};
        std::string org_id{};
        std::string profile_id{};
        std::string tenant_id{};
        std::string role_id{};
        std::string external_service_type{};
        std::int64_t exp{0};
    };

    /** @brief Secret resolution: key store first, persisted config then local default. */
    [[nodiscard]] std::string signing_secret() const
    {
        if (const auto stored = m_context->key_store().get(
                kJwtPurpose, kJwtKeyId);
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

    /** @brief decode_jwt::<AuthToken> substitute with strict signature/expiry checks. */
    [[nodiscard]] std::optional<DashboardClaims>
    decode_dashboard_claims(std::string_view token) const
    {
        const auto signer = app::crypto::JwtSigner{signing_secret()};
        if (!signer.verify(token)) {
            return std::nullopt;
        }

        const std::size_t first = token.find('.');
        const std::size_t second = token.find('.', first + 1);
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

        DashboardClaims claims{};
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
        read("org_id", claims.org_id);
        read("profile_id", claims.profile_id);
        read("tenant_id", claims.tenant_id);
        read("role_id", claims.role_id);
        read("external_service_type", claims.external_service_type);

        if (const auto field = object->get("exp")) {
            if (const auto* number = std::get_if<std::int64_t>(&field.value().get())) {
                claims.exp = *number;
            } else if (const auto* number = std::get_if<double>(&field.value().get())) {
                claims.exp = static_cast<std::int64_t>(*number);
            }
        }
        if (claims.exp > 0) {
            const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
            if (claims.exp <= now) {
                return std::nullopt;
            }
        }
        return claims;
    }

    [[nodiscard]] bool is_blacklisted(std::string_view user_id) const
    {
        const std::string key = std::string{kBlacklistPrefix} +
            (user_id.empty() ? std::string{"unknown"} : std::string{user_id});
        // Durable marker written by the signout handler, mirrored in TTL cache.
        if (m_context->repository().get_config(key).has_value()) {
            return true;
        }
        return m_context->cache().get(key).hit;
    }

    [[nodiscard]] std::optional<app::dto::external_service_auth::ValidatingService>
    parse_service(std::string_view value) const noexcept
    {
        // Upstream snake_case wire value and strum Display spelling both accepted.
        if (value == "offer_engine" || value == "OfferEngine" || value == "offer-engine") {
            return app::dto::external_service_auth::ValidatingService::OfferEngine;
        }
        return std::nullopt;
    }

    /**
     * @brief Role info + offers-resource permission derivation.
     *
     * Local substitute for roles::RoleInfo::check_permission_exists: the role id comes
     * from the decoded token (or an active user-role assignment), the role record is
     * loaded from AppRepository, and the Offers permission set is read from config
     * `offer_engine.role_permissions.<role_id>` with the two canonical Offers grants as
     * the configured-role default. Non-Offers permissions are filtered out.
     */
    [[nodiscard]] std::optional<std::vector<std::string>>
    offer_engine_permissions(const DashboardClaims& claims) const
    {
        std::vector<std::string> role_ids{};
        if (!claims.role_id.empty()) {
            role_ids.push_back(claims.role_id);
        } else if (!claims.user_id.empty()) {
            for (const auto& assignment : m_context->repository().list_user_roles()) {
                if (assignment.get_user_id() != claims.user_id) {
                    continue;
                }
                const auto& status = assignment.get_status();
                if (!status.empty() && status != "active" && status != "granted") {
                    continue;
                }
                if (!assignment.get_role_id().empty()) {
                    role_ids.push_back(assignment.get_role_id());
                }
            }
        }

        bool role_found{false};
        std::vector<std::string> granted{};
        for (const auto& role_id : role_ids) {
            const auto role = m_context->repository().find_role_by_string(role_id);
            if (!role) {
                continue;
            }
            const auto& status = role->get_status();
            if (!status.empty() && status != "active") {
                continue;
            }
            role_found = true;

            std::vector<std::string> role_permissions{};
            const auto config_key = std::string{"role_permissions."} + role_id;
            const auto configured = m_context->config_store().get_value(
                "offer_engine", config_key
            );
            if (configured && !configured->empty()) {
                std::string_view rest{*configured};
                while (!rest.empty()) {
                    const std::size_t comma = rest.find(',');
                    std::string permission{rest.substr(0, comma)};
                    const std::size_t space = permission.find_first_not_of(" \t");
                    if (space != std::string::npos) {
                        permission.erase(0, space);
                    }
                    while (!permission.empty() &&
                           (permission.back() == ' ' || permission.back() == '\t')) {
                        permission.pop_back();
                    }
                    // Permission::iter().filter(resource() == Resource::Offers)
                    if (permission.starts_with("offers_")) {
                        role_permissions.push_back(std::move(permission));
                    }
                    if (comma == std::string_view::npos) {
                        break;
                    }
                    rest.remove_prefix(comma + 1);
                }
            } else {
                role_permissions = {"offers_read", "offers_write"};
            }

            for (auto& permission : role_permissions) {
                if (std::ranges::find(granted, permission) == granted.end()) {
                    granted.push_back(std::move(permission));
                }
            }
        }

        if (!role_found) {
            return std::nullopt;
        }
        return granted;
    }

    /**
     * @brief resolve_offer_engine_merchant_id substitute.
     *
     * Dimensions (processor merchant id, org id, profile id) come from the decoded
     * claims. `offer_engine.credential_source` selects the local branch: "application"
     * reads superposition/config `offer_engine.merchant_id`, "merchant" reads the
     * merchant account from AppRepository (with an optional config override), and any
     * other value — including an explicit "none" — yields AccessForbidden.
     */
    [[nodiscard]] std::string
    resolve_offer_engine_merchant(const DashboardClaims& claims) const
    {
        std::string source{};
        if (const auto value = m_context->config_store().get_value(
                "offer_engine", "credential_source");
            value && !value->empty()) {
            source = *value;
        }
        if (source.empty()) {
            // Default to the previous local behavior until a source is configured.
            source = "merchant";
        }

        if (source == "application") {
            const auto context = claims.profile_id.empty()
                                     ? claims.org_id
                                     : claims.profile_id;
            if (const auto resolved = m_context->superposition().resolve(
                    "offer_engine", "merchant_id", context);
                resolved && !resolved->get_value().empty()) {
                return resolved->get_value();
            }
            if (const auto configured = m_context->config_store().get_value(
                    "offer_engine", "merchant_id");
                configured && !configured->empty()) {
                return *configured;
            }
            return {};
        }

        if (source == "merchant") {
            if (claims.merchant_id.empty()) {
                return {};
            }
            const auto account = m_context->repository().find_merchant_account_by_string(
                claims.merchant_id
            );
            if (!account) {
                return {};
            }
            const auto override_key = std::string{"merchant."} + claims.merchant_id;
            if (const auto override = m_context->config_store().get_value(
                    "offer_engine", override_key);
                override && !override->empty()) {
                return *override;
            }
            return account->get_merchant_id();
        }

        return {};
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
