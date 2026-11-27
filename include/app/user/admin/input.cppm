export module app_user:admin_input;

import std;
import interfaces;
import core_behavior;
import core_error;
import app_context;
import app_auth;
import app_http;
import app_dto;
import app_model;

export namespace app::handlers::admin {

/**
 * @brief Parse/auth/path/query support for the admin handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime. Every check mirrors the original
 * AdminRouteHandlers private helpers (authorize_request / path_part / path_has /
 * size_query), returning the error instead of writing it so the handler owns
 * the single response-writing path for every route.
 */
class AdminInputSupport
{
public:
    explicit AdminInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto context = ::app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(context, rule);
    }

    [[nodiscard]] bool path_has(interfaces::io::IRequest& req, std::string_view expected) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i{0}; i < segments.count(); ++i) {
            const auto part = segments.at(i);
            if (part.has_value() && *part == expected) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::size_t
    size_query(interfaces::io::IRequest& req, std::string_view key, std::size_t fallback) const
    {
        const auto raw = ::app::http::RequestQuery{}(req).get(key);
        if (!raw.has_value() || raw->empty()) {
            return fallback;
        }
        std::size_t value{fallback};
        const auto result = std::from_chars(raw->data(), raw->data() + raw->size(), value);
        return result.ec == std::errc{} ? value : fallback;
    }

private:
    ::app::context::AppContext& m_context;
};

/** @brief Request snapshot shared by every admin runtime method. */
[[nodiscard]] inline ::core::behavior::CallContext make_call_context(interfaces::io::IRequest& req)
{
    ::core::behavior::CallContext ctx{};
    const auto auth = ::app::auth::AuthContextBuilder{}(req);
    ctx.merchant_id = std::string{::app::http::merchant_id_from_header{}(req)};
    if (ctx.merchant_id.empty()) {
        ctx.merchant_id = auth.merchant_id();
    }
    ctx.org_id = std::string{::app::http::org_id_from_header{}(req)};
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
        ctx.path_segments.push_back(std::string{segments.at(i).value_or("")});
    }
    const auto raw_path = req.get_path();
    const auto question = raw_path.find('?');
    if (question != std::string_view::npos) {
        const std::string_view tail = raw_path.substr(question + 1);
        std::size_t start{0};
        while (start <= tail.size()) {
            const auto amp = tail.find('&', start);
            const auto end = amp == std::string_view::npos ? tail.size() : amp;
            const auto pair = tail.substr(start, end - start);
            const auto eq = pair.find('=');
            if (!pair.empty()) {
                const std::string key{eq == std::string_view::npos ? pair : pair.substr(0, eq)};
                const std::string value{
                    eq == std::string_view::npos ? std::string_view{} : pair.substr(eq + 1)
                };
                ctx.query.insert_or_assign(key, value);
            }
            if (amp == std::string_view::npos) {
                break;
            }
            start = amp + 1;
        }
    }
    constexpr std::string_view kHeaders[] = {
        "x-merchant-id", "x-org-id", "x-organization-id", "x-profile-id", "authorization",
        "x-client-secret", "x-api-key", "x-connected-merchant-id", "x-forwarded-for"
    };
    for (const auto name: kHeaders) {
        const auto value = req.find_header(name);
        if (!value.empty()) {
            ctx.headers.insert_or_assign(std::string{name}, std::string{value});
        }
    }
    for (const auto& [key, value]: auth.claims()) {
        ctx.claims.insert_or_assign(key, value);
    }
    ctx.claims.insert_or_assign("merchant_id", ctx.merchant_id);
    ctx.claims.insert_or_assign("org_id", ctx.org_id);
    ctx.claims.insert_or_assign("profile_id", ctx.profile_id);
    return ctx;
}

[[nodiscard]] inline std::string ctx_string(const ::core::behavior::CallContext& ctx, std::string_view key)
{
    const auto it = ctx.claims.find(std::string{key});
    return it == ctx.claims.end() ? std::string{} : it->second;
}

[[nodiscard]] inline bool ctx_bool(const ::core::behavior::CallContext& ctx, std::string_view key)
{
    return ctx_string(ctx, key) == "true";
}

[[nodiscard]] inline std::size_t ctx_size(const ::core::behavior::CallContext& ctx, std::string_view key)
{
    const auto raw = ctx_string(ctx, key);
    std::size_t value{};
    const auto result = std::from_chars(raw.data(), raw.data() + raw.size(), value);
    return result.ec == std::errc{} && result.ptr == raw.data() + raw.size() ? value : 0U;
}

/**
 * @brief Domain authorization policy for Admin.
 *
 * The route operation string selects the registered auth rule; the scope is
 * resolved from the request once, for every handler in the domain.
 */
class AdminPolicy
{
public:
    explicit AdminPolicy(AdminInputSupport& input) noexcept :
        m_input{input}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const ::core::behavior::AuthSpec&
    ) const
    {
        return m_input.authorize(req, operation);
    }

    [[nodiscard]] ::core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        ::core::behavior::Scope out{};
        out.merchant_id = std::string{::app::http::merchant_id_from_header{}(req)};
        if (out.merchant_id.empty()) {
            out.merchant_id = auth.merchant_id();
        }
        out.org_id = std::string{::app::http::org_id_from_header{}(req)};
        if (out.org_id.empty()) {
            out.org_id = auth.org_id();
        }
        out.profile_id = auth.profile_id();
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        if (connected.has_value()) {
            out.connected_merchant_id = std::string{*connected};
        }
        return out;
    }

    [[nodiscard]] ::core::behavior::CallContext
    context(interfaces::io::IRequest& req) const
    {
        return make_call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const ::core::behavior::Scope&) const
    {
        return std::nullopt;
    }

    /** @brief From-end segment index, matching the legacy path_part helper. */
    [[nodiscard]] static std::size_t path_index(interfaces::io::IRequest& req, std::size_t from_end)
    {
        const ::app::http::PathSegments segments{::app::http::RequestPath{}(req)};
        if (from_end == 0 || from_end > segments.count()) {
            return std::numeric_limits<std::size_t>::max();
        }
        return segments.count() - from_end;
    }

private:
    AdminInputSupport& m_input;
};

using AdminRequestSupport = ::core::behavior::RequestSupport<AdminPolicy>;

} // namespace app::handlers::admin
