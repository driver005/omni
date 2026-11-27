export module app_user:user_role_handlers_v1_roles_crud;

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

// Role CRUD family (unversioned /role and /user routes). Handlers are verbatim
// ports of the original facade bodies; each install_route_NN owns one original
// add_route registration so the facade can replay exact registration order.
class UserRoleV1RolesCrudHandlers
{
public:
    UserRoleV1RolesCrudHandlers(
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

    // ── POST /role — core::role::create_role ───────────────────────────────────
    void handle_create_role(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "create_role", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto parsed = core::behavior::Serializer<CreateRoleInputWire>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.create_role(*parsed, ctx).handle(sink);
    }

    // ── GET /role/:role_id — core::role::get_role_with_groups ──────────────────
    void handle_get_role(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "get_role", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const auto role_id = m_input_support.role_id_from_path(req);
        if (!role_id.has_value() || role_id->empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleId");
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("role_id", std::string{*role_id});
        m_runtime.get_role(ctx).handle(sink);
    }

    // ── PUT /role/:role_id — core::role::update_role ───────────────────────────
    void handle_update_role(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "update_role", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        const auto role_id = m_input_support.role_id_from_path(req);
        if (!role_id.has_value() || role_id->empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::NOT_FOUND, "InvalidRoleOperation");
            error.send(sink);
            return;
        }
        const auto body = req.body_as_string();
        auto parsed = core::behavior::Serializer<UpdateRoleInputWire>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("role_id", std::string{*role_id});
        m_runtime.update_role(*parsed, ctx).handle(sink);
    }

    // ── POST /user/update_role — core::update_user_role ────────────────────────
    void handle_update_user_role(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "update_user_role", {})) {
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
        auto parsed = serde::Ser::deserialize<::app::dto::user_role::UpdateUserRoleRequest>(
            "application/json",
            body
        );
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.update_user_role(*parsed, ctx).handle(sink);
    }

    // ── DELETE /user/delete — core::delete_user_role ───────────────────────────
    void handle_delete_user_role(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "delete_user_role", {})) {
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
        auto parsed = serde::Ser::deserialize<::app::dto::user_role::DeleteUserRoleRequest>(
            "application/json",
            body
        );
        if (!parsed) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON");
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.delete_user_role(*parsed, ctx).handle(sink);
    }

    // original install() route #05
    void install_route_05(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_route(
            Route{"role"}
                .post(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handle_create_role(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch create_role")
        );
    }

    // original install() route #07
    void install_route_07(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "role"}.add_route(
                Route{":role_id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_role(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_role")
            )
        );
    }

    // original install() route #09
    void install_route_09(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "role"}.add_route(
                Route{":role_id"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_update_role(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch update_role")
            )
        );
    }

    // original install() route #10
    void install_route_10(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "user"}.add_route(
                Route{"update_role"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_update_user_role(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch update_user_role")
            )
        );
    }

    // original install() route #15
    void install_route_15(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "user"}.add_route(
                Route{"delete"}
                    .delt(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_delete_user_role(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch delete_user_role")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_05(router);
        install_route_07(router);
        install_route_09(router);
        install_route_10(router);
        install_route_15(router);
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntime& m_runtime;
    UserRoleInputSupport& m_input_support;
    UserRoleRequestSupport& m_support;
};

} // namespace app::handlers::user_role
