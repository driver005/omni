export module app_payment:payout_link_input;

import std;
import interfaces;
import core_error;
import app_context;
import app_auth;
import app_http;

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
    [[nodiscard]] std::expected<std::pair<std::string, std::string>, core::error::AppError>
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

// The render route has no request body. The old core read the sec-fetch-dest /
// origin / referer / accept-language headers directly off IRequest; they are
// captured here so the runtime stays free of HTTP types.
struct PayoutLinkRenderInput
{
    std::string merchant_id{};
    std::string payout_id{};
    std::string sec_fetch_dest{};
    std::string origin{};
    std::string referer{};
    std::string accept_language{};
};

} // namespace app::handlers::payout_link
