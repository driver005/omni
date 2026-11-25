export module app_handlers_webhooks:input;

import std;
import interfaces;
import core_error;
import app_context;
import app_auth;
import app_http;

export namespace app::handlers::webhooks {

/** Which upstream auth extractor a route uses (routes/webhooks.rs:40,79,120-123,163-166,190). */
enum class AuthVariant : std::uint8_t
{
    MERCHANT_ID,
    MERCHANT_AND_PROFILE_ID,
    NO_AUTH,
};

struct PathIds
{
    std::string merchant_id{};
    std::string profile_id{};
    std::string connector_id{};
};

/**
 * Deferred header lookup. The handler supplies a lambda that reads the live
 * request (`HeaderReader{}.get`), so the runtime can resolve config-named
 * headers without ever touching an HTTP type.
 */
using HeaderReaderFn = std::function<std::string(std::string_view)>;

/**
 * @brief Parse/auth/path/query support for the webhook handlers.
 *
 * Free of business logic: turns request data into the plain Input structs
 * consumed by the domain runtime. All error shapes are produced here as
 * `AppError`s whose `kind_name` carries the upstream ApiErrorResponse code
 * (IR_01 / WE_01 / ...); the handler re-renders the nested wire JSON.
 */
class WebhooksInputSupport
{
public:
    explicit WebhooksInputSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] HeaderReaderFn header_reader(interfaces::io::IRequest& req) const
    {
        return [&req](std::string_view name) -> std::string
        {
            return std::string{req.find_header(name)};
        };
    }

    [[nodiscard]] std::size_t id_base(const app::http::PathSegments& segments) const
    {
        std::size_t index = segments.at(0).value_or("") == "v2" ? 1 : 0;
        while (index < segments.count()) {
            const auto value = segments.at(index).value_or("");
            if (value == "webhooks" || value == "relay" || value == "recovery") {
                ++index;
                continue;
            }
            break;
        }
        return index;
    }

    [[nodiscard]] PathIds path_ids(interfaces::io::IRequest& req, bool with_profile) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        const std::size_t base = id_base(segments);
        PathIds result;
        result.merchant_id = std::string{segments.at(base).value_or("")};
        const std::size_t connector_index = base + (with_profile ? 2 : 1);
        result.connector_id = std::string{segments.at(connector_index).value_or("")};
        if (with_profile) {
            result.profile_id = std::string{segments.at(base + 1).value_or("")};
        }
        return result;
    }

    // Local typed rules mirror the upstream extractors (routes/webhooks.rs:40,
    // :79, :120-123, :163-166, :190). The shared auth registry still carries the
    // legacy HEADER_MERCHANT rule; the handler builds the typed rule locally so
    // the auth semantics stay upstream-identical without editing shared files.
    [[nodiscard]] app::auth::RouteAuthRule auth_rule(AuthVariant variant) const
    {
        using app::auth::AppPermission;
        using app::auth::AuthScheme;
        using app::auth::RouteAuthRuleBuilder;

        if (variant == AuthVariant::NO_AUTH) {
            return RouteAuthRuleBuilder{}
                .scheme(AuthScheme::NO_AUTH)
                .permission(AppPermission::ADMIN)();
        }
        if (variant == AuthVariant::MERCHANT_AND_PROFILE_ID) {
            return RouteAuthRuleBuilder{}
                .scheme(AuthScheme::MERCHANT_ID)
                .permission(AppPermission::WEBHOOK_WRITE)
                .id_source("x-profile-id")();
        }
        return RouteAuthRuleBuilder{}
            .scheme(AuthScheme::MERCHANT_ID)
            .permission(AppPermission::WEBHOOK_WRITE)();
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req, AuthVariant variant, const PathIds& ids) const
    {
        auto context = app::auth::AuthContextBuilder{}(req);
        if (!ids.merchant_id.empty()) {
            context.set_merchant_id(ids.merchant_id);
            context.add_claim("x-merchant-id", ids.merchant_id);
        }
        if (!ids.profile_id.empty()) {
            context.set_profile_id(ids.profile_id);
            context.add_claim("x-profile-id", ids.profile_id);
        }
        const auto denied = m_context.authorizer().authorize_route(context, auth_rule(variant));
        if (!denied) {
            return std::nullopt;
        }
        // Upstream MerchantIdAuth / MerchantIdAndProfileIdAuth failures are
        // ApiErrorResponse::Unauthorized (api_error_response.rs:560-564).
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::UNAUTHORIZED,
            "API key not provided or invalid API key used"
        );
        error.set_kind_name("IR_01");
        return error;
    }

private:
    app::context::AppContext& m_context;
};

/** Plain inputs — everything the old handle_* methods read off the request. */
struct ReceiveIncomingWebhookInput
{
    PathIds ids{};
    std::string body{};
    HeaderReaderFn header{};
    std::string operation{};
    std::string flow{};
    std::string counter{};
};

struct ReceiveIncomingWebhookV2Input
{
    PathIds ids{};
    std::string body{};
    HeaderReaderFn header{};
    std::string operation{};
    std::string flow{};
    std::string counter{};
};

struct ReceiveNetworkTokenInput
{
    PathIds ids{};
    std::string body{};
    HeaderReaderFn header{};
    std::string operation{};
    std::string flow{};
    std::string counter{};
};

} // namespace app::handlers::webhooks
