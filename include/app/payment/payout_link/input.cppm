export module app_payment:payout_link_input;

import std;
import core_behavior;
import interfaces;
import core_error;
import app_context;
import app_auth;
import app_http;

import :context;
export namespace app::handlers::payout_link {

/**
 * @brief Path/auth support for the payout link render handler.
 *
 * Kept free of business logic: it only extracts the path ids, verifies the
 * MerchantIdAuth binding and returns plain data consumed by the runtime.
 */
class PayoutLinkInputSupport
{
public:
    explicit PayoutLinkInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /** @brief Extract (merchant_id, payout_id) from the :merchant_id/:payout_id path. */
    [[nodiscard]] core::behavior::Outcome<std::pair<std::string, std::string>>
    payout_link_ids_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i{0}; i + 2 < segments.count(); ++i) {
            if (segments.at(i).value_or(std::string_view{}) == "payout_link") {
                const auto merchant = segments.at(i + 1);
                const auto payout = segments.at(i + 2);
                if (merchant.has_value() && payout.has_value() && !merchant->empty() &&
                    !payout->empty()) {
                    return std::pair<std::string, std::string>{
                        std::string{*merchant},
                        std::string{*payout}
                    };
                }
                return std::unexpected{core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "payout link path requires merchant_id and payout_id"
                )};
            }
        }
        if (segments.count() >= 2) {
            return std::pair<std::string, std::string>{
                std::string{*segments.at(segments.count() - 2)},
                std::string{*segments.at(segments.count() - 1)}
            };
        }
        return std::unexpected{core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INVALID_REQUEST,
            "payout link path requires merchant_id and payout_id"
        )};
    }

    // &auth::MerchantIdAuth(merchant_id) (routes/payout_link.rs:35): bind the
    // principal to the path merchant id and reject a conflicting header.
    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation,
        std::string_view path_merchant_id
    ) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        auto context = ::app::auth::AuthContextBuilder{}(req);
        if (!context.merchant_id().empty() &&
            context.merchant_id() != std::string{path_merchant_id}) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "merchant id header does not match payout link path"
            );
        }
        context.set_merchant_id(std::string{path_merchant_id});
        return m_context.authorizer().authorize_route(context, rule);
    }

private:
    ::app::context::AppContext& m_context;
};
struct PayoutLinkRenderData
{
    std::string merchant_id{};
    std::string payout_id{};
    std::string sec_fetch_dest{};
    std::string origin{};
    std::string referer{};
    std::string accept_language{};
    explicit PayoutLinkRenderData(const core::behavior::CallContext& ctx) :
        merchant_id{ctx.merchant_id},
        payout_id{::app::payment::detail::last_path(ctx)},
        sec_fetch_dest{::app::payment::detail::header_of(ctx, "sec-fetch-dest")},
        origin{::app::payment::detail::header_of(ctx, "origin")},
        referer{::app::payment::detail::header_of(ctx, "referer")},
        accept_language{::app::payment::detail::header_of(ctx, "accept-language")}
    {
    }
};


/** @brief Request policy for the payout_link domain: wraps the domain auth rules. */
class PayoutLinkPolicy
{
public:
    PayoutLinkPolicy(
        ::app::context::AppContext& /*context*/,
        PayoutLinkInputSupport& support
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
        const auto ids = m_support.payout_link_ids_from_path(req);
        if (ids) {
            return m_support.authorize_operation(req, operation, ids->first);
        }
        return m_support.authorize_operation(req, operation, std::string_view{});
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


private:
    PayoutLinkInputSupport& m_support;
};

using PayoutLinkRequestSupport = core::behavior::RequestSupport<PayoutLinkPolicy>;
using PayoutLinkPolicySpec = core::behavior::AuthSpec;

[[nodiscard]] PayoutLinkPolicySpec spec_for(std::string_view operation) noexcept
{
    return PayoutLinkPolicySpec{.permission = operation};
}
} // namespace app::handlers::payout_link
