export module app_payment:tokenization_input;

import std;
import core_behavior;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_auth;
import app_http;

import :context;
export namespace app::handlers::tokenization {

struct AuthorizedRequest
{
    std::optional<core::error::AppError> error{};
    ::app::auth::AuthContext context{};
};

/**
 * @brief Parse/auth/path support for the tokenization handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime.
 */
class TokenizationInputSupport
{
public:
    explicit TokenizationInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ───────────────────────── Request parsing ─────────────────────────
    // Secret<serde_json::Value> is transparent on the wire: `token_request` is an
    // arbitrary JSON value kept as its exact encoded text (or "null" when absent).
    // A typed Ser::deserialize<GenericTokenizationRequest> cannot express this, so the
    // original parse shape is preserved here. Empty-body / decode / shape failures are
    // all reported by the handler as INVALID_JSON "invalid JSON".
    [[nodiscard]] std::optional<::app::dto::tokenization::GenericTokenizationRequest>
    parse_create_request(std::string_view body) const
    {
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            return std::nullopt;
        }
        const auto* object = std::get_if<serde::Value::Object>(&parsed->get());
        if (object == nullptr) {
            return std::nullopt;
        }
        ::app::dto::tokenization::GenericTokenizationRequest request{};
        if (const auto field = object->get("customer_id")) {
            if (const auto* text = std::get_if<std::string>(&field.value().get())) {
                request.set_customer_id(*text);
            }
        }
        if (const auto field = object->get("token_request")) {
            request.set_token_request(serde::Ser::encode_json(field.value()));
        } else {
            request.set_token_request("null");
        }
        return request;
    }

    [[nodiscard]] std::optional<::app::dto::tokenization::DeleteTokenDataRequest>
    parse_delete_request(std::string_view body) const
    {
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            return std::nullopt;
        }
        const auto* object = std::get_if<serde::Value::Object>(&parsed->get());
        if (object == nullptr) {
            return std::nullopt;
        }
        ::app::dto::tokenization::DeleteTokenDataRequest request{};
        if (const auto field = object->get("customer_id")) {
            if (const auto* text = std::get_if<std::string>(&field.value().get())) {
                request.set_customer_id(*text);
            }
        }
        if (const auto field = object->get("session_id")) {
            if (const auto* text = std::get_if<std::string>(&field.value().get())) {
                request.set_session_id(*text);
            }
        }
        return request;
    }

    // ───────────────────────── Auth ─────────────────────────
    // routes/tokenization.rs:56-65 / :92-101 — api_or_client_auth(
    // V2ApiKeyAuth{false,false}, V2ClientAuth(ResourceId::Customer / PaymentMethodSession)).
    [[nodiscard]] AuthorizedRequest authorize_request(
        interfaces::io::IRequest& req,
        std::string_view operation,
        std::string_view bound_resource
    ) const
    {
        const auto context = ::app::auth::AuthContextBuilder{}(req);
        auto error = authorize_request_impl(context, operation, bound_resource);
        return AuthorizedRequest{std::move(error), context};
    }

    // Upstream: provider.get_account().get_id() (core/tokenization.rs:72). The local
    // substitute uses the auth-derived merchant id (header-backed) canonicalized via
    // the repository to its stored provider-account value.
    [[nodiscard]] std::string
    resolve_merchant_id(interfaces::io::IRequest& req, const ::app::auth::AuthContext& auth) const
    {
        std::string merchant_id = auth.merchant_id();
        if (merchant_id.empty()) {
            merchant_id = std::string{::app::http::merchant_id_from_header{}(req)};
        }
        if (const auto account =
                m_context.repository().find_merchant_account_by_string(merchant_id)) {
            merchant_id = account->get_merchant_id();
        }
        return merchant_id;
    }

    // routes/tokenization.rs:82 — GlobalTokenId comes from web::Path, not the body.
    [[nodiscard]] core::behavior::Outcome<std::string>
    token_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto path = ::app::http::RequestPath{}(req);
        const std::string token_id = std::string{path.at(2).value_or("")};
        if (token_id.empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing token id"
            )};
        }
        return token_id;
    }

private:
    [[nodiscard]] std::optional<core::error::AppError> authorize_request_impl(
        const ::app::auth::AuthContext& context,
        std::string_view operation,
        std::string_view bound_resource
    ) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);

        // Primary api_or_client_auth branch: V2ApiKeyAuth with both overrides false.
        auto denied = m_context.authorizer().authorize_route(context, rule);
        if (!denied) {
            return enforce_no_connected_no_platform(context, rule);
        }

        // Fallback branch: V2ClientAuth(ResourceId::Customer / PaymentMethodSession).
        if (bound_resource.empty()) {
            return denied;
        }
        auto scoped = context;
        scoped.add_claim("x-resource-id", std::string{bound_resource});
        const auto client_rule = ::app::auth::RouteAuthRuleBuilder{}
                                     .scheme(::app::auth::AuthScheme::SDK)
                                     .permission(rule.permission())
                                     .allow_connected(false)
                                     .allow_platform(false)
                                     .id_source("x-resource-id")
                                     .build();
        denied = m_context.authorizer().authorize_route(scoped, client_rule);
        if (denied) {
            return denied;
        }
        return enforce_no_connected_no_platform(scoped, client_rule);
    }

    [[nodiscard]] std::optional<core::error::AppError> enforce_no_connected_no_platform(
        const ::app::auth::AuthContext& context,
        const ::app::auth::RouteAuthRule& rule
    ) const
    {
        // V2ApiKeyAuth { allow_connected_scope_operation: false,
        // allow_platform_self_operation: false } — routes/tokenization.rs:57-60,93-96.
        if (!rule.allow_connected()) {
            const auto connected = context.claim(::app::auth::kClaimConnectedMerchantId);
            if (connected.has_value() && !connected->empty()) {
                return core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::FORBIDDEN,
                    "connected-scope operations are not allowed for tokenization"
                );
            }
        }
        if (!rule.allow_platform()) {
            const auto internal_key = context.claim(::app::auth::kClaimInternalApiKey);
            if (internal_key.has_value() && !internal_key->empty()) {
                return core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::FORBIDDEN,
                    "platform self-operations are not allowed for tokenization"
                );
            }
        }
        return std::nullopt;
    }

    ::app::context::AppContext& m_context;
};
struct CreateTokenVaultData
{
    ::app::dto::tokenization::GenericTokenizationRequest request{};
    std::string merchant_id{};
    template<class Dto>
    explicit CreateTokenVaultData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{::app::payment::detail::as_dto<::app::dto::tokenization::GenericTokenizationRequest>(dto)},
        merchant_id{ctx.merchant_id}
    {
    }
};
struct DeleteTokenizedDataData
{
    std::string token_id{};
    std::string customer_id{};
    explicit DeleteTokenizedDataData(const core::behavior::CallContext& ctx) :
        token_id{::app::payment::detail::last_path(ctx)},
        customer_id{::app::payment::detail::last_path(ctx)}
    {
    }
};


/** @brief Request policy for the tokenization domain: wraps the domain auth rules. */
class TokenizationPolicy
{
public:
    TokenizationPolicy(
        ::app::context::AppContext& /*context*/,
        TokenizationInputSupport& support
    ) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] core::behavior::CallContext context(interfaces::io::IRequest& req) const
    {
        return ::app::payment::detail::make_call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)spec;
        m_last = m_support.authorize_request(req, operation, spec.permission);
        return m_last->error;
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        core::behavior::Scope result;
        result.merchant_id = std::string{auth.merchant_id()};
        result.profile_id = std::string{auth.profile_id()};
        result.org_id = std::string{auth.org_id()};
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        result.connected_merchant_id = connected ? std::string{*connected} : std::string{};
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope&) const
    {
        return std::nullopt;
    }

    [[nodiscard]] const AuthorizedRequest& last() const noexcept
    {
        return *m_last;
    }

private:
    TokenizationInputSupport& m_support;
    mutable std::optional<AuthorizedRequest> m_last{};
};

using TokenizationRequestSupport = core::behavior::RequestSupport<TokenizationPolicy>;
using TokenizationPolicySpec = core::behavior::AuthSpec;

[[nodiscard]] TokenizationPolicySpec spec_for(std::string_view operation) noexcept
{
    return TokenizationPolicySpec{.permission = operation};
}
} // namespace app::handlers::tokenization
