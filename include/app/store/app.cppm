export module app_store:app_handler;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :app_input;
import :app_result;
import :app_runtime;

export namespace app::handlers::app {

/**
 * @brief Upstream `crates/router/src/routes/app.rs` defines NO HTTP scope of its own. The file
 * contains only state types (`ReqState`/`SessionState`/`AppState` plus their `*Info` traits),
 * internal startup helpers (`payment_callback_config`, `create_email_client`, `with_storage`,
 * `get_store_interface`, `new`, `get_session_state`, `process_env_mappings`), and ~50
 * `impl Xxx::server` scope builders that register SIBLING domains' handlers
 * (app.rs:810-3638; 553 `.to(...)` registrations, all resolving to handlers imported from
 * other route modules). Strict upstream parity therefore requires ZERO `/app` HTTP routes in
 * this module: the five former synthetic diagnostic routes had no upstream counterpart and
 * have been removed from `install()`.
 *
 * The operations below are retained as in-process LOCAL HELPERS mirroring those upstream
 * internal helpers; they are intentionally NOT mounted as HTTP routes because no upstream wire
 * route exists (evidence app.rs:207-212, 477-495, 497-643, 648-727, 729-743). All genuine
 * per-scope HTTP routes are hosted by their own handlers and tracked in their own parity
 * reports; this module's route surface is empty by design.
 */
class AppRouteHandlers
{
public:
    explicit AppRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context}
    {
    }

    void handle_payment_callback_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream SessionState::payment_callback_config() is an internal OnceCell load
        // (app.rs:207-212) with no HTTP auth. The registry row is NO_AUTH.
        const auto auth = m_input_support.authorize(req, "payment_callback_config");
        if (auth) {
            const auto& error = *auth;
            error.send(res, send);
            return;
        }
        PaymentCallbackConfigInput input{};
        auto result = m_runtime.payment_callback_config(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_create_email_client(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream create_email_client() is a free helper (app.rs:477-495), no HTTP auth.
        const auto auth = m_input_support.authorize(req, "create_email_client");
        if (auth) {
            const auto& error = *auth;
            error.send(res, send);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        const auto proxy_url = m_input_support.query(req, "proxy_url");
        CreateEmailClientInput input{*parsed, proxy_url};
        auto result = m_runtime.create_email_client(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_with_storage(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream AppState::with_storage() is an async constructor (app.rs:497-643), no auth.
        const auto auth = m_input_support.authorize(req, "with_storage");
        if (auth) {
            const auto& error = *auth;
            error.send(res, send);
            return;
        }

        const auto storage_impl = m_input_support.query(req, "storage_impl").value_or("postgresql");
        const auto service_name = m_input_support.query(req, "service_name").value_or("default");
        WithStorageInput input{storage_impl, service_name};
        auto result = m_runtime.with_storage(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_get_store_interface(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream AppState::get_store_interface() is an async constructor (app.rs:648-727).
        const auto auth = m_input_support.authorize(req, "get_store_interface");
        if (auth) {
            const auto& error = *auth;
            error.send(res, send);
            return;
        }

        const auto storage_impl = m_input_support.query(req, "storage_impl").value_or("postgresql");
        const auto event_handler = m_input_support.query(req, "event_handler").value_or("logs");
        GetStoreInterfaceInput input{
            storage_impl,
            event_handler,
            m_input_support.org_id(req),
            m_input_support.merchant_id(req)
        };
        auto result = m_runtime.get_store_interface(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_new(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream AppState::new() delegates to with_storage(Postgresql) (app.rs:729-743).
        const auto auth = m_input_support.authorize(req, "new");
        if (auth) {
            const auto& error = *auth;
            error.send(res, send);
            return;
        }

        const auto service_name = m_input_support.query(req, "service_name").value_or("default");
        NewInput input{service_name};
        auto result = m_runtime.app_new(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // No HTTP routes are mounted. Upstream `app.rs` defines no `/app` scope: its ~50 `server()`
    // builders (app.rs:810-3638) register sibling domains' handlers, which are hosted by their
    // own C++ modules and installed individually by routes.cppm:16-80. The five former synthetic
    // diagnostic routes had no upstream counterpart and are removed for strict parity. Kept as
    // a no-op because routes.cppm:18 calls `handlers.app().install(router)`.
    void install_routes(core::router::RouterContext<>& router)
    {
        (void)router;
    }

private:
    AppRuntime m_runtime;
    AppInputSupport m_input_support;
};

} // namespace app::handlers::app
