export module app_routing:routing_handlers_v2_decision;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_context;
import core_error;

import :routing_input;
import :routing_result;
import :routing_runtime;

export namespace app::handlers::routing {

/**
 * v2 decision family: three_ds decision-manager upsert/retrieve under
 * profiles/:profile_id. Adapters are duplicated from the v1 copies on purpose:
 * upstream registers the same operations under both scopes and version
 * partitions do not share methods. Runtime/input support are owned once by
 * the root facade and shared by ref.
 */
class RoutingV2DecisionHandlers
{
public:
    RoutingV2DecisionHandlers(
        ::app::context::AppContext& context,
        RoutingRuntime& runtime,
        RoutingInputSupport& input_support,
        RoutingRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_request_support{request_support}
    {
    }

    // ─── upsert_decision_manager_config (shared v1/v2 adapter, v2 scope) ───────────
    void handle_upsert_decision_manager_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "upsert_decision_manager_config", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        auto parsed = serde::Ser::decode_generic(req.get_content_type(), req.body_as_string());
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(sink);
            return;
        }
        m_runtime.upsert_decision_manager_config(*parsed, m_request_support.context(req))
            .handle(sink);
    }

    // ─── retrieve_decision_manager_config (shared v1/v2 adapter, v2 scope) ─────────
    void handle_retrieve_decision_manager_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "retrieve_decision_manager_config", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        m_runtime.retrieve_decision_manager_config(m_request_support.context(req)).handle(sink);
    }

    // ─── Route registrations (original install() order) ────────────────────────────
    void install_route_37(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "v2"}.add_router(
                Router{router, "profiles"}.add_router(
                    Router{router, ":profile_id"}.add_route(
                        Route{"decision"}
                            .put(
                                [this](auto& req, auto& res, auto send)
                                {
                                    handle_upsert_decision_manager_config(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch upsert_decision_manager_config")
                    )
                )
            )
        );
    }

    void install_route_38(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "v2"}.add_router(
                Router{router, "profiles"}.add_router(
                    Router{router, ":profile_id"}.add_route(
                        Route{"decision"}
                            .get(
                                [this](auto& req, auto& res, auto send)
                                {
                                    handle_retrieve_decision_manager_config(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch retrieve_decision_manager_config")
                    )
                )
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_37(router);
        install_route_38(router);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntime& m_runtime;
    RoutingInputSupport& m_input_support;
    RoutingRequestSupport& m_request_support;
};

} // namespace app::handlers::routing
