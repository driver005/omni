export module app_store:app_input;

import std;
import interfaces;
import core_error;
import core_behavior;
import app_context;
import app_context_core;
import app_http;

export namespace app::handlers::app {

/**
 * @brief Parse/auth/path/query support for the app handlers.
 *
 * These operations are retained as in-process LOCAL HELPERS mirroring upstream
 * `app.rs` internal helpers; no HTTP route is mounted (see handler.cppm). The
 * support also assembles the HTTP-free CallContext consumed by the runtime.
 */
class AppInputSupport
{
public:
    explicit AppInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError>
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        return m_context.authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] std::string org_id(interfaces::io::IRequest& req) const
    {
        return ::app::http::org_id_from_header{}(req);
    }

    [[nodiscard]] std::string merchant_id(interfaces::io::IRequest& req) const
    {
        return ::app::http::merchant_id_from_header{}(req);
    }

    /** @brief HTTP-free view of the request for the app sub-runtimes. */
    [[nodiscard]] core::behavior::CallContext call_context(interfaces::io::IRequest& req) const
    {
        core::behavior::CallContext ctx{};
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        ctx.merchant_id = merchant_id(req);
        if (ctx.merchant_id.empty()) {
            ctx.merchant_id = auth.merchant_id();
        }
        ctx.org_id = org_id(req);
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
 * Owns only the app domain's auth rule resolution and request scope. Auth still
 * resolves through the shared registry by operation; generic path/query/header
 * extraction lives in RequestSupport.
 */
class AppPolicy
{
public:
    explicit AppPolicy(AppInputSupport& support) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)spec; // app helpers resolve their rule from the registry by operation.
        return m_support.authorize(req, operation);
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        core::behavior::Scope result;
        result.org_id = m_support.org_id(req);
        result.merchant_id = m_support.merchant_id(req);
        return result;
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
    AppInputSupport& m_support;
};

} // namespace app::handlers::app
