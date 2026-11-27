export module app_webhook:webhook_events_input;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_auth;
import app_http;

export namespace app::handlers::webhook_events {

struct PathIds
{
    std::string merchant_id{};
    // Attempts route: upstream initial_attempt_id. Retry route: event_id.
    std::string event_id{};
};

struct WebhookEventsAuthorizedIdentity
{
    std::optional<core::error::AppError> error{};
    ::app::auth::AuthContext context{};
};

/**
 * @brief Parse/auth/path support for the webhook_events handlers.
 *
 * Only request-to-Input work lives here: authorization, path ids and the
 * merchant/profile checks the route layer performs before the core runs.
 */
class WebhookEventsInputSupport
{
public:
    explicit WebhookEventsInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Local typed rules mirror the upstream extractors
    // (routes/webhook_events.rs:46-51, :95-99, :131-136, :169-174). The shared
    // registry carries only the legacy flat permissions; building the rule here
    // keeps allow_connected / allow_platform upstream-identical without editing
    // the shared auth module.
    [[nodiscard]] ::app::auth::RouteAuthRule auth_rule(std::string_view operation) const
    {
        using ::app::auth::AppPermission;
        using ::app::auth::AuthScheme;
        using ::app::auth::RouteAuthRuleBuilder;

        const AppPermission permission = operation == "retry_webhook_delivery_attempt"
                                             ? AppPermission::WEBHOOK_WRITE
                                             : AppPermission::WEBHOOK_READ;
        return RouteAuthRuleBuilder{}
            .scheme(AuthScheme::JWT)
            .permission(permission)
            .allow_connected(true)
            .allow_platform(true)
            .build();
    }

    [[nodiscard]] WebhookEventsAuthorizedIdentity
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        auto identity = ::app::auth::AuthContextBuilder{}(req);
        auto denied = m_context.authorizer().authorize_route(identity, auth_rule(operation));
        return WebhookEventsAuthorizedIdentity{std::move(denied), std::move(identity)};
    }

    // JWTAuthMerchantFromRoute extracts merchant_id from the path and records it
    // as a claim before the rule is applied (routes/webhook_events.rs:18-55).
    [[nodiscard]] WebhookEventsAuthorizedIdentity authorize_with_merchant(
        interfaces::io::IRequest& req,
        std::string_view operation,
        std::string_view merchant_id
    ) const
    {
        auto identity = ::app::auth::AuthContextBuilder{}(req);
        identity.set_merchant_id(std::string{merchant_id});
        identity.add_claim("x-merchant-id", std::string{merchant_id});
        auto denied = m_context.authorizer().authorize_route(identity, auth_rule(operation));
        return WebhookEventsAuthorizedIdentity{std::move(denied), std::move(identity)};
    }

    [[nodiscard]] std::string merchant_from_identity(const ::app::auth::AuthContext& identity) const
    {
        if (!identity.merchant_id().empty()) {
            return identity.merchant_id();
        }
        if (const auto connected = identity.claim(::app::auth::kClaimConnectedMerchantId)) {
            return std::string{*connected};
        }
        return {};
    }

    // Local in-process substitute for find_business_profile_by_merchant_id_profile_id
    // / find_business_profile_by_profile_id (core:30-53, :344-353).
    [[nodiscard]] std::optional<core::error::AppError>
    validate_profile(const std::string& profile_id, const std::string& merchant_id) const
    {
        if (profile_id.empty()) {
            return std::nullopt;
        }
        const auto stored = m_context.config_store().get_value("business_profile", profile_id);
        if (!stored) {
            m_context.logger().counter("WEBHOOK_EVENT_PROFILE_UNRESOLVED").increment();
            return std::nullopt;
        }
        if (!merchant_id.empty() && *stored != merchant_id) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "business profile does not belong to merchant"
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] bool is_generic_json(std::string_view body) const
    {
        return serde::Ser::decode_generic("application/json", body).has_value();
    }

    // Skip router/literal prefixes so ids resolve for both mounted forms.
    [[nodiscard]] std::size_t id_base(const ::app::http::PathSegments& segments) const
    {
        std::size_t index = 0;
        while (index < segments.count()) {
            const auto value = segments.at(index).value_or("");
            if (value == "events" || value == "profile" || value == "list") {
                ++index;
                continue;
            }
            break;
        }
        return index;
    }

    [[nodiscard]] PathIds path_ids(interfaces::io::IRequest& req) const
    {
        const ::app::http::PathSegments segments{::app::http::RequestPath{}(req)};
        const std::size_t base = id_base(segments);
        return PathIds{
            std::string{segments.at(base).value_or("")},
            std::string{segments.at(base + 1).value_or("")},
        };
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::webhook_events
