export module app_handlers_app:input;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_auth;
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

    [[nodiscard]] std::optional<std::string>
    query(interfaces::io::IRequest& req, std::string_view name) const
    {
        return ::app::http::RequestQuery{}(req).get(name);
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
