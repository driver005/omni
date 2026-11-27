export module app_routing:routing_handlers_v2_algorithms;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_model;
import app_context;
import core_error;

import :routing_input;
import :routing_result;
import :routing_runtime;

export namespace app::handlers::routing {

/**
 * v2 algorithms family: routing-algorithm create/retrieve, fallback (default)
 * get/patch, and activate/deactivate/linked under profiles/:profile_id.
 * These adapters are duplicated from the v1 copies on purpose: upstream registers
 * the same operations under both scopes and version partitions do not share methods.
 * Runtime/input support are owned once by the root facade and shared by ref.
 */
class RoutingV2AlgorithmsHandlers
{
public:
    RoutingV2AlgorithmsHandlers(
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

    // ─── routing_create_config (shared v1/v2 adapter, v2 scope) ────────────────────
    void handle_routing_create_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "routing_create_config", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        auto parsed = core::behavior::Serializer<::app::model::routing::RoutingConfigRequest>::pass(
            req,
            sink
        );
        if (!parsed) {
            return;
        }
        m_runtime.routing_create_config(*parsed, m_request_support.context(req)).handle(sink);
    }

    // ─── routing_retrieve_config (shared v1/v2 adapter, v2 scope) ──────────────────
    void handle_routing_retrieve_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "routing_retrieve_config", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        m_runtime.routing_retrieve_config(m_request_support.context(req)).handle(sink);
    }

    // ─── routing_update_default_config (shared v1/v2 adapter, v2 scope) ────────────
    void handle_routing_update_default_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "routing_update_default_config", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        auto parsed =
            serde::Ser::deserialize<Connectors>(req.get_content_type(), req.body_as_string());
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(sink);
            return;
        }
        m_runtime.routing_update_default_config(*parsed, m_request_support.context(req))
            .handle(sink);
    }

    // ─── routing_retrieve_default_config (shared v1/v2 adapter, v2 scope) ──────────
    // Wire: Vec<RoutableConnectorChoice> in both cases.
    void handle_routing_retrieve_default_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "routing_retrieve_default_config", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        m_runtime.routing_retrieve_default_config(m_request_support.context(req)).handle(sink);
    }

    // ─── routing_link_config (shared v1/v2 adapter, v2 scope) ──────────────────────
    void handle_routing_link_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "routing_link_config", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        std::optional<::app::model::routing::RoutingLinkWrapper> link{};
        std::optional<::app::model::routing::RoutingActivatePayload> activate{};
        const auto body = req.body_as_string();
        if (!body.empty()) {
            link = serde::Ser::deserialize<::app::model::routing::RoutingLinkWrapper>(
                "application/json",
                body
            );
            activate = serde::Ser::deserialize<::app::model::routing::RoutingActivatePayload>(
                "application/json",
                body
            );
            if (!link && !activate) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_JSON,
                    "invalid JSON"
                );
                error.send(sink);
                return;
            }
        }
        m_runtime.routing_link_config(link, activate, m_request_support.context(req)).handle(sink);
    }

    // ─── routing_unlink_config (shared v1/v2 adapter, v2 scope) ────────────────────
    void handle_routing_unlink_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied = m_request_support.authorize(req, "routing_unlink_config", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        std::optional<::app::model::routing::RoutingConfigRequest> config_request{};
        const auto body = req.body_as_string();
        if (!body.empty()) {
            config_request = serde::Ser::deserialize<::app::model::routing::RoutingConfigRequest>(
                "application/json",
                body
            );
        }
        m_runtime.routing_unlink_config(config_request, m_request_support.context(req))
            .handle(sink);
    }

    // ─── routing_retrieve_linked_config (shared v1/v2 adapter, v2 scope) ───────────
    // Wire: LinkedRoutingConfigRetrieveResponse::ProfileBased -> untagged JSON array.
    void handle_routing_retrieve_linked_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "routing_retrieve_linked_config", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        m_runtime.routing_retrieve_linked_config(m_request_support.context(req)).handle(sink);
    }

    // ─── Route registrations (original install() order) ────────────────────────────
    void install_route_30(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "v2"}.add_route(
                Route{"routing-algorithms"}
                    .post(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_routing_create_config(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch routing_create_config")
            )
        );
    }

    void install_route_31(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "v2"}.add_router(
                Router{router, "routing-algorithms"}.add_route(
                    Route{":algorithm_id"}
                        .get(
                            [this](auto& req, auto& res, auto send)
                            {
                                handle_routing_retrieve_config(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch routing_retrieve_config")
                )
            )
        );
    }

    void install_route_32(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "v2"}.add_router(
                Router{router, "profiles"}.add_router(
                    Router{router, ":profile_id"}.add_route(
                        Route{"fallback-routing"}
                            .get(
                                [this](auto& req, auto& res, auto send)
                                {
                                    handle_routing_retrieve_default_config(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch routing_retrieve_default_config")
                    )
                )
            )
        );
    }

    void install_route_33(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "v2"}.add_router(
                Router{router, "profiles"}.add_router(
                    Router{router, ":profile_id"}.add_route(
                        Route{"fallback-routing"}
                            .patch(
                                [this](auto& req, auto& res, auto send)
                                {
                                    handle_routing_update_default_config(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch routing_update_default_config")
                    )
                )
            )
        );
    }

    void install_route_34(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "v2"}.add_router(
                Router{router, "profiles"}.add_router(
                    Router{router, ":profile_id"}.add_route(
                        Route{"activate-routing-algorithm"}
                            .patch(
                                [this](auto& req, auto& res, auto send)
                                {
                                    handle_routing_link_config(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch routing_link_config")
                    )
                )
            )
        );
    }

    void install_route_35(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "v2"}.add_router(
                Router{router, "profiles"}.add_router(
                    Router{router, ":profile_id"}.add_route(
                        Route{"deactivate-routing-algorithm"}
                            .patch(
                                [this](auto& req, auto& res, auto send)
                                {
                                    handle_routing_unlink_config(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch routing_unlink_config")
                    )
                )
            )
        );
    }

    void install_route_36(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "v2"}.add_router(
                Router{router, "profiles"}.add_router(
                    Router{router, ":profile_id"}.add_route(
                        Route{"routing-algorithm"}
                            .get(
                                [this](auto& req, auto& res, auto send)
                                {
                                    handle_routing_retrieve_linked_config(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch routing_retrieve_linked_config")
                    )
                )
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_30(router);
        install_route_31(router);
        install_route_32(router);
        install_route_33(router);
        install_route_34(router);
        install_route_35(router);
        install_route_36(router);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntime& m_runtime;
    RoutingInputSupport& m_input_support;
    RoutingRequestSupport& m_request_support;
};

} // namespace app::handlers::routing
