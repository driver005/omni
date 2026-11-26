export module app_routing:routing_handlers_dynamic_routing;

import std;
import interfaces;
import core_router;
import core_serde;
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
        RoutingInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    // ─── create_specific_dynamic_routing (core:2556), SuccessRateBasedRouting ──────
    void handle_create_success_based_routing(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_input_support.authorize_operation(req, "create_success_based_routing")) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        RoutingInput input{};
        m_input_support.fill_common(req, input);
        input.body = req.body_as_string();

        auto result = m_runtime.create_success_based_routing(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    void handle_create_elimination_routing(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_input_support.authorize_operation(req, "create_elimination_routing")) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        RoutingInput input{};
        m_input_support.fill_common(req, input);
        input.body = req.body_as_string();

        auto result = m_runtime.create_elimination_routing(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    // ─── contract_based_dynamic_routing_setup (core:2717) ───────────────────────────
    void handle_contract_based_routing_setup_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_input_support.authorize_operation(req, "contract_based_routing_setup_config")) {
            const auto& error = *denied;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        RoutingInput input{};
        m_input_support.fill_common(req, input);
        input.body = req.body_as_string();

        auto result = m_runtime.contract_based_routing_setup_config(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
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
};

} // namespace app::handlers::routing
