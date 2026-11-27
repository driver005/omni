export module app_store:health_handlers_v2;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_context;
import core_error;

import :health_input;
import :health_runtime;

export namespace app::handlers::health {

/**
 * v2 surface: upstream `web::scope("/v2/health")` (app.rs:822-829). Upstream
 * registers the same two operations under both scopes, so this partition
 * carries its own copies of the thin adapters; the runtime call and all
 * serialization are identical to v1. The facade owns the single runtime and
 * input helper and shares them by reference.
 */
class HealthHandlersV2
{
public:
    HealthHandlersV2(
        ::app::context::AppContext& context,
        HealthRuntime& runtime,
        core::behavior::RequestSupport<HealthPolicy>& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_request_support{request_support}
    {
    }

    void handle_health(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // 1. request -> DTO + CallContext. Upstream routes::health::health() has no server_wrap
        // and no auth at all; the registry resolves "health" to NO_AUTH, so this
        // authorize_route call is an exact no-op (kept for the error path).
        if (const auto auth_error = m_request_support.authorize(req, "health", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        // HttpResponse::Ok().body("health is good") is published through the
        // typed Outcome handle.
        m_runtime.health(m_request_support.context(req)).handle(sink);
    }

    void handle_deep_health_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // 1. request -> DTO + CallContext. Upstream uses &auth::NoAuth via server_wrap
        // (routes/health.rs:38); "deep_health_check" resolves to NO_AUTH.
        if (const auto auth_error = m_request_support.authorize(req, "deep_health_check", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        // RouterHealthCheckResponse / HealthCheckError are published by the
        // typed Outcome handle; HE_00 stays in AppError::kind_name.
        m_runtime.deep_health_check(m_request_support.context(req)).handle(sink);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        Router{router, "v2/health"}
            .add_route(
                Route{""}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_health(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch health v2")
            )
            .add_route(
                Route{"ready"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_deep_health_check(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch deep_health_check v2")
            );
    }

private:
    ::app::context::AppContext& m_context;
    HealthRuntime& m_runtime;
    core::behavior::RequestSupport<HealthPolicy>& m_request_support;
};

} // namespace app::handlers::health
