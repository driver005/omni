export module app_handlers_card_issuer:input;

import std;
import interfaces;
import core_error;
import app_context;
import app_auth;
import app_http;
import app_dto;

export namespace app::handlers::card_issuer {

/**
 * @brief Parse/auth/path support for the card issuer handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime.
 */
class CardIssuerInputSupport
{
public:
    explicit CardIssuerInputSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Upstream routes/card_issuer.rs AdminApiAuth (add/update/delete) and the
    // list route's ApiKeyAuth OR JWTAuth(MerchantAccountRead) fallback. The
    // original authorize() helper is preserved verbatim, including the rule
    // registry lookup and the inline JWT fallback rule.
    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        bool list_jwt_fallback = false
    ) const
    {
        const auto context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context.auth_rules().rule_for(operation);
        auto auth = m_context.authorizer().authorize_route(context, rule);
        if (auth && list_jwt_fallback) {
            // Upstream routes/card_issuer.rs:85-96: list accepts
            // HeaderAuth(ApiKeyAuth{allow_connected_scope_operation:false,
            // allow_platform_self_operation:false}) OR
            // JWTAuth(MerchantAccountRead, allow_connected:false, allow_platform:false).
            const auto jwt_rule = app::auth::RouteAuthRuleBuilder{}
                                      .scheme(app::auth::AuthScheme::JWT)
                                      .permission(app::auth::AppPermission::MERCHANT_READ)
                                      .allow_connected(false)
                                      .allow_platform(false)
                                      .build();
            auth = m_context.authorizer().authorize_route(context, jwt_rule);
        }
        return auth;
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    // Upstream web::Path<id_type::CardIssuerId> extraction (path id only).
    [[nodiscard]] std::string id_from_request(interfaces::io::IRequest& req) const
    {
        return std::string{app::http::RequestPath{}(req).at(1).value_or("")};
    }

private:
    app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old *_core call received.

struct CardIssuerAddInput
{
    app::dto::card_issuer::CardIssuerRequest request{};
};

struct CardIssuerUpdateInput
{
    std::string id{};
    app::dto::card_issuer::CardIssuerUpdateRequest request{};
};

struct CardIssuerDeleteInput
{
    std::string id{};
};

struct CardIssuerListInput
{};

} // namespace app::handlers::card_issuer
