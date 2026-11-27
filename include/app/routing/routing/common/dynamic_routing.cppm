export module app_routing:routing_handlers_dynamic_routing;

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
 * Unversioned dynamic-routing family: success-based, elimination and
 * contract-based setup under account/:account_id/business_profile/:profile_id.
 * Owns its own add_route registrations; the common facade replays
 * install_route_NN in the original order. Runtime/input support are owned once
 * by the root facade and shared by ref.
 */
class RoutingDynamicRoutingHandlers
{
public:
    RoutingDynamicRoutingHandlers(
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

    // ─── create_specific_dynamic_routing (core:2556), SuccessRateBasedRouting ──────
    void handle_create_success_based_routing(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "create_success_based_routing", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        std::optional<serde::Value> body{};
        const auto text = req.body_as_string();
        if (!text.empty()) {
            body = serde::Ser::decode_generic(req.get_content_type(), text);
            if (!body) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_JSON,
                    "invalid JSON"
                );
                error.send(sink);
                return;
            }
        }
        m_runtime.create_success_based_routing(body, m_request_support.context(req)).handle(sink);
    }

    void handle_create_elimination_routing(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "create_elimination_routing", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        std::optional<serde::Value> body{};
        const auto text = req.body_as_string();
        if (!text.empty()) {
            body = serde::Ser::decode_generic(req.get_content_type(), text);
            if (!body) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_JSON,
                    "invalid JSON"
                );
                error.send(sink);
                return;
            }
        }
        m_runtime.create_elimination_routing(body, m_request_support.context(req)).handle(sink);
    }

    // ─── contract_based_dynamic_routing_setup (core:2717) ───────────────────────────
    void handle_contract_based_routing_setup_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto denied =
                m_request_support.authorize(req, "contract_based_routing_setup_config", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }
        std::optional<serde::Value> body{};
        const auto text = req.body_as_string();
        if (!text.empty()) {
            body = serde::Ser::decode_generic(req.get_content_type(), text);
            if (!body) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_JSON,
                    "invalid JSON"
                );
                error.send(sink);
                return;
            }
        }
        m_runtime.contract_based_routing_setup_config(body, m_request_support.context(req))
            .handle(sink);
    }

    // ─── Route registrations (original install() order) ────────────────────────────
    void install_route_39(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "account"}.add_router(
                Router{router, ":account_id"}.add_router(
                    Router{router, "business_profile"}.add_router(
                        Router{router, ":profile_id"}.add_router(
                            Router{router, "dynamic_routing"}.add_router(
                                Router{router, "success_based"}.add_route(
                                    Route{"create"}
                                        .post(
                                            [this](auto& req, auto& res, auto send)
                                            {
                                                handle_create_success_based_routing(
                                                    req,
                                                    res,
                                                    std::move(send)
                                                );
                                            }
                                        )
                                        .summary("Hyperswitch create_success_based_routing")
                                )
                            )
                        )
                    )
                )
            )
        );
    }

    void install_route_42(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "account"}.add_router(
                Router{router, ":account_id"}.add_router(
                    Router{router, "business_profile"}.add_router(
                        Router{router, ":profile_id"}.add_router(
                            Router{router, "dynamic_routing"}.add_router(
                                Router{router, "elimination"}.add_route(
                                    Route{"create"}
                                        .post(
                                            [this](auto& req, auto& res, auto send)
                                            {
                                                handle_create_elimination_routing(
                                                    req,
                                                    res,
                                                    std::move(send)
                                                );
                                            }
                                        )
                                        .summary("Hyperswitch create_elimination_routing")
                                )
                            )
                        )
                    )
                )
            )
        );
    }

    void install_route_43(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "account"}.add_router(
                Router{router, ":account_id"}.add_router(
                    Router{router, "business_profile"}.add_router(
                        Router{router, ":profile_id"}.add_router(
                            Router{router, "dynamic_routing"}.add_router(
                                Router{router, "contracts"}.add_route(
                                    Route{"toggle"}
                                        .post(
                                            [this](auto& req, auto& res, auto send)
                                            {
                                                handle_contract_based_routing_setup_config(
                                                    req,
                                                    res,
                                                    std::move(send)
                                                );
                                            }
                                        )
                                        .summary("Hyperswitch contract_based_routing_setup_config")
                                )
                            )
                        )
                    )
                )
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_39(router);
        install_route_42(router);
        install_route_43(router);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntime& m_runtime;
    RoutingInputSupport& m_input_support;
    RoutingRequestSupport& m_request_support;
};

} // namespace app::handlers::routing
