export module app_routing:routing_handlers_v1_surcharge;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_auth;
import app_http;

import :routing_input;
import :routing_result;
import :routing_runtime;

export namespace app::handlers::routing {

/**
 * v1 surcharge decision-manager family: upsert/retrieve/delete under
 * decision/surcharge. Owns its own add_route registrations; the v1 version
 * facade replays install_route_NN in the original order. Runtime/input support
 * are owned once by the root facade and shared by ref.
 */
class RoutingV1SurchargeHandlers
{
public:
    RoutingV1SurchargeHandlers(
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

    // ─── surcharge_decision_config::upsert_surcharge_decision_config ───────────────
    void handle_upsert_surcharge_decision_manager_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "upsert_surcharge_decision_manager_config", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        auto parsed = core::behavior::Serializer<
            ::app::dto::surcharge_decision_configs::SurchargeDecisionConfigReq>::pass(req, sink);
        if (!parsed) {
            return;
        }
        m_runtime.upsert_surcharge_decision_manager_config(*parsed, m_request_support.context(req))
            .handle(sink);
    }

    void handle_delete_surcharge_decision_manager_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "delete_surcharge_decision_manager_config", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        m_runtime.delete_surcharge_decision_manager_config(m_request_support.context(req))
            .handle(sink);
    }

    void handle_retrieve_surcharge_decision_manager_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support
                    .authorize(req, "retrieve_surcharge_decision_manager_config", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        m_runtime.retrieve_surcharge_decision_manager_config(m_request_support.context(req))
            .handle(sink);
    }

    void install_route_13(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "decision"}.add_route(
                Route{"surcharge"}
                    .put(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_upsert_surcharge_decision_manager_config(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch upsert_surcharge_decision_manager_config")
            )
        );
    }

    void install_route_14(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "decision"}.add_route(
                Route{"surcharge"}
                    .get(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_retrieve_surcharge_decision_manager_config(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch retrieve_surcharge_decision_manager_config")
            )
        );
    }

    void install_route_15(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "decision"}.add_route(
                Route{"surcharge"}
                    .delt(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_delete_surcharge_decision_manager_config(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch delete_surcharge_decision_manager_config")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_13(router);
        install_route_14(router);
        install_route_15(router);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntime& m_runtime;
    RoutingInputSupport& m_input_support;
    RoutingRequestSupport& m_request_support;
};

} // namespace app::handlers::routing
