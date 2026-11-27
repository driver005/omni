export module app_store:currency_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_context_core;
import app_http;
import app_dto;

export namespace app::handlers::currency {

/**
 * @brief Query/auth support for the currency handlers.
 *
 * Kept free of business logic: it parses the conversion query into the typed
 * CurrencyConversionParams DTO and projects the request into an HTTP-free
 * CallContext. Auth failures stay in the handler's single response-writing path.
 */
class CurrencyInputSupport
{
public:
    explicit CurrencyInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ── server_wrap auth_type(ApiKeyAuth, DashboardNoPermissionAuth) OR-path ─
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_forex(interfaces::io::IRequest& req) const
    {
        const auto context = ::app::auth::AuthContextBuilder{}(req);
        // HeaderAuth(ApiKeyAuth) first (registry: retrieve_forex = API_KEY).
        auto denied = m_context.authorizer().authorize_route(
            context,
            m_context.auth_rules().rule_for("retrieve_forex")
        );
        if (!denied) {
            return std::nullopt;
        }
        // DashboardNoPermissionAuth second (registry: convert_forex = JWT).
        denied = m_context.authorizer().authorize_route(
            context,
            m_context.auth_rules().rule_for("convert_forex")
        );
        return denied;
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

    /** @brief HTTP-free view of the request for the forex sub-runtimes. */
    [[nodiscard]] core::behavior::CallContext call_context(interfaces::io::IRequest& req) const
    {
        core::behavior::CallContext ctx{};
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        ctx.merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (ctx.merchant_id.empty()) {
            ctx.merchant_id = auth.merchant_id();
        }
        ctx.org_id = ::app::http::org_id_from_header{}(req);
        if (ctx.org_id.empty()) {
            ctx.org_id = auth.org_id();
        }
        ctx.profile_id = auth.profile_id();
        if (const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId)) {
            ctx.connected_merchant_id = std::string{*connected};
        }
        if (const auto secret = auth.claim(::app::auth::kClaimClientSecret)) {
            ctx.client_secret = std::string{*secret};
        }
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i{0}; i < segments.count(); ++i) {
            if (const auto segment = segments.at(i)) {
                ctx.path_segments.emplace_back(*segment);
            }
        }
        const std::string_view raw = req.get_path();
        const auto question = raw.find('?');
        if (question != std::string_view::npos) {
            auto tail = raw.substr(question + 1);
            while (!tail.empty()) {
                const auto amp = tail.find('&');
                const auto pair =
                    tail.substr(0, amp == std::string_view::npos ? tail.size() : amp);
                const auto eq = pair.find('=');
                ctx.query[std::string{
                    eq == std::string_view::npos ? pair : pair.substr(0, eq)
                }] = eq == std::string_view::npos ? std::string{} : std::string{pair.substr(eq + 1)};
                if (amp == std::string_view::npos) {
                    break;
                }
                tail = tail.substr(amp + 1);
            }
        }
        for (const auto& [key, value]: auth.claims()) {
            ctx.claims[key] = value;
        }
        return ctx;
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

/**
 * @brief Domain policy consumed by core::behavior::RequestSupport.
 *
 * The forex OR-path (API key first, dashboard JWT second) is the genuine
 * currency auth rule and stays in CurrencyInputSupport; this policy adapts it
 * to the generic authorize/scope/context/scope_error shape.
 */
class CurrencyPolicy
{
public:
    explicit CurrencyPolicy(CurrencyInputSupport& support) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view,
        const core::behavior::AuthSpec&
    ) const
    {
        return m_support.authorize_forex(req);
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest&) const noexcept
    {
        return {};
    }

    [[nodiscard]] core::behavior::CallContext context(interfaces::io::IRequest& req) const
    {
        return m_support.call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope&) const noexcept
    {
        return std::nullopt;
    }

private:
    CurrencyInputSupport& m_support;
};

} // namespace app::handlers::currency
