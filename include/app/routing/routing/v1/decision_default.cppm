export module app_routing:routing_handlers_v1_decision;

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
 * v1 default-config and decision-manager family. Owns its own add_route registrations;
 * the v1 version facade replays install_route_NN in the original order.
 * Runtime/input support are owned once by the root facade and shared by ref.
 */
class RoutingV1DecisionHandlers
{
public:
    RoutingV1DecisionHandlers(
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

    // ─── update_default_routing_config (core:2026 v1) / update_default_fallback (:1954 v2) ─
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

    // ─── retrieve_default_routing_config (core:2101 v1) / fallback (:2079 v2) ─────
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

    // ─── conditional_config::upsert_conditional_config (v1 :66 / v2 :18) ───────────
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

    void handle_delete_decision_manager_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "delete_decision_manager_config", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        m_runtime.delete_decision_manager_config(m_request_support.context(req)).handle(sink);
    }

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

    // ─── retrieve_default_routing_config_for_profiles (core:2408) ──────────────────
    // Wire: Vec<ProfileDefaultRoutingConfig>.
    void handle_routing_retrieve_default_config_for_profiles(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support
                    .authorize(req, "routing_retrieve_default_config_for_profiles", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        m_runtime.routing_retrieve_default_config_for_profiles(m_request_support.context(req))
            .handle(sink);
    }

    // ─── update_default_routing_config_for_profile (core:2454) ─────────────────────
    void handle_routing_update_default_config_for_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "routing_update_default_config_for_profile", {})) {
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
        m_runtime.routing_update_default_config_for_profile(*parsed, m_request_support.context(req))
            .handle(sink);
    }

    // ─── Route registrations (original install() order) ────────────────────────────
    void install_route_6(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_route(
            Route{"default"}
                .post(
                    [this](auto& req, auto& res, auto send)
                    {
                        handle_routing_update_default_config(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch routing_update_default_config")
        );
    }

    void install_route_10(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_route(
            Route{"decision"}
                .put(
                    [this](auto& req, auto& res, auto send)
                    {
                        handle_upsert_decision_manager_config(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch upsert_decision_manager_config")
        );
    }

    void install_route_11(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_route(
            Route{"decision"}
                .get(
                    [this](auto& req, auto& res, auto send)
                    {
                        handle_retrieve_decision_manager_config(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch retrieve_decision_manager_config")
        );
    }

    void install_route_12(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_route(
            Route{"decision"}
                .delt(
                    [this](auto& req, auto& res, auto send)
                    {
                        handle_delete_decision_manager_config(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch delete_decision_manager_config")
        );
    }

    void install_route_16(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "default"}.add_router(
                Router{router, "profile"}.add_route(
                    Route{":profile_id"}
                        .post(
                            [this](auto& req, auto& res, auto send)
                            {
                                handle_routing_update_default_config_for_profile(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch routing_update_default_config_for_profile")
                )
            )
        );
    }

    void install_route_17(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "default"}.add_route(
                Route{"profile"}
                    .get(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_routing_retrieve_default_config(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch routing_retrieve_default_config")
            )
        );
    }

    void install_route_24(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "payouts"}.add_route(
                Route{"default"}
                    .get(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_routing_retrieve_default_config(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch routing_retrieve_default_config")
            )
        );
    }

    void install_route_25(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "payouts"}.add_route(
                Route{"default"}
                    .post(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_routing_update_default_config(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch routing_update_default_config")
            )
        );
    }

    void install_route_28(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "payouts"}.add_router(
                Router{router, "default"}.add_router(
                    Router{router, "profile"}.add_route(
                        Route{":profile_id"}
                            .post(
                                [this](auto& req, auto& res, auto send)
                                {
                                    handle_routing_update_default_config_for_profile(
                                        req,
                                        res,
                                        std::move(send)
                                    );
                                }
                            )
                            .summary("Hyperswitch routing_update_default_config_for_profile")
                    )
                )
            )
        );
    }

    void install_route_29(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "payouts"}.add_router(
                Router{router, "default"}.add_route(
                    Route{"profile"}
                        .get(
                            [this](auto& req, auto& res, auto send)
                            {
                                handle_routing_retrieve_default_config_for_profiles(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch routing_retrieve_default_config_for_profiles")
                )
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_6(router);
        install_route_10(router);
        install_route_11(router);
        install_route_12(router);
        install_route_16(router);
        install_route_17(router);
        install_route_24(router);
        install_route_25(router);
        install_route_28(router);
        install_route_29(router);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntime& m_runtime;
    RoutingInputSupport& m_input_support;
    RoutingRequestSupport& m_request_support;
};

} // namespace app::handlers::routing
