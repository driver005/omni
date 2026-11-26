export module app_user:user_role_handlers_v1_list_info;

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
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_events;
import app_keystore;

import :user_role_input;
import :user_role_result;
import :user_role_runtime;

export namespace app::handlers::user_role {

// List/info family (permission_info, module/parent/user/role lists and
// token-derived role info). Handlers and route registrations are verbatim ports.
class UserRoleV1ListInfoHandlers
{
public:
    UserRoleV1ListInfoHandlers(
        ::app::context::AppContext& context,
        UserRoleRuntime& runtime,
        UserRoleInputSupport& input_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support}
    {
    }

    // ── GET /permission_info — core::get_authorization_info_with_groups ────────
    void handle_get_authorization_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "get_authorization_info")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        GetAuthorizationInfoInput input{m_input_support.identity_from(req)};
        auto result = m_runtime.get_authorization_info(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_items()));
        send();
    }

    // ── GET /role — core::role::get_role_from_token_with_groups ────────────────
    void handle_get_role_from_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "get_role_from_token")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        GetRoleFromTokenInput input{m_input_support.identity_from(req)};
        auto result = m_runtime.get_role_from_token(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_items()));
        send();
    }

    // ── GET /module/list — core::get_authorization_info_with_group_tag ─────────
    void handle_get_role_information(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "get_role_information")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        GetRoleInformationInput input{m_input_support.identity_from(req)};
        auto result = m_runtime.get_role_information(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_items()));
        send();
    }

    // ── GET /parent/list — core::get_parent_group_info ─────────────────────────
    void handle_get_parent_group_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "get_parent_group_info")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        GetParentGroupInfoInput input{
            m_input_support.identity_from(req),
            m_input_support.query_value(req, "entity_type"),
            m_input_support.query_value(req, "product_type")
        };
        auto result = m_runtime.get_parent_group_info(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_items()));
        send();
    }

    // ── GET /user/list — core::list_users_in_lineage ───────────────────────────
    void handle_list_users_in_lineage(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "list_users_in_lineage")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        ListUsersInLineageInput input{
            m_input_support.identity_from(req),
            m_input_support.query_value(req, "entity_type")
        };
        auto result = m_runtime.list_users_in_lineage(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_items()));
        send();
    }

    // ── GET /role/list — core::role::list_roles_with_info ──────────────────────
    void handle_list_roles_with_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_input_support.authorize(req, "list_roles_with_info")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        ListRolesWithInfoInput input{
            m_input_support.identity_from(req),
            m_input_support.query_value(req, "entity_type"),
            m_input_support.query_value(req, "groups") == "true"
        };
        auto result = m_runtime.list_roles_with_info(input);
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

    // ── GET /role/list/update — core::role::list_roles_at_entity_level(Update) ─
    void handle_list_updatable_roles_at_entity_level(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_input_support.authorize(req, "list_updatable_roles_at_entity_level")) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        ListUpdatableRolesAtEntityLevelInput input{
            m_input_support.identity_from(req),
            m_input_support.query_value(req, "entity_type")
        };
        auto result = m_runtime.list_updatable_roles_at_entity_level(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_items()));
        send();
    }

    // original install() route #01
    void install_route_01(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_route(
            Route{"permission_info"}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_get_authorization_info(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch get_authorization_info")
        );
    }

    // original install() route #02
    void install_route_02(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_route(
            Route{"role"}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_get_role_from_token(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch get_role_from_token")
        );
    }

    // original install() route #17
    void install_route_17(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "module"}.add_route(
                Route{"list"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_role_information(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_role_information")
            )
        );
    }

    // original install() route #18
    void install_route_18(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "parent"}.add_route(
                Route{"list"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_parent_group_info(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_parent_group_info")
            )
        );
    }

    // original install() route #19
    void install_route_19(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "user"}.add_route(
                Route{"list"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_list_users_in_lineage(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_users_in_lineage")
            )
        );
    }

    // original install() route #20
    void install_route_20(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "role"}.add_route(
                Route{"list"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_list_roles_with_info(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_roles_with_info")
            )
        );
    }

    // original install() route #23
    void install_route_23(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "role"}.add_router(
                Router{router, "list"}.add_route(
                    Route{"update"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_list_updatable_roles_at_entity_level(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch list_updatable_roles_at_entity_level")
                )
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_01(router);
        install_route_02(router);
        install_route_17(router);
        install_route_18(router);
        install_route_19(router);
        install_route_20(router);
        install_route_23(router);
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntime& m_runtime;
    UserRoleInputSupport& m_input_support;
};

} // namespace app::handlers::user_role
