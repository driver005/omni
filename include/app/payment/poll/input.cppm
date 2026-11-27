export module app_payment:poll_input;

import std;
import core_behavior;
import interfaces;
import core_error;
import app_context;
import app_dto;
import app_auth;
import app_http;

import :context;
export namespace app::handlers::poll {

struct PollAuthorizedRequest
{
    std::optional<core::error::AppError> error{};
    ::app::auth::AuthContext context{};
};

/**
 * @brief Parse/auth/path support for the poll handler.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * consumed by the domain runtime. The auth extractor selection mirrors
 * routes/poll.rs:37-50 (Authorization -> SDK, otherwise publishable key).
 */
class PollInputSupport
{
public:
    explicit PollInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // routes/poll.rs:37-50 — dynamic auth extractor selection. The base rule
    // gates registration; an Authorization header selects SDK authorization,
    // otherwise HeaderAuth(PublishableKeyAuth). Both allow connected scope and
    // deny platform-self operations.
    [[nodiscard]] PollAuthorizedRequest
    authorize_retrieve_poll_status(interfaces::io::IRequest& req) const
    {
        const auto auth_ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto base_rule = m_context.auth_rules().rule_for("retrieve_poll_status");
        if (base_rule.scheme() == ::app::auth::AuthScheme::FORBIDDEN) {
            return PollAuthorizedRequest{
                std::optional<core::error::AppError>{core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::FORBIDDEN,
                    "operation is not registered in the auth rule registry"
                )},
                auth_ctx
            };
        }
        const bool has_authorization = !req.find_header("authorization").empty();
        const auto rule = ::app::auth::RouteAuthRuleBuilder{}
                              .scheme(
                                  has_authorization ? ::app::auth::AuthScheme::SDK
                                                    : ::app::auth::AuthScheme::PUBLISHABLE_KEY
                              )
                              .permission(base_rule.permission())
                              .allow_connected(true)
                              .allow_platform(false)
                              .build();
        auto denied = m_context.authorizer().authorize_route(auth_ctx, rule);
        return PollAuthorizedRequest{std::move(denied), auth_ctx};
    }

    // routes/poll.rs:31,34-36 — web::Path<String> wrapped into PollId. Returns
    // the error instead of writing it so the handler keeps one response path.
    [[nodiscard]] core::behavior::Outcome<std::string>
    poll_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        if (segments.count() == 0) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing poll id"
            )};
        }
        const auto segment = segments.at(segments.count() - 1);
        if (!segment || segment->empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing poll id"
            )};
        }
        return std::string{*segment};
    }

    /**
     * @brief auth.platform.get_processor() -> processor.get_account().get_id()
     * (routes/poll.rs:57, core/poll.rs:25). No processor domain object exists in
     * this port, so the authenticated merchant id (falling back to the header)
     * is canonicalized through the merchant-account repository.
     */
    [[nodiscard]] std::string resolve_processor_account_id(
        interfaces::io::IRequest& req,
        const ::app::auth::AuthContext& auth_ctx
    ) const
    {
        std::string merchant_id{auth_ctx.merchant_id()};
        if (merchant_id.empty()) {
            merchant_id = std::string{::app::http::merchant_id_from_header{}(req)};
        }
        if (const auto account =
                m_context.repository().find_merchant_account_by_string(merchant_id)) {
            merchant_id = account->get_merchant_id();
        }
        return merchant_id;
    }

private:
    ::app::context::AppContext& m_context;
};
struct RetrievePollStatusData
{
    std::string request_poll_id{};
    std::string processor_merchant_id{};
    explicit RetrievePollStatusData(const core::behavior::CallContext& ctx) :
        request_poll_id{::app::payment::detail::last_path(ctx)},
        processor_merchant_id{ctx.merchant_id}
    {
    }
};


/** @brief Request policy for the poll domain: wraps the domain auth rules. */
class PollPolicy
{
public:
    PollPolicy(::app::context::AppContext& /*context*/, PollInputSupport& support) noexcept :
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
        m_last = m_support.authorize_retrieve_poll_status(req);
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

    [[nodiscard]] const PollAuthorizedRequest& last() const noexcept
    {
        return *m_last;
    }

private:
    PollInputSupport& m_support;
    mutable std::optional<PollAuthorizedRequest> m_last{};
};

using PollRequestSupport = core::behavior::RequestSupport<PollPolicy>;
using PollPolicySpec = core::behavior::AuthSpec;

[[nodiscard]] PollPolicySpec spec_for(std::string_view operation) noexcept
{
    return PollPolicySpec{.permission = operation};
}
} // namespace app::handlers::poll
