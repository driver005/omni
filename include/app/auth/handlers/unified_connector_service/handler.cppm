export module app_auth:unified_connector_service_handler;

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
import app_metrics;
import app_services;

import :unified_connector_service_input;
import :unified_connector_service_result;
import :unified_connector_service_runtime;

export namespace app::handlers::unified_connector_service {

// Upstream gates the whole scope behind `#[cfg(feature = "olap")]`
// (routes/app.rs:2444). The project's local compile-time-feature convention is
// a namespace-scoped `constexpr bool` (health.cppm:27); this is that substitute.
inline constexpr bool kOlap = true;

class UnifiedConnectorServiceRouteHandlers
{
public:
    // ───────────────────────── Routes ─────────────────────────
    // Upstream kill_switch::trip_status (kill_switch.rs:580-671).
    void handle_kill_switch_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // 1. request -> Input (body-less: admin auth + final path segment only).
        if (const auto auth = m_input_support.authorize_admin(req, "kill_switch_status")) {
            const auto& error = *auth;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto resolved = m_input_support.resolve_scope(req);
        if (resolved.listed_key.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "scope is required"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        KillSwitchStatusInput input{resolved};

        // 2. Input -> Result.
        auto result = m_runtime.status(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // 3. Result -> response (direct writes).
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // Upstream kill_switch::reset (kill_switch.rs:543-577).
    void handle_reset_kill_switch(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // 1. request -> Input (body-less: admin auth + final path segment only).
        if (const auto auth = m_input_support.authorize_admin(req, "reset_kill_switch")) {
            const auto& error = *auth;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const auto resolved = m_input_support.resolve_scope(req);
        if (resolved.listed_key.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "scope is required"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        ResetKillSwitchInput input{resolved};

        // 2. Input -> Result.
        auto result = m_runtime.reset(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // 3. Result -> response (direct writes). Upstream StatusOk is bodyless;
        // the typed empty-object document serializes as "{}".
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        // routes/app.rs:2444-2454: the entire scope is gated by feature "olap".
        if constexpr (!kOlap) {
            return;
        }

        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "unified-connector-service"}
            .add_router(
                Router{router, "kill-switch"}.add_route(
                    Route{":scope"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_kill_switch_status(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch kill_switch_status")
                )
            )
            .add_router(
                Router{router, "kill-switch"}.add_route(
                    Route{":scope"}
                        .delt(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_reset_kill_switch(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch reset_kill_switch")
                )
            );
    }

    explicit UnifiedConnectorServiceRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context}
    {
    }

private:
    UnifiedConnectorServiceRuntime m_runtime;
    UnifiedConnectorServiceInputSupport m_input_support;
};

} // namespace app::handlers::unified_connector_service
