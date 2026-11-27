export module app_user:user_input;

import std;
import interfaces;
import core_behavior;
import core_error;
import app_context;
import app_dto;
import app_model;
import app_auth;
import app_http;

export namespace app::handlers::user {

// Plain request-derived values consumed by the route runtimes. No HTTP response types.
/** @brief Request snapshot shared by every user runtime method. */
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
    if (const auto c = auth.claim(::app::auth::kClaimConnectedMerchantId)) {
        ctx.connected_merchant_id = std::string{*c};
    }
    if (const auto s = auth.claim(::app::auth::kClaimClientSecret)) {
        ctx.client_secret = std::string{*s};
    }
    const auto segments = ::app::http::RequestPath{}(req);
    for (std::size_t i{0}; i < segments.count(); ++i) {
        ctx.path_segments.push_back(std::string{segments.at(i).value_or("")});
    }
    const auto raw = req.get_path();
    if (const auto q = raw.find('?'); q != std::string_view::npos) {
        const std::string_view tail = raw.substr(q + 1);
        std::size_t start{0};
        while (start <= tail.size()) {
            const auto amp = tail.find('&', start);
            const auto end = amp == std::string_view::npos ? tail.size() : amp;
            const auto pair = tail.substr(start, end - start);
            const auto eq = pair.find('=');
            if (!pair.empty()) {
                ctx.query.insert_or_assign(
                    std::string{eq == std::string_view::npos ? pair : pair.substr(0, eq)},
                    std::string{
                        eq == std::string_view::npos ? std::string_view{} : pair.substr(eq + 1)
                    }
                );
            }
            if (amp == std::string_view::npos) {
                break;
            }
            start = amp + 1;
        }
    }
    constexpr std::string_view kHeaders[] = {
        "x-merchant-id",
        "x-org-id",
        "x-organization-id",
        "x-profile-id",
        "authorization",
        "x-client-secret",
        "x-api-key",
        "x-connected-merchant-id",
        "x-forwarded-for"
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
    ctx.claims.insert_or_assign("token", auth.token().value());
    ctx.claims.insert_or_assign("client_ip", std::string{req.find_header("x-forwarded-for")});
    ctx.claims.insert_or_assign("body", req.body_as_string());
    return ctx;
}

[[nodiscard]] inline std::string
ctx_string(const ::core::behavior::CallContext& ctx, std::string_view key)
{
    const auto it = ctx.claims.find(std::string{key});
    return it == ctx.claims.end() ? std::string{} : it->second;
}

class CtxQuery
{
public:
    explicit CtxQuery(const ::core::behavior::CallContext& ctx) noexcept :
        m_ctx{ctx}
    {
    }

    [[nodiscard]] std::optional<std::string> get(std::string_view key) const
    {
        const auto it = m_ctx.query.find(std::string{key});
        return it == m_ctx.query.end() ? std::nullopt : std::optional<std::string>{it->second};
    }

private:
    const ::core::behavior::CallContext& m_ctx;
};

[[nodiscard]] inline CtxQuery ctx_query(const ::core::behavior::CallContext& ctx)
{
    return CtxQuery{ctx};
}

[[nodiscard]] inline ::app::auth::BearerToken ctx_token(const ::core::behavior::CallContext& ctx)
{
    return ::app::auth::BearerToken{ctx_string(ctx, "token")};
}

[[nodiscard]] inline ::app::auth::AuthContext ctx_auth(const ::core::behavior::CallContext& ctx)
{
    return ::app::auth::AuthContext{
        ctx_token(ctx),
        ctx.merchant_id,
        ctx.org_id,
        ctx.profile_id,
        std::map<std::string, std::string>{ctx.claims.begin(), ctx.claims.end()}
    };
}

[[nodiscard]] inline std::string ctx_path_string(const ::core::behavior::CallContext& ctx)
{
    std::string out{};
    for (const auto& segment: ctx.path_segments) {
        out.push_back('/');
        out += segment;
    }
    return out;
}

/** @brief Parse/auth/header/path/query support shared by all user handlers. */
class UserInputSupport
{
public:
    explicit UserInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Auth denial carries the upstream wire code in kind_name and the exact message;
    // the handler renders the nested ApiErrorResponse body from those fields.
    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        (void)operation;
        const auto auth_ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context.auth_rules().rule_for(std::string{operation});
        if (!m_context.authorizer().authorize_route(auth_ctx, rule)) {
            return std::nullopt;
        }
        if (rule.scheme() == ::app::auth::AuthScheme::PLATFORM_ADMIN) {
            auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "API key not provided or invalid API key used"
            );
            error.set_kind_name("IR_01");
            return error;
        }
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::UNAUTHORIZED,
            "Access forbidden, invalid JWT token was used"
        );
        error.set_kind_name("IR_17");
        return error;
    }

private:
    ::app::context::AppContext& m_context;
};

// One plain input per route: the parsed request plus all request-derived values.

/**
 * @brief Domain authorization policy for User.
 *
 * The route operation string selects the registered auth rule; the scope is
 * resolved from the request once, for every handler in the domain.
 */
class UserPolicy
{
public:
    explicit UserPolicy(UserInputSupport& input) noexcept :
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

    [[nodiscard]] ::core::behavior::CallContext context(interfaces::io::IRequest& req) const
    {
        return make_call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const ::core::behavior::Scope&) const
    {
        return std::nullopt;
    }


private:
    UserInputSupport& m_input;
};

using UserRequestSupport = ::core::behavior::RequestSupport<UserPolicy>;

} // namespace app::handlers::user
