export module app_handlers_revenue_recovery:input;

import std;
import interfaces;
import core_error;
import app_context;
import app_auth;
import app_http;

export namespace app::handlers::revenue_recovery {

/**
 * @brief Parse/auth/path support for the revenue-recovery cancel handler.
 *
 * Kept free of business logic: it only authenticates the request (V2ApiKeyAuth with
 * both scope-override flags false, plus check_merchant_access) and extracts the
 * merchant_reference_id path segment. Errors are returned, never written to a response.
 */
class RevenueRecoveryInputSupport
{
public:
    explicit RevenueRecoveryInputSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Upstream V2ApiKeyAuth { allow_connected_scope_operation: false,
    // allow_platform_self_operation: false } (routes/revenue_recovery.rs:32-35).
    // The registry row carries the permission; the scheme is narrowed locally to
    // V2_API_KEY so authorize_route verifies the api-key credential V2ApiKeyAuth reads
    // (services/authentication.rs:2978-2991, AuthContextBuilder parses "api-key=" at
    // context.cppm:206-207). Both override flags are false.
    //
    // Upstream check_merchant_access with both flags false
    // (services/authentication.rs:6487-6551): a connected-scope operation maps to
    // ConnectedAccountAuthNotSupported (BadRequest, "API does not support connected
    // account operation", :720-721) and a platform self-operation maps to
    // PlatformAccountAuthNotSupported (BadRequest, "API does not support platform
    // account operation", :702-703). AuthContext has no merchant account type, so the
    // local signals are the connected-merchant and internal-api-key claims.
    [[nodiscard]] std::expected<app::auth::AuthContext, core::error::AppError>
    authorize_cancel(interfaces::io::IRequest& req) const
    {
        const auto base_rule = m_context.auth_rules().rule_for("revenue_recovery_cancel");
        const app::auth::RouteAuthRule
            rule{app::auth::AuthScheme::V2_API_KEY, base_rule.permission(), false, false};
        const auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        if (const auto denied = m_context.authorizer().authorize_route(auth_ctx, rule)) {
            return std::unexpected{*denied};
        }

        const auto connected = auth_ctx.claim(app::auth::kClaimConnectedMerchantId);
        if (connected.has_value() && !connected->empty()) {
            return std::unexpected{
                make_bad_request("API does not support connected account operation")
            };
        }
        const auto internal_key = auth_ctx.claim(app::auth::kClaimInternalApiKey);
        if (internal_key.has_value() && !internal_key->empty()) {
            return std::unexpected{
                make_bad_request("API does not support platform account operation")
            };
        }
        return auth_ctx;
    }

    // Upstream Path<PaymentReferenceId> from the route segment (routes/revenue_recovery.rs:14,
    // :17; app.rs:3550-3551). The cancel route is /v2/recovery/{merchant_reference_id}/cancel,
    // so the segment immediately before "cancel" is the merchant_reference_id. No body is
    // read (upstream takes none).
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    merchant_reference_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        std::string merchant_reference_id{};
        for (std::size_t i{1}; i < segments.count(); ++i) {
            if (segments.at(i).value_or("") == "cancel") {
                merchant_reference_id = std::string{segments.at(i - 1).value_or("")};
                break;
            }
        }
        if (merchant_reference_id.empty()) {
            return std::unexpected{
                make_bad_request("recovery cancel path requires a merchant_reference_id")
            };
        }
        return merchant_reference_id;
    }

private:
    [[nodiscard]] core::error::AppError make_bad_request(std::string message) const
    {
        return core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INVALID_REQUEST,
            std::move(message)
        );
    }

    app::context::AppContext& m_context;
};

// Everything the old cancel_revenue_recovery_core call received: the path id and
// the authenticated context (carrier of auth.platform and auth.profile).
struct RevenueRecoveryCancelInput
{
    std::string merchant_reference_id{};
    app::auth::AuthContext auth{};
};

} // namespace app::handlers::revenue_recovery
