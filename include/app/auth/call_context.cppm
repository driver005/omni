export module app_auth:call_context;

import std;
import interfaces;
import core_behavior;
import app_http;
import app_context_core;

export namespace app::handlers::detail {

/**
 * @brief Build the runtime-facing CallContext from a request.
 *
 * This is the only place in the auth area that reads HTTP request facts.
 * Runtimes receive the resulting plain value and never see IRequest.
 */
[[nodiscard]] inline core::behavior::CallContext
make_call_context(interfaces::io::IRequest& req, std::string_view operation = {})
{
    core::behavior::CallContext ctx{};
    ctx.operation = std::string{operation};

    const auto auth = ::app::auth::AuthContextBuilder{}(req);
    ctx.merchant_id = auth.merchant_id().empty() ? ::app::http::merchant_id_from_header{}(req)
                                                 : std::string{auth.merchant_id()};
    ctx.profile_id = auth.profile_id();
    ctx.org_id = auth.org_id();
    if (const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId)) {
        ctx.connected_merchant_id = *connected;
    }
    if (const auto secret = auth.claim(::app::auth::kClaimClientSecret)) {
        ctx.client_secret = *secret;
    }
    for (const auto& [key, value]: auth.claims()) {
        ctx.claims.insert_or_assign(key, value);
    }

    // Identity headers not captured by AuthContextBuilder.
    for (const auto name: {"authorization", "content-type", "x-user-id", "x-user", "x-user-name"}) {
        const auto value = req.find_header(name);
        if (!value.empty()) {
            ctx.headers.insert_or_assign(std::string{name}, std::string{value});
        }
    }

    const auto segments = ::app::http::RequestPath{}(req);
    ctx.path_segments.reserve(segments.count());
    for (std::size_t i{0}; i < segments.count(); ++i) {
        ctx.path_segments.emplace_back(segments.at(i).value_or(""));
    }

    const auto raw = req.get_path();
    const auto question = raw.find('?');
    if (question != std::string_view::npos) {
        std::size_t start{question + 1};
        while (start < raw.size()) {
            const auto amp = raw.find('&', start);
            const auto end = amp == std::string_view::npos ? raw.size() : amp;
            const auto pair = raw.substr(start, end - start);
            const auto equal = pair.find('=');
            const auto key = equal == std::string_view::npos ? pair : pair.substr(0, equal);
            const auto value =
                equal == std::string_view::npos ? std::string_view{} : pair.substr(equal + 1);
            if (!key.empty()) {
                ctx.query.insert_or_assign(std::string{key}, std::string{value});
            }
            if (amp == std::string_view::npos) {
                break;
            }
            start = amp + 1;
        }
    }
    return ctx;
}

/** @brief Last non-empty path segment, or an empty string. */
[[nodiscard]] inline const std::string& last_path_segment(const core::behavior::CallContext& ctx)
{
    static const std::string empty{};
    return ctx.path_segments.empty() ? empty : ctx.path_segments.back();
}

/** @brief Path segment at @p index, or an empty string when absent. */
[[nodiscard]] inline const std::string&
path_segment(const core::behavior::CallContext& ctx, std::size_t index)
{
    static const std::string empty{};
    return index < ctx.path_segments.size() ? ctx.path_segments[index] : empty;
}

/** @brief Claim value, or an empty string when the claim is absent. */
[[nodiscard]] inline const std::string&
claim_value(const core::behavior::CallContext& ctx, std::string_view key)
{
    static const std::string empty{};
    const auto it = ctx.claims.find(std::string{key});
    return it == ctx.claims.end() ? empty : it->second;
}

/** @brief Raw request body carried through the context for verbatim field echo. */
[[nodiscard]] inline std::string_view request_body(const core::behavior::CallContext& ctx)
{
    return claim_value(ctx, "x-request-body");
}

/** @brief True when the authenticated flow is the SDK/client flow. */
[[nodiscard]] inline bool is_client_flow(const core::behavior::CallContext& ctx)
{
    return claim_value(ctx, "x-auth-flow") == "client";
}

/** @brief Connected merchant scope when present, otherwise the plain merchant id. */
[[nodiscard]] inline const std::string&
effective_merchant_id(const core::behavior::CallContext& ctx)
{
    return ctx.connected_merchant_id.empty() ? ctx.merchant_id : ctx.connected_merchant_id;
}

/** @brief Bearer credential from the authorization header, case-insensitive scheme. */
[[nodiscard]] inline std::string bearer_token(const core::behavior::CallContext& ctx)
{
    const auto it = ctx.headers.find("authorization");
    if (it == ctx.headers.end()) {
        return {};
    }
    constexpr std::string_view scheme{"Bearer "};
    if (it->second.size() <= scheme.size()) {
        return {};
    }
    for (std::size_t i{0}; i < scheme.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(it->second[i])) !=
            std::tolower(static_cast<unsigned char>(scheme[i]))) {
            return {};
        }
    }
    return it->second.substr(scheme.size());
}

/** @brief Segment following @p marker, falling back to the final segment. */
[[nodiscard]] inline const std::string&
path_segment_after(const core::behavior::CallContext& ctx, std::string_view marker)
{
    for (std::size_t i{0}; i + 1 < ctx.path_segments.size(); ++i) {
        if (ctx.path_segments[i] == marker) {
            return ctx.path_segments[i + 1];
        }
    }
    return last_path_segment(ctx);
}

} // namespace app::handlers::detail
