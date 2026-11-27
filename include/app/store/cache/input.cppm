export module app_store:cache_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_context_core;
import app_http;

export namespace app::handlers::cache {

/**
 * @brief Parse/auth/path support for the cache handler.
 *
 * Kept free of business logic: auth is resolved from the shared registry and
 * the request is projected into an HTTP-free CallContext for the runtime.
 */
class CacheInputSupport
{
public:
    explicit CacheInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Upstream wraps this route in server_wrap(Flow::CacheInvalidate,
    // AdminApiAuth, LockAction::NotApplicable) (routes/cache.rs:20-28).
    // AdminApiAuth is an admin-api-key bearer check. The registry row
    // "invalidate" is the declarative local form: PLATFORM_ADMIN + ADMIN +
    // bearer requirement. NotApplicable means no lock is acquired here.
    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req) const
    {
        const auto rule = m_context.auth_rules().rule_for("invalidate");
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    /** @brief HTTP-free view of the request for the invalidate runtime. */
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
                const auto pair = tail.substr(0, amp == std::string_view::npos ? tail.size() : amp);
                const auto eq = pair.find('=');
                ctx.query[std::string{eq == std::string_view::npos ? pair : pair.substr(0, eq)}] =
                    eq == std::string_view::npos ? std::string{} : std::string{pair.substr(eq + 1)};
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
    ::app::context::AppContext& m_context;
};

/**
 * @brief Domain policy consumed by core::behavior::RequestSupport.
 *
 * Cache invalidation has one declarative rule ("invalidate"); the cache key is
 * a generic path segment, so the runtime reads it from CallContext::path_segments.
 */
class CachePolicy
{
public:
    explicit CachePolicy(CacheInputSupport& support) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view,
        const core::behavior::AuthSpec&
    ) const
    {
        return m_support.authorize(req);
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
    CacheInputSupport& m_support;
};

} // namespace app::handlers::cache
