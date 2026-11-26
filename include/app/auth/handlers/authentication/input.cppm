export module app_auth:authentication_input;

import std;
import interfaces;
import core_error;
import app_context;
import app_dto;
import app_context_core;
import app_http;

export namespace app::handlers::authentication {

// Upstream `api::AuthFlow` (services/api.rs:158): Client for SDK/publishable-key
// auth, Merchant for ApiKeyAuth. Threaded into every authentication_*_core call
// by the route layer (routes/authentication.rs:108-112,153-157,232-236).
enum class AuthFlowKind
{
    Client,
    Merchant,
};

struct AuthorizedRequest
{
    std::optional<core::error::AppError> error{};
    ::app::auth::AuthContext context{};
    AuthFlowKind flow{AuthFlowKind::Merchant};
    std::string client_secret{};
};

/**
 * @brief Parse/auth/path/query support for the authentication handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime.
 */
class AuthenticationInputSupport
{
public:
    explicit AuthenticationInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ───────────────────────── Auth semantics ─────────────────────────
    // routes/authentication.rs:40-43 and :205-208 declare
    // HeaderAuth(ApiKeyAuth { allow_connected_scope_operation: false,
    // allow_platform_self_operation: false }). The shared rule registry currently records
    // authentication_create as (true,true) — see "## Shared contract requests" in the report —
    // so the exact upstream scope flags are applied here with an inline rule
    // (the same pattern used by card_issuer.cppm:359-365).
    [[nodiscard]] AuthorizedRequest authorize_api_key(
        interfaces::io::IRequest& req,
        std::string_view operation,
        ::app::auth::AppPermission permission
    ) const
    {
        const auto context = ::app::auth::AuthContextBuilder{}(req);
        const auto rule = ::app::auth::RouteAuthRuleBuilder{}
                              .scheme(::app::auth::AuthScheme::API_KEY)
                              .permission(permission)
                              .allow_connected(false)
                              .allow_platform(false)
                              .build();
        auto denied = m_context.authorizer().authorize_route(context, rule);
        (void)operation;
        return AuthorizedRequest{std::move(denied), context, AuthFlowKind::Merchant, {}};
    }

    [[nodiscard]] AuthorizedRequest
    authorize_merchant(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto context = ::app::auth::AuthContextBuilder{}(req);
        const auto rule = ::app::auth::RouteAuthRuleBuilder{}
                              .scheme(::app::auth::AuthScheme::HEADER_MERCHANT)
                              .permission(::app::auth::AppPermission::PAYMENT_WRITE)
                              .allow_connected(false)
                              .allow_platform(false)
                              .build();
        auto denied = m_context.authorizer().authorize_route(context, rule);
        (void)operation;
        return AuthorizedRequest{std::move(denied), context, AuthFlowKind::Merchant, {}};
    }

    // Upstream services/authentication.rs:6037-6065 (check_sdk_auth_and_get_auth) and
    // :5996-6033 (check_client_secret_and_get_auth):
    //  * Authorization (non-Bearer) / publishable-key + client_secret -> AuthFlow::Client
    //  * api-key (non "pk_") -> AuthFlow::Merchant; a client_secret in the payload is rejected
    //  * "pk_" without client_secret -> MissingRequiredField(client_secret)
    // ApiKeyAuth::default() carries both scope flags false (derive Default, :430-434).
    [[nodiscard]] AuthorizedRequest authorize_sdk(
        interfaces::io::IRequest& req,
        std::string_view operation,
        std::string_view body,
        const std::optional<std::string>& body_secret
    ) const
    {
        const auto context = ::app::auth::AuthContextBuilder{}(req, body);
        const auto api_key = context.claim(::app::auth::kClaimApiKey);
        const auto publishable = context.claim(::app::auth::kClaimPublishableKey);
        const auto header_secret = context.claim(::app::auth::kClaimClientSecret);
        const bool sdk_authorization = context.claim(::app::auth::kClaimSdkKey).has_value();

        std::optional<core::error::AppError> denied{};
        AuthFlowKind flow{AuthFlowKind::Merchant};
        std::string effective_secret{};

        if (sdk_authorization || (publishable.has_value() && !publishable->empty())) {
            if (header_secret.has_value() && !header_secret->empty()) {
                effective_secret = std::string{*header_secret};
            } else if (body_secret.has_value() && !body_secret->empty()) {
                effective_secret = *body_secret;
            } else {
                denied = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::VALIDATION_FAILED,
                    "MissingRequiredField: client_secret"
                );
            }
            const auto rule = ::app::auth::RouteAuthRuleBuilder{}
                                  .scheme(::app::auth::AuthScheme::SDK)
                                  .permission(::app::auth::AppPermission::PAYMENT_WRITE)
                                  .allow_connected(false)
                                  .allow_platform(false)
                                  .build();
            denied = m_context.authorizer().authorize_route(context, rule);
            flow = AuthFlowKind::Client;
        } else if (api_key.has_value() && api_key->starts_with("pk_")) {
            if (!body_secret.has_value() || body_secret->empty()) {
                denied = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::VALIDATION_FAILED,
                    "MissingRequiredField: client_secret"
                );
            }
            if (header_secret.has_value() && !header_secret->empty()) {
                effective_secret = std::string{*header_secret};
            } else if (body_secret.has_value()) {
                effective_secret = *body_secret;
            }
            const auto rule = ::app::auth::RouteAuthRuleBuilder{}
                                  .scheme(::app::auth::AuthScheme::PUBLISHABLE_KEY)
                                  .permission(::app::auth::AppPermission::PAYMENT_WRITE)
                                  .allow_connected(false)
                                  .allow_platform(false)
                                  .build();
            denied = m_context.authorizer().authorize_route(context, rule);
            flow = AuthFlowKind::Client;
        } else {
            if (body_secret.has_value() && !body_secret->empty()) {
                denied = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "client_secret is not a valid parameter"
                );
            }
            const auto rule = ::app::auth::RouteAuthRuleBuilder{}
                                  .scheme(::app::auth::AuthScheme::API_KEY)
                                  .permission(::app::auth::AppPermission::PAYMENT_WRITE)
                                  .allow_connected(false)
                                  .allow_platform(false)
                                  .build();
            denied = m_context.authorizer().authorize_route(context, rule);
            flow = AuthFlowKind::Merchant;
        }
        (void)operation;
        return AuthorizedRequest{std::move(denied), context, flow, std::move(effective_secret)};
    }

    // Path helpers return the error instead of writing it, so the handler keeps
    // the single response-writing path (set_status/set_body/send) for every route.
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    auth_id_from_path(interfaces::io::IRequest& req, std::size_t index) const
    {
        const auto path = ::app::http::RequestPath{}(req);
        const auto segment = path.at(index);
        if (!segment.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "authentication id missing in path"
            )};
        }
        return std::string{*segment};
    }

    [[nodiscard]] std::expected<std::pair<std::string, std::string>, core::error::AppError>
    merchant_and_auth_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto path = ::app::http::RequestPath{}(req);
        const auto merchant_segment = path.at(1);
        const auto id_segment = path.at(2);
        if (!merchant_segment.has_value() || !id_segment.has_value()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "merchant id or authentication id missing in path"
            )};
        }
        return std::make_pair(std::string{*merchant_segment}, std::string{*id_segment});
    }

private:
    ::app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old *_core call received.

struct AuthenticationCreateInput
{
    ::app::dto::authentication::AuthenticationCreateRequest request{};
    std::string merchant_id{};
    AuthorizedRequest auth{};
};

struct AuthenticationEligibilityInput
{
    ::app::dto::authentication::AuthenticationEligibilityRequest request{};
    std::string authentication_id{};
    AuthorizedRequest auth{};
    std::string body{};
};

struct AuthenticationAuthenticateInput
{
    ::app::dto::authentication::AuthenticationAuthenticateRequest request{};
    std::string authentication_id{};
    AuthorizedRequest auth{};
};

struct AuthenticationEligibilityCheckInput
{
    ::app::dto::authentication::AuthenticationEligibilityCheckRequest request{};
    std::string authentication_id{};
    AuthorizedRequest auth{};
    std::string body{};
};

struct AuthenticationRetrieveEligibilityCheckInput
{
    std::string authentication_id{};
    AuthorizedRequest auth{};
};

struct AuthenticationSyncInput
{
    ::app::dto::authentication::AuthenticationSyncRequest request{};
    std::string merchant_id{};
    std::string authentication_id{};
    AuthorizedRequest auth{};
};

struct AuthenticationSyncPostUpdateInput
{
    std::string authentication_id{};
    std::string merchant_id{};
    AuthorizedRequest auth{};
};

struct AuthenticationSessionTokenInput
{
    ::app::dto::authentication::AuthenticationSessionTokenRequest request{};
    std::string authentication_id{};
    AuthorizedRequest auth{};
};

} // namespace app::handlers::authentication
