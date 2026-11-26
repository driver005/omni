export module app_store:currency_input;

import std;
import interfaces;
import core_error;
import app_context;
import app_context_core;
import app_http;
import app_dto;

export namespace app::handlers::currency {

// server_wrap auth_type(ApiKeyAuth, DashboardNoPermissionAuth) OR-path outcome.
// Carried into the Input exactly as the old route layer carried the AuthContext.
struct ForexAuth
{
    std::optional<core::error::AppError> error{};
    ::app::auth::AuthContext context{};
};

/**
 * @brief Query/auth support for the currency handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime.
 */
class CurrencyInputSupport
{
public:
    explicit CurrencyInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ── server_wrap auth_type(ApiKeyAuth, DashboardNoPermissionAuth) OR-path ─
    [[nodiscard]] ForexAuth authorize_forex(interfaces::io::IRequest& req) const
    {
        const auto context = ::app::auth::AuthContextBuilder{}(req);
        // HeaderAuth(ApiKeyAuth) first (registry: retrieve_forex = API_KEY).
        auto denied = m_context.authorizer().authorize_route(
            context,
            m_context.auth_rules().rule_for("retrieve_forex")
        );
        if (!denied) {
            return ForexAuth{std::nullopt, context};
        }
        // DashboardNoPermissionAuth second (registry: convert_forex = JWT).
        denied = m_context.authorizer().authorize_route(
            context,
            m_context.auth_rules().rule_for("convert_forex")
        );
        return ForexAuth{std::move(denied), context};
    }

    // ── request parsing (Query<CurrencyConversionParams>) ────────────────────
    // The Query extractor's deserialization failure maps to InvalidRequestData.
    [[nodiscard]] std::
        expected<::app::dto::currency::CurrencyConversionParams, core::error::AppError>
        parse_conversion_params(interfaces::io::IRequest& req) const
    {
        const auto query = ::app::http::RequestQuery{}(req);
        const auto amount = query.get("amount");
        const auto to = query.get("to_currency");
        const auto from = query.get("from_currency");
        if (!amount || !to || !from) {
            return std::unexpected{
                invalid_request_data("Missing or invalid currency conversion query parameter")
            };
        }
        const auto minor = ::app::dto::currency::MinorUnit::from_string(*amount);
        if (!minor) {
            return std::unexpected{
                invalid_request_data("Missing or invalid currency conversion query parameter")
            };
        }
        ::app::dto::currency::CurrencyConversionParams params;
        params.set_amount(*minor);
        params.set_to_currency(*to);
        params.set_from_currency(*from);
        return params;
    }

private:
    [[nodiscard]] core::error::AppError invalid_request_data(std::string message) const
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INVALID_REQUEST,
            std::move(message)
        );
        error.set_kind_name("InvalidRequestData");
        return error;
    }

    ::app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old route call handed to the logic.

struct RetrieveForexInput
{
    ForexAuth auth{};
};

struct ConvertForexInput
{
    ForexAuth auth{};
    ::app::dto::currency::CurrencyConversionParams params{};
};

} // namespace app::handlers::currency
