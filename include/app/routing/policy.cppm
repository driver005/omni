export module app_routing:policy;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_auth;
import app_http;

export namespace app::routing::policy {

/** Default request scope shared by the routing-area domain policies. */
[[nodiscard]] inline ::core::behavior::Scope scope_of(::interfaces::io::IRequest& req)
{
    const auto auth = ::app::auth::AuthContextBuilder{}(req);
    ::core::behavior::Scope scope{};
    scope.merchant_id = auth.merchant_id();
    scope.profile_id = auth.profile_id();
    scope.org_id = auth.org_id();
    if (const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId)) {
        scope.connected_merchant_id = std::string{*connected};
    }
    return scope;
}

/** Index of the final non-empty path segment, for RequestSupport::path. */
[[nodiscard]] inline std::size_t last_path_index(::interfaces::io::IRequest& req)
{
    const auto count = ::app::http::RequestPath{}(req).count();
    return count == 0U ? 0U : count - 1U;
}

/** Default scope validation; domains with stricter rules override this in their policy. */
[[nodiscard]] inline std::optional<::core::error::AppError>
scope_error(const ::core::behavior::Scope& scope)
{
    if (scope.merchant_id.empty()) {
        return ::core::error::AppErrorBuilder{}(
            ::core::error::AppErrorKind::INVALID_REQUEST,
            "missing merchant id"
        );
    }
    return std::nullopt;
}

} // namespace app::routing::policy
