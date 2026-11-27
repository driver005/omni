export module app_user:user_role_handlers_v1_invites;

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

// Invite family (unversioned /user/invite and /role/list/invite routes).
// The base /user/invite/accept route calls the distinct accept_invitations_v2
// runtime method; only the /v2 alias routes live in the v2 partition.
class UserRoleV1InvitesHandlers
{
public:
    UserRoleV1InvitesHandlers(
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

    // ── POST /user/invite/accept — core::accept_invitations_v2 ──────────────────
    void handle_accept_invitations_v2(
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

    // ── POST /user/invite/accept/pre_auth — core::accept_invitations_pre_auth ──
    void handle_accept_invitations_pre_auth(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "accept_invitations_pre_auth", {})) {
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

    // ── GET /role/list/invite — core::role::list_roles_at_entity_level(Invite) ──
    void handle_list_invitable_roles_at_entity_level(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "list_invitable_roles_at_entity_level", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign(
            "entity_type",
            std::string{m_support.query(req, "entity_type").value_or("")}
        );
        m_runtime.list_invitable_roles_at_entity_level(ctx).handle(sink);
    }

    // ── GET /list/invitation — core::list_invitations_for_user ─────────────────
    void handle_list_invitations_for_user(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "list_invitations_for_user", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.list_invitations_for_user(ctx).handle(sink);
    }

    // original install() route #11
    void install_route_11(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "user"}.add_router(
                Router{router, "invite"}.add_route(
                    Route{"accept"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_accept_invitations_v2(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch accept_invitations_v2")
                )
            )
        );
    }

    // original install() route #13
    void install_route_13(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "user"}.add_router(
                Router{router, "invite"}.add_router(
                    Router{router, "accept"}.add_route(
                        Route{"pre_auth"}
                            .post(
                                [this](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handle_accept_invitations_pre_auth(req, res, std::move(send));
                                }
                            )
                            .summary("Hyperswitch accept_invitations_pre_auth")
                    )
                )
            )
        );
    }

    // original install() route #22
    void install_route_22(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "role"}.add_router(
                Router{router, "list"}.add_route(
                    Route{"invite"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_list_invitable_roles_at_entity_level(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch list_invitable_roles_at_entity_level")
                )
            )
        );
    }

    // original install() route #24
    void install_route_24(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "list"}.add_route(
                Route{"invitation"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_list_invitations_for_user(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_invitations_for_user")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_11(router);
        install_route_13(router);
        install_route_22(router);
        install_route_24(router);
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntime& m_runtime;
    UserRoleInputSupport& m_input_support;
    UserRoleRequestSupport& m_support;
};

} // namespace app::handlers::user_role
