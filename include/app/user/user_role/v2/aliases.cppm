export module app_user:user_role_handlers_v2_aliases;

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

// V2 alias family. Each original handler registered under both a base path and
// a /v2 path is duplicated here as a thin adapter (same gate string, same
// runtime call, same serialization). No handler is shared across versions.
class UserRoleV2AliasesHandlers
{
public:
    UserRoleV2AliasesHandlers(
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

    // Alias of handle_accept_invitations_v2 (POST /user/invite/accept/v2).
    void handle_accept_invitations_v2_alias(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "accept_invitations_v2", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request");
            error.send(sink);
            return;
        }
        auto parsed =
            serde::Ser::deserialize<std::vector<AcceptEntityWire>>("application/json", body);
        if (!parsed || parsed->empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid invitation payload"
            );
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.accept_invitations_v2(*parsed, ctx).handle(sink);
    }

    // Alias of handle_accept_invitations_pre_auth (POST .../accept/v2/pre_auth).
    void handle_accept_invitations_pre_auth_alias(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "accept_invitations_pre_auth", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request");
            error.send(sink);
            return;
        }
        auto parsed =
            serde::Ser::deserialize<std::vector<AcceptEntityWire>>("application/json", body);
        if (!parsed || parsed->empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid invitation payload"
            );
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.accept_invitations_pre_auth(*parsed, ctx).handle(sink);
    }

    // Alias of handle_list_users_in_lineage (GET /user/v2/list).
    void handle_list_users_in_lineage_alias(
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
        ctx.claims.insert_or_assign("entity_type", std::string{m_support.query(req, "entity_type").value_or("")});
        m_runtime.list_users_in_lineage(ctx).handle(sink);
    }

    // Alias of handle_list_roles_with_info (GET /role/v2/list).
    void handle_list_roles_with_info_alias(
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
        ctx.claims.insert_or_assign("entity_type", std::string{m_support.query(req, "entity_type").value_or("")});
        ctx.claims.insert_or_assign("groups", m_support.query(req, "groups").value_or("") == "true" ? "true" : "false");
        m_runtime.list_roles_with_info(ctx).handle(sink);
    }

    // original install() route #12
    void install_route_12(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "user"}.add_router(
                Router{router, "invite"}.add_router(
                    Router{router, "accept"}.add_route(
                        Route{"v2"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_accept_invitations_v2_alias(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch accept_invitations_v2 (v2 alias)")
                    )
                )
            )
        );
    }

    // original install() route #14
    void install_route_14(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "user"}.add_router(
                Router{router, "invite"}.add_router(
                    Router{router, "accept"}.add_router(
                        Router{router, "v2"}.add_route(
                            Route{"pre_auth"}
                                .post(
                                    [this](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        handle_accept_invitations_pre_auth_alias(
                                            req,
                                            res,
                                            std::move(send)
                                        );
                                    }
                                )
                                .summary("Hyperswitch accept_invitations_pre_auth (v2 alias)")
                        )
                    )
                )
            )
        );
    }

    // original install() route #16
    void install_route_16(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "user"}.add_router(
                Router{router, "v2"}.add_route(
                    Route{"list"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_list_users_in_lineage_alias(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch list_users_in_lineage (v2 alias)")
                )
            )
        );
    }

    // original install() route #21
    void install_route_21(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "role"}.add_router(
                Router{router, "v2"}.add_route(
                    Route{"list"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_list_roles_with_info_alias(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch list_roles_with_info (v2 alias)")
                )
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_12(router);
        install_route_14(router);
        install_route_16(router);
        install_route_21(router);
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntime& m_runtime;
    UserRoleInputSupport& m_input_support;
    UserRoleRequestSupport& m_support;
};

} // namespace app::handlers::user_role
