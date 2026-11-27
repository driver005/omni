export module app_routing:routing_handlers_v1_config_crud;

import std;
import interfaces;
import core_router;
import core_serde;
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
 * v1 config-CRUD family: entry, list, create, retrieve, link/unlink and the
 * linked-config lookup. Owns its own add_route registrations; the v1 version
 * facade replays install_route_NN in the original registration order.
 * Runtime/input support are owned once by the root facade and shared by ref.
 */
class RoutingV1ConfigCrudHandlers
{
public:
    RoutingV1ConfigCrudHandlers(
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

    // ─── core::routing::routing_entry (core/routing.rs:151-239) ────────────────────
    void handle_routing_entry(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support.authorize(req, "routing_entry", {})) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        RoutingInput input{};
        m_input_support.fill_common(req, input);
        // Upstream takes RoutingEntryRequest from web::Query, not the body (routing.rs:26).
        input.target = m_request_support.query(req, "target");

        auto result = m_runtime.routing_entry(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // ─── create_routing_algorithm_under_profile (core:826 v1 / :757 v2) ────────────
    void handle_routing_create_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support.authorize(req, "routing_create_config", {})) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        auto parsed = serde::Ser::deserialize<::app::model::routing::RoutingConfigRequest>(
            "application/json",
            body
        );
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        RoutingInput input{};
        m_input_support.fill_common(req, input);
        input.body = body;
        input.config_request = *parsed;

        auto result = m_runtime.routing_create_config(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // ─── link_routing_config (core:1119 v1) / link_..._under_profile (:1049 v2) ─────
    void handle_routing_link_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support.authorize(req, "routing_link_config", {})) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto body = req.body_as_string();
        RoutingInput input{};
        m_input_support.fill_common(req, input);
        input.body = body;
        if (!body.empty()) {
            auto link = serde::Ser::deserialize<::app::model::routing::RoutingLinkWrapper>(
                "application/json",
                body
            );
            auto activate = serde::Ser::deserialize<::app::model::routing::RoutingActivatePayload>(
                "application/json",
                body
            );
            if (!link && !activate) {
                const auto error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_JSON,
                    "invalid JSON"
                );
                error.send(res, send);
                return;
            }
            if (link) {
                input.link = *link;
            }
            if (activate) {
                input.activate = *activate;
            }
        }

        auto result = m_runtime.routing_link_config(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // ─── retrieve_routing_algorithm_from_algorithm_id (core:1605/1640) ─────────────
    void handle_routing_retrieve_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support.authorize(req, "routing_retrieve_config", {})) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        RoutingInput input{};
        m_input_support.fill_common(req, input);

        auto result = m_runtime.routing_retrieve_config(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // ─── retrieve_merchant_routing_dictionary (core:590) + merge (:636/:704) ───────
    // Wire: RoutingKind::RoutingAlgorithm -> untagged JSON array (api_models/routing.rs:950).
    void handle_list_routing_configs(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support.authorize(req, "list_routing_configs", {})) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        RoutingInput input{};
        m_input_support.fill_common(req, input);
        input.list_limit = m_input_support.query_number<std::uint16_t>(req, "limit");
        input.list_offset = m_input_support.query_number<std::uint8_t>(req, "offset");

        auto result = m_runtime.list_routing_configs(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_list_routing_configs_for_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_request_support.authorize(req, "list_routing_configs_for_profile", {})) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        RoutingInput input{};
        m_input_support.fill_common(req, input);
        input.list_limit = m_input_support.query_number<std::uint16_t>(req, "limit");
        input.list_offset = m_input_support.query_number<std::uint8_t>(req, "offset");

        auto result = m_runtime.list_routing_configs_for_profile(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // ─── unlink_routing_config (core:1767 v1) / unlink_..._under_profile (:1714 v2) ─
    void handle_routing_unlink_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = m_request_support.authorize(req, "routing_unlink_config", {})) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        const auto body = req.body_as_string();
        RoutingInput input{};
        m_input_support.fill_common(req, input);
        input.body = body;
        if (!body.empty()) {
            if (auto parsed = serde::Ser::deserialize<::app::model::routing::RoutingConfigRequest>(
                    "application/json",
                    body
                )) {
                input.config_request = *parsed;
            }
        }

        auto result = m_runtime.routing_unlink_config(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // ─── retrieve_linked_routing_config (core:2168 v1) / under_profile (:2128 v2) ──
    // Wire: LinkedRoutingConfigRetrieveResponse::ProfileBased -> untagged JSON array.
    void handle_routing_retrieve_linked_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied =
                m_request_support.authorize(req, "routing_retrieve_linked_config", {})) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }
        RoutingInput input{};
        m_input_support.fill_common(req, input);

        auto result = m_runtime.routing_retrieve_linked_config(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // ─── Route registrations (original install() order) ────────────────────────────
    void install_route_1(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_route(
            Route{"entry"}
                .post(
                    [this](auto& req, auto& res, auto send)
                    {
                        handle_routing_entry(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch routing_entry")
        );
    }

    void install_route_2(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_route(
            Route{"active"}
                .get(
                    [this](auto& req, auto& res, auto send)
                    {
                        handle_routing_retrieve_linked_config(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch routing_retrieve_linked_config")
        );
    }

    void install_route_3(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_route(
            Route{""}
                .get(
                    [this](auto& req, auto& res, auto send)
                    {
                        handle_list_routing_configs(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch list_routing_configs")
        );
    }

    void install_route_4(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_route(
            Route{""}
                .post(
                    [this](auto& req, auto& res, auto send)
                    {
                        handle_routing_create_config(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch routing_create_config")
        );
    }

    void install_route_5(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "list"}.add_route(
                Route{"profile"}
                    .get(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_list_routing_configs_for_profile(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_routing_configs_for_profile")
            )
        );
    }

    void install_route_9(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_route(
            Route{"deactivate"}
                .post(
                    [this](auto& req, auto& res, auto send)
                    {
                        handle_routing_unlink_config(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch routing_unlink_config")
        );
    }

    void install_route_18(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_route(
            Route{":algorithm_id"}
                .get(
                    [this](auto& req, auto& res, auto send)
                    {
                        handle_routing_retrieve_config(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch routing_retrieve_config")
        );
    }

    void install_route_19(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, ":algorithm_id"}.add_route(
                Route{"activate"}
                    .post(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_routing_link_config(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch routing_link_config")
            )
        );
    }

    void install_route_20(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_route(
            Route{"payouts"}
                .get(
                    [this](auto& req, auto& res, auto send)
                    {
                        handle_list_routing_configs(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch list_routing_configs")
        );
    }

    void install_route_21(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_route(
            Route{"payouts"}
                .post(
                    [this](auto& req, auto& res, auto send)
                    {
                        handle_routing_create_config(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch routing_create_config")
        );
    }

    void install_route_22(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "payouts"}.add_router(
                Router{router, "list"}.add_route(
                    Route{"profile"}
                        .get(
                            [this](auto& req, auto& res, auto send)
                            {
                                handle_list_routing_configs_for_profile(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch list_routing_configs_for_profile")
                )
            )
        );
    }

    void install_route_23(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "payouts"}.add_route(
                Route{"active"}
                    .get(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_routing_retrieve_linked_config(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch routing_retrieve_linked_config")
            )
        );
    }

    void install_route_26(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "payouts"}.add_router(
                Router{router, ":algorithm_id"}.add_route(
                    Route{"activate"}
                        .post(
                            [this](auto& req, auto& res, auto send)
                            {
                                handle_routing_link_config(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch routing_link_config")
                )
            )
        );
    }

    void install_route_27(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "routing"}.add_router(
            Router{router, "payouts"}.add_route(
                Route{"deactivate"}
                    .post(
                        [this](auto& req, auto& res, auto send)
                        {
                            handle_routing_unlink_config(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch routing_unlink_config")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_1(router);
        install_route_2(router);
        install_route_3(router);
        install_route_4(router);
        install_route_5(router);
        install_route_9(router);
        install_route_18(router);
        install_route_19(router);
        install_route_20(router);
        install_route_21(router);
        install_route_22(router);
        install_route_23(router);
        install_route_26(router);
        install_route_27(router);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntime& m_runtime;
    RoutingInputSupport& m_input_support;
    RoutingRequestSupport& m_request_support;
};

} // namespace app::handlers::routing
