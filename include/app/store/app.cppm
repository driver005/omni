export module app_store:app_handler;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
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
        m_input_support{context},
        m_policy{m_input_support},
        m_request_support{m_policy}
    {
    }

    void handle_payment_callback_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Upstream SessionState::payment_callback_config() is an internal OnceCell load
        // (app.rs:207-212) with no HTTP auth. The registry row is NO_AUTH.
        const auto auth = m_request_support.authorize(req, "payment_callback_config", {});
        if (auth) {
            const auto& error = *auth;
            error.send(sink);
            return;
        }
        m_runtime.payment_callback_config(m_request_support.context(req)).handle(sink);
    }

    void handle_create_email_client(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Upstream create_email_client() is a free helper (app.rs:477-495), no HTTP auth.
        const auto auth = m_request_support.authorize(req, "create_email_client", {});
        if (auth) {
            const auto& error = *auth;
            error.send(sink);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(sink);
            return;
        }
        m_runtime.create_email_client(*parsed, m_request_support.context(req)).handle(sink);
    }

    void handle_with_storage(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Upstream AppState::with_storage() is an async constructor (app.rs:497-643), no auth.
        const auto auth = m_request_support.authorize(req, "with_storage", {});
        if (auth) {
            const auto& error = *auth;
            error.send(sink);
            return;
        }

        m_runtime.with_storage(m_request_support.context(req)).handle(sink);
    }

    void handle_get_store_interface(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Upstream AppState::get_store_interface() is an async constructor (app.rs:648-727).
        const auto auth = m_request_support.authorize(req, "get_store_interface", {});
        if (auth) {
            const auto& error = *auth;
            error.send(sink);
            return;
        }

        m_runtime.get_store_interface(m_request_support.context(req)).handle(sink);
    }

    void handle_new(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Upstream AppState::new() delegates to with_storage(Postgresql) (app.rs:729-743).
        const auto auth = m_request_support.authorize(req, "new", {});
        if (auth) {
            const auto& error = *auth;
            error.send(sink);
            return;
        }

        m_runtime.app_new(m_request_support.context(req)).handle(sink);
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
    AppPolicy m_policy;
    core::behavior::RequestSupport<AppPolicy> m_request_support;
};

} // namespace app::handlers::app
