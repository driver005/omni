export module app_auth:hypersense_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_dto;
import app_context_core;
import app_http;

import :call_context;

export namespace app::handlers::hypersense {

/**
 * @brief Domain policy for the dashboard-no-permission hypersense routes.
 *
 * These routes apply no declarative authorization; the bearer credential and
 * merchant identity are request facts exposed through RequestSupport::scope.
 */
class HypersensePolicy
{
public:
    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)req;
        (void)operation;
        (void)spec;
        return std::nullopt;
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        core::behavior::Scope out{};
        out.merchant_id = ::app::http::merchant_id_from_header{}(req);
        return out;
    }

    [[nodiscard]] core::behavior::CallContext
    context(interfaces::io::IRequest& req) const
    {
        return detail::make_call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope& scope) const
    {
        (void)scope;
        return std::nullopt;
    }
};

} // namespace app::handlers::hypersense
