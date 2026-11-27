export module app_user:user_role_handlers_v1_list_info;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
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
        UserRoleInputSupport& input_support,
        UserRoleRequestSupport& request_support
    ) noexcept :
        m_context{context},
        m_runtime{runtime},
        m_input_support{input_support},
        m_support{request_support}
    {
    }

    // ── GET /permission_info — core::get_authorization_info_with_groups ────────
    void handle_get_authorization_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "get_authorization_info", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.get_authorization_info(ctx).handle(sink);
    }

    // ── GET /role — core::role::get_role_from_token_with_groups ────────────────
    void handle_get_role_from_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "get_role_from_token", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.get_role_from_token(ctx).handle(sink);
    }

    // ── GET /module/list — core::get_authorization_info_with_group_tag ─────────
    void handle_get_role_information(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "get_role_information", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.get_role_information(ctx).handle(sink);
    }

    // ── GET /parent/list — core::get_parent_group_info ─────────────────────────
    void handle_get_parent_group_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "get_parent_group_info", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign(
            "entity_type",
            std::string{m_support.query(req, "entity_type").value_or("")}
        );
        ctx.claims.insert_or_assign(
            "product_type",
            std::string{m_support.query(req, "product_type").value_or("")}
        );
        m_runtime.get_parent_group_info(ctx).handle(sink);
    }

    // ── GET /user/list — core::list_users_in_lineage ───────────────────────────
    void handle_list_users_in_lineage(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "list_users_in_lineage", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign(
            "entity_type",
            std::string{m_support.query(req, "entity_type").value_or("")}
        );
        m_runtime.list_users_in_lineage(ctx).handle(sink);
    }

    // ── GET /role/list — core::role::list_roles_with_info ──────────────────────
    void handle_list_roles_with_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "list_roles_with_info", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign(
            "entity_type",
            std::string{m_support.query(req, "entity_type").value_or("")}
        );
        ctx.claims.insert_or_assign(
            "groups",
            m_support.query(req, "groups").value_or("") == "true" ? "true" : "false"
        );
        m_runtime.list_roles_with_info(ctx).handle(sink);
    }

    // ── GET /role/list/update — core::role::list_roles_at_entity_level(Update) ─
    void handle_list_updatable_roles_at_entity_level(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "list_updatable_roles_at_entity_level", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign(
            "entity_type",
            std::string{m_support.query(req, "entity_type").value_or("")}
        );
        m_runtime.list_updatable_roles_at_entity_level(ctx).handle(sink);
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
    UserRoleRequestSupport& m_support;
};

} // namespace app::handlers::user_role
