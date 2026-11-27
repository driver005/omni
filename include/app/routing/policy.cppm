export module app_routing:policy;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_auth;
import app_http;

export namespace app::routing::policy {

/**
 * @brief Build the runtime call context from request headers, path, query and auth claims.
 *
 * Query keys/values are plus/percent-decoded exactly as app_http::RequestQuery decoded
 * them before the CallContext rewrite. Canonical headers are inserted under lowercase
 * names through find_header() so no request enumeration is required.
 */
[[nodiscard]] inline ::core::behavior::CallContext context_of(::interfaces::io::IRequest& req)
{
    ::core::behavior::CallContext ctx{};

    const auto auth = ::app::auth::AuthContextBuilder{}(req);
    ctx.merchant_id = auth.merchant_id();
    ctx.profile_id = auth.profile_id();
    ctx.org_id = auth.org_id();
    if (const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId)) {
        ctx.connected_merchant_id = std::string{*connected};
    }
    if (const auto secret = auth.claim(::app::auth::kClaimClientSecret)) {
        ctx.client_secret = std::string{*secret};
    }
    for (const auto& [name, value]: auth.claims()) {
        ctx.claims.emplace(name, value);
    }

    const auto segments = ::app::http::RequestPath{}(req);
    ctx.path_segments.reserve(segments.count());
    for (std::size_t i{0}; i < segments.count(); ++i) {
        ctx.path_segments.emplace_back(std::string{segments.at(i).value_or("")});
    }

    // RequestQuery exposes only get(); walk the raw path for the ordered key/value pairs.
    const auto raw = req.get_path();
    const auto question = raw.find('?');
    if (question != std::string_view::npos) {
        std::size_t start = question + 1;
        while (start < raw.size()) {
            const auto amp = raw.find('&', start);
            const auto end = amp == std::string_view::npos ? raw.size() : amp;
            const auto pair = raw.substr(start, end - start);
            const auto eq = pair.find('=');
            const auto encoded_key = eq == std::string_view::npos ? pair : pair.substr(0, eq);
            const auto encoded_value =
                eq == std::string_view::npos ? std::string_view{} : pair.substr(eq + 1);
            auto decode = [](std::string_view value)
            {
                std::string out{};
                out.reserve(value.size());
                for (std::size_t i{0}; i < value.size(); ++i) {
                    if (value[i] == '+') {
                        out.push_back(' ');
                    } else if (value[i] == '%' && i + 2U < value.size()) {
                        const auto hex = [](char ch) -> int
                        {
                            if (ch >= '0' && ch <= '9') {
                                return ch - '0';
                            }
                            if (ch >= 'a' && ch <= 'f') {
                                return ch - 'a' + 10;
                            }
                            if (ch >= 'A' && ch <= 'F') {
                                return ch - 'A' + 10;
                            }
                            return -1;
                        };
                        const int high = hex(value[i + 1]);
                        const int low = hex(value[i + 2]);
                        if (high >= 0 && low >= 0) {
                            out.push_back(static_cast<char>((high << 4) | low));
                            i += 2;
                            continue;
                        }
                        out.push_back(value[i]);
                    } else {
                        out.push_back(value[i]);
                    }
                }
                return out;
            };
            const auto key = decode(encoded_key);
            ctx.query[key] = decode(encoded_value);
            if (amp == std::string_view::npos) {
                break;
            }
            start = amp + 1;
        }
    }
    for (const auto name: {
             "x-merchant-id",
             "x-org-id",
             "x-profile-id",
             "x-workspace",
             "x-provider-merchant-id",
             "x-connected-merchant-id",
             "x-client-secret",
             "x-publishable-key",
             "x-api-key",
             "authorization",
             "content-type",
         }) {
        const auto value = req.find_header(name);
        if (!value.empty()) {
            ctx.headers[name] = std::string{value};
        }
    }
    return ctx;
}

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
