export module app_payment:card_issuer_input;

import std;
import core_behavior;
import interfaces;
import core_error;
import app_context;
import app_auth;
import app_http;
import app_dto;

import :context;

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
    explicit CardIssuerInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Upstream routes/card_issuer.rs AdminApiAuth (add/update/delete) and the
    // list route's ApiKeyAuth OR JWTAuth(MerchantAccountRead) fallback. The
    // original authorize() helper is preserved verbatim, including the rule
    // registry lookup and the inline JWT fallback rule.
    [[nodiscard]] core::behavior::CallContext context(interfaces::io::IRequest& req) const
    {
        return ::app::payment::detail::make_call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        bool list_jwt_fallback = false
    ) const
    {
        const auto context = ::app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context.auth_rules().rule_for(operation);
        auto auth = m_context.authorizer().authorize_route(context, rule);
        if (auth && list_jwt_fallback) {
            // Upstream routes/card_issuer.rs:85-96: list accepts
            // HeaderAuth(ApiKeyAuth{allow_connected_scope_operation:false,
            // allow_platform_self_operation:false}) OR
            // JWTAuth(MerchantAccountRead, allow_connected:false, allow_platform:false).
            const auto jwt_rule = ::app::auth::RouteAuthRuleBuilder{}
                                      .scheme(::app::auth::AuthScheme::JWT)
                                      .permission(::app::auth::AppPermission::MERCHANT_READ)
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
        return std::string{::app::http::RequestPath{}(req).at(1).value_or("")};
    }

private:
    ::app::context::AppContext& m_context;
};

struct CardIssuerAddData
{
    ::app::dto::card_issuer::CardIssuerRequest request{};

    template<class Dto>
    explicit CardIssuerAddData(const Dto& dto, const core::behavior::CallContext& ctx) :
        request{::app::payment::detail::as_dto<::app::dto::card_issuer::CardIssuerRequest>(dto)}
    {
    }
};

struct CardIssuerUpdateData
{
    std::string id{};
    ::app::dto::card_issuer::CardIssuerUpdateRequest request{};

    template<class Dto>
    explicit CardIssuerUpdateData(const Dto& dto, const core::behavior::CallContext& ctx) :
        id{::app::payment::detail::last_path(ctx)},
        request{
            ::app::payment::detail::as_dto<::app::dto::card_issuer::CardIssuerUpdateRequest>(dto)
        }
    {
    }
};

struct CardIssuerDeleteData
{
    std::string id{};

    explicit CardIssuerDeleteData(const core::behavior::CallContext& ctx) :
        id{::app::payment::detail::last_path(ctx)}
    {
    }
};

struct CardIssuerListData
{
    explicit CardIssuerListData(const core::behavior::CallContext& ctx)
    {
        (void)ctx;
    }
};

class CardIssuerPolicy
{
public:
    CardIssuerPolicy(::app::context::AppContext&, CardIssuerInputSupport& support) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] core::behavior::CallContext context(interfaces::io::IRequest& req) const
    {
        return m_support.context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec&
    ) const
    {
        return m_support.authorize(req, operation);
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest&) const
    {
        return {};
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope&) const
    {
        return std::nullopt;
    }

private:
    CardIssuerInputSupport& m_support;
};

using CardIssuerRequestSupport = core::behavior::RequestSupport<CardIssuerPolicy>;
using CardIssuerPolicySpec = core::behavior::AuthSpec;

[[nodiscard]] CardIssuerPolicySpec spec_for(std::string_view operation) noexcept
{
    return CardIssuerPolicySpec{.permission = operation};
}
} // namespace app::handlers::card_issuer
