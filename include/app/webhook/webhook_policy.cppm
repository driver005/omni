export module app_webhook:webhook_policy;

import std;
import interfaces;
import core_behavior;
import core_error;
import app_context;
import app_auth;
import app_http;

import :webhooks_input;
import :recovery_webhooks_input;
import :webhook_events_input;
import :verification_input;
import :subscription_input;

/**
 * Per-domain request policies for the webhook area.
 *
 * Each Policy is the thin domain-auth layer that `core::behavior::RequestSupport`
 * wraps. It owns only genuine domain auth/scope rules and delegates the actual
 * check to the existing `XxxInputSupport` methods, so every operation string,
 * error kind, message and rule stays upstream-identical. Path/query helpers
 * that build typed request data stay on InputSupport (they are parsing, not
 * policy) and are intentionally not duplicated here.
 */
export namespace app::webhook::policy {

using ::core::behavior::AuthSpec;
using ::core::behavior::CallContext;
using ::core::behavior::Scope;
using AppError = ::core::error::AppError;

/**
 * Builds the HTTP-free request snapshot handed to runtime methods. Headers, query
 * and path segments are copied once here; the runtime never sees IRequest.
 */
[[nodiscard]] inline CallContext
make_call_context(interfaces::io::IRequest& req, const Scope& scope)
{
    CallContext ctx;
    ctx.merchant_id = scope.merchant_id;
    ctx.profile_id = scope.profile_id;
    ctx.org_id = scope.org_id;
    ctx.connected_merchant_id = scope.connected_merchant_id;

    const auto lower = [](std::string_view value)
    {
        std::string out;
        out.reserve(value.size());
        for (const char ch: value) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return out;
    };

    const ::app::http::PathSegments segments{::app::http::RequestPath{}(req)};
    for (std::size_t index = 0; index < segments.count(); ++index) {
        ctx.path_segments.emplace_back(segments.at(index).value_or(""));
    }

    const auto raw = req.get_path();
    if (const auto question = raw.find('?'); question != std::string_view::npos) {
        auto tail = raw.substr(question + 1);
        while (!tail.empty()) {
            const auto amp = tail.find('&');
            const auto pair = tail.substr(0, amp == std::string_view::npos ? tail.size() : amp);
            if (const auto eq = pair.find('='); eq != std::string_view::npos) {
                ctx.query[std::string{pair.substr(0, eq)}] = std::string{pair.substr(eq + 1)};
            } else if (!pair.empty()) {
                ctx.query[std::string{pair}] = std::string{};
            }
            if (amp == std::string_view::npos) {
                break;
            }
            tail = tail.substr(amp + 1);
        }
    }

    for (const auto& entry: req.get_headers()) {
        std::visit(
            [&](const auto& field)
            {
                if (!field) {
                    return;
                }
                std::string name;
                using Name = std::remove_cvref_t<decltype(field->get_name())>;
                if constexpr (std::same_as<Name, std::string>) {
                    name = field->get_name();
                } else {
                    name = std::string{interfaces::io::types::token_to_string(field->get_name())};
                }
                if (!name.empty()) {
                    ctx.headers[lower(name)] = field->get_value();
                }
            },
            entry
        );
    }

    const auto auth = ::app::auth::AuthContextBuilder{}(req);
    for (const auto& [key, value]: auth.claims()) {
        ctx.claims.emplace(key, value);
    }
    if (const auto secret = auth.claim(::app::auth::kClaimClientSecret)) {
        ctx.client_secret = std::string{*secret};
    }
    return ctx;
}

// ───────────────────────────── webhooks ─────────────────────────────

/** `spec.permission` selects the upstream extractor variant for the route. */
inline constexpr AuthSpec kWebhooksMerchantAuth{.permission = "MERCHANT_ID"};
inline constexpr AuthSpec kWebhooksMerchantProfileAuth{.permission = "MERCHANT_AND_PROFILE_ID"};
inline constexpr AuthSpec kWebhooksNoAuth{.permission = "NO_AUTH"};

class WebhooksPolicy
{
public:
    using InputSupport = ::app::handlers::webhooks::WebhooksInputSupport;

    explicit WebhooksPolicy(InputSupport& input) noexcept :
        m_input{input}
    {
    }

    [[nodiscard]] std::optional<AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view /*operation*/,
        const AuthSpec& spec
    ) const
    {
        using ::app::handlers::webhooks::AuthVariant;

        AuthVariant variant = AuthVariant::MERCHANT_ID;
        if (spec.permission == "MERCHANT_AND_PROFILE_ID") {
            variant = AuthVariant::MERCHANT_AND_PROFILE_ID;
        } else if (spec.permission == "NO_AUTH") {
            variant = AuthVariant::NO_AUTH;
        }

        ::app::handlers::webhooks::PathIds ids{};
        if (variant == AuthVariant::NO_AUTH) {
            // NoAuth route still binds the header merchant into the auth context.
            ids.merchant_id = ::app::http::merchant_id_from_header{}(req);
        } else {
            ids = m_input.path_ids(req, variant == AuthVariant::MERCHANT_AND_PROFILE_ID);
        }
        return m_input.authorize(req, variant, ids);
    }

    [[nodiscard]] Scope scope(interfaces::io::IRequest& req) const
    {
        const auto ids = m_input.path_ids(req, true);
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        Scope result;
        result.merchant_id = ids.merchant_id;
        result.profile_id = ids.profile_id;
        if (const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId)) {
            result.connected_merchant_id = std::string{*connected};
        }
        return result;
    }

    [[nodiscard]] std::optional<AppError> scope_error(const Scope&) const
    {
        return std::nullopt;
    }

    [[nodiscard]] CallContext context(interfaces::io::IRequest& req) const
    {
        auto ctx = make_call_context(req, scope(req));
        if (ctx.merchant_id.empty()) {
            ctx.merchant_id = ::app::http::merchant_id_from_header{}(req);
        }
        return ctx;
    }

private:
    InputSupport& m_input;
};

// ───────────────────────── recovery_webhooks ────────────────────────

class RecoveryWebhooksPolicy
{
public:
    using InputSupport = ::app::handlers::recovery_webhooks::RecoveryWebhooksInputSupport;

    explicit RecoveryWebhooksPolicy(InputSupport& input) noexcept :
        m_input{input}
    {
    }

    [[nodiscard]] std::optional<AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view /*operation*/,
        const AuthSpec& /*spec*/
    ) const
    {
        // The rule is keyed by the operation string in InputSupport; the URL ids
        // are bound before the check, exactly as upstream does.
        return m_input.authorize(req, m_input.recovery_ids(req));
    }

    [[nodiscard]] Scope scope(interfaces::io::IRequest& req) const
    {
        const auto ids = m_input.recovery_ids(req);
        Scope result;
        result.merchant_id = ids.merchant_id;
        result.profile_id = ids.profile_id;
        result.org_id = ::app::http::org_id_from_header{}(req);
        return result;
    }

    [[nodiscard]] std::optional<AppError> scope_error(const Scope&) const
    {
        return std::nullopt;
    }

    [[nodiscard]] CallContext context(interfaces::io::IRequest& req) const
    {
        return make_call_context(req, scope(req));
    }

private:
    InputSupport& m_input;
};

// ─────────────────────────── webhook_events ─────────────────────────

class WebhookEventsPolicy
{
public:
    using InputSupport = ::app::handlers::webhook_events::WebhookEventsInputSupport;

    explicit WebhookEventsPolicy(InputSupport& input) noexcept :
        m_input{input}
    {
    }

    [[nodiscard]] std::optional<AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const AuthSpec& /*spec*/
    ) const
    {
        // The JWT profile-list route is the only one without a route-bound
        // merchant; every other route is JWTAuthMerchantFromRoute.
        if (operation == "list_initial_webhook_delivery_attempts_with_jwtauth") {
            return m_input.authorize(req, operation).error;
        }
        const auto ids = m_input.path_ids(req);
        return m_input.authorize_with_merchant(req, operation, ids.merchant_id).error;
    }

    [[nodiscard]] Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        const auto ids = m_input.path_ids(req);
        Scope result;
        // Route-bound merchant wins, mirroring authorize_with_merchant which
        // overwrites the identity merchant with the path merchant id.
        result.merchant_id =
            ids.merchant_id.empty() ? m_input.merchant_from_identity(auth) : ids.merchant_id;
        result.profile_id = auth.profile_id();
        if (const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId)) {
            result.connected_merchant_id = std::string{*connected};
        }
        return result;
    }

    [[nodiscard]] std::optional<AppError> scope_error(const Scope&) const
    {
        return std::nullopt;
    }

    [[nodiscard]] CallContext context(interfaces::io::IRequest& req) const
    {
        auto ctx = make_call_context(req, scope(req));
        if (ctx.merchant_id.empty()) {
            const auto auth = ::app::auth::AuthContextBuilder{}(req);
            ctx.merchant_id = m_input.merchant_from_identity(auth);
        }
        return ctx;
    }

private:
    InputSupport& m_input;
};

// ──────────────────────────── verification ──────────────────────────

/** `spec.permission` selects the API-key scheme generation ("v1"/"v2"). */
inline constexpr AuthSpec kVerificationV1{.permission = "v1"};
inline constexpr AuthSpec kVerificationV2{.permission = "v2"};

class VerificationPolicy
{
public:
    using InputSupport = ::app::handlers::verification::VerificationInputSupport;

    explicit VerificationPolicy(InputSupport& input) noexcept :
        m_input{input}
    {
    }

    [[nodiscard]] std::optional<AppError>
    authorize(interfaces::io::IRequest& req, std::string_view operation, const AuthSpec& spec) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        return m_input.authorize_verification(auth, operation, spec.permission == "v2");
    }

    [[nodiscard]] Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        Scope result;
        result.profile_id = auth.profile_id();
        // Registration binds the last path segment; retrieve binds the query
        // parameter. Both are the exact sources the handlers use.
        const auto from_path = m_input.merchant_id_from_path(req);
        if (from_path) {
            result.merchant_id = *from_path;
        } else if (const auto from_query = ::app::http::RequestQuery{}(req).get("merchant_id")) {
            result.merchant_id = *from_query;
        }
        if (const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId)) {
            result.connected_merchant_id = std::string{*connected};
        }
        return result;
    }

    [[nodiscard]] std::optional<AppError> scope_error(const Scope&) const
    {
        return std::nullopt;
    }

    [[nodiscard]] CallContext context(interfaces::io::IRequest& req) const
    {
        return make_call_context(req, scope(req));
    }

private:
    InputSupport& m_input;
};

// ───────────────────────────── subscription ─────────────────────────

inline constexpr AuthSpec kSubscriptionWrite{
    .permission = ::app::handlers::subscription::kWritePermission
};
inline constexpr AuthSpec kSubscriptionRead{
    .permission = ::app::handlers::subscription::kReadPermission
};

/**
 * `operation` selects the auth mode: "api_key", "api_key_or_jwt", "sdk_body"
 * (client-secret claim comes from the request body) or "sdk_query" (client
 * secret comes from the query payload). `spec.permission` is the route's wire
 * permission for the JWT branches.
 */
class SubscriptionPolicy
{
public:
    using InputSupport = ::app::handlers::subscription::SubscriptionInputSupport;

    [[nodiscard]] std::optional<AppError>
    authorize(interfaces::io::IRequest& req, std::string_view operation, const AuthSpec& spec) const
    {
        using ::app::auth::AuthContextBuilder;

        if (operation == "sdk_body") {
            const auto raw_body = req.body_as_string();
            const auto auth = AuthContextBuilder{}(req, raw_body);
            auto payload = m_input.parse_json_body(req);
            const std::string secret =
                payload ? m_input.json_string(payload->root, "client_secret").value_or("")
                        : std::string{};
            return m_input.authorize_sdk(req, auth, spec.permission, secret);
        }
        if (operation == "sdk_query") {
            const auto auth = AuthContextBuilder{}(req);
            const auto query = ::app::http::RequestQuery{}(req);
            const auto payload = m_input.json_from_query(query);
            return m_input.authorize_sdk(
                req,
                auth,
                spec.permission,
                m_input.json_string(payload, "client_secret").value_or("")
            );
        }

        const auto auth = AuthContextBuilder{}(req);
        if (operation == "api_key_or_jwt") {
            return m_input.authorize_api_key_or_jwt(req, auth, spec.permission);
        }
        return m_input.authorize_api_key(req, auth);
    }

    [[nodiscard]] Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        Scope result;
        result.merchant_id = m_input.merchant_id(req, auth);
        const auto profile = m_input.extract_profile_id(req);
        if (profile) {
            result.profile_id = *profile;
        }
        if (const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId)) {
            result.connected_merchant_id = std::string{*connected};
        }
        return result;
    }

    [[nodiscard]] std::optional<AppError> scope_error(const Scope&) const
    {
        return std::nullopt;
    }

    [[nodiscard]] CallContext context(interfaces::io::IRequest& req) const
    {
        auto ctx = make_call_context(req, scope(req));
        if (ctx.client_secret.empty()) {
            if (const auto payload = m_input.parse_json_body_optional(req)) {
                ctx.client_secret =
                    m_input.json_string(payload->root, "client_secret").value_or("");
            }
        }
        return ctx;
    }

private:
    InputSupport m_input{};
};

} // namespace app::webhook::policy
