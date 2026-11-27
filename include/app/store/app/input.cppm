export module app_store:app_input;

import std;
import interfaces;
import core_serde;
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
 * `app.rs` internal helpers; no HTTP route is mounted (see handler.cppm).
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

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope&) const noexcept
    {
        return std::nullopt;
    }

private:
    AppInputSupport& m_support;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly the request-derived data the old inline core
// block consumed.

struct PaymentCallbackConfigInput
{};

struct CreateEmailClientInput
{
    serde::Value payload{};
    std::optional<std::string> proxy_url{};
};

struct WithStorageInput
{
    std::string storage_impl{"postgresql"};
    std::string service_name{"default"};
};

struct GetStoreInterfaceInput
{
    std::string storage_impl{"postgresql"};
    std::string event_handler{"logs"};
    std::string org_id{};
    std::string merchant_id{};
};

struct NewInput
{
    std::string service_name{"default"};
};

} // namespace app::handlers::app
