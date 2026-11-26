export module app_store:health_handlers_v2;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_context;
import core_error;

import :health_input;
import :health_result;
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
        HealthInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    void handle_health(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // 1. request -> Input. Upstream routes::health::health() has no server_wrap
        // and no auth at all; the registry resolves "health" to NO_AUTH, so this
        // authorize_route call is an exact no-op (kept for the error path).
        if (const auto auth_error = m_input_support.authorize(req, "health")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const HealthInput input{};

        // 2. Input -> Result.
        auto result = m_runtime.health(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // 3. Result -> response. HttpResponse::Ok().body("health is good") is
        // plain text: the typed string value is encoded through Ser and the
        // text/plain content type is kept on the response.
        res.set_status(interfaces::io::types::Status::OK);
        res.add_content_type("text/plain");
        res.set_body(serde::Ser::serialize("application/json", result->get_value()));
        send();
    }

    void handle_deep_health_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // 1. request -> Input. Upstream uses &auth::NoAuth via server_wrap
        // (routes/health.rs:38); "deep_health_check" resolves to NO_AUTH.
        if (const auto auth_error = m_input_support.authorize(req, "deep_health_check")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        const DeepHealthCheckInput input{};

        // 2. Input -> Result.
        auto result = m_runtime.deep_health_check(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            // HE_00 stays in AppError::kind_name; the human-readable component
            // failure is in AppError::message. Both are serialized as typed fields.
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // 3. Result -> response. The RouterHealthCheckResponse document omits
        // None Option fields and carries dynamic gRPC keys as typed values.
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
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
    HealthInputSupport& m_input_support;
};

} // namespace app::handlers::health
