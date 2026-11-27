export module app_user:user_role_handlers_v2_roles_crud;

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

// Distinct v2 role methods: create_role_v2 and get_parent_info_for_role
// (mounted at /role/v2 and /role/:role_id/v2). No methods shared with v1.
class UserRoleV2RolesCrudHandlers
{
public:
    UserRoleV2RolesCrudHandlers(
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

    // ── POST /role/v2 — core::role::create_role_v2 ─────────────────────────────
    void handle_create_role_v2(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error = m_support.authorize(req, "create_role_v2", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto parsed = core::behavior::Serializer<CreateRoleInputWire>::pass(req, sink);
        if (!parsed) return;
        auto ctx = m_support.context(req);
        m_runtime.create_role_v2(*parsed, ctx).handle(sink);
    }

    // ── GET /role/:role_id/v2 — core::role::get_parent_info_for_role ───────────
    void handle_get_parent_info_for_role(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "get_parent_info_for_role", {})) {
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
        m_runtime.get_parent_info_for_role(ctx).handle(sink);
    }

    // original install() route #06
    void install_route_06(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "role"}.add_route(
                Route{"v2"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_create_role_v2(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch create_role_v2")
            )
        );
    }

    // original install() route #08
    void install_route_08(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "role"}.add_router(
                Router{router, ":role_id"}.add_route(
                    Route{"v2"}
                        .get(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_get_parent_info_for_role(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch get_parent_info_for_role")
                )
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_06(router);
        install_route_08(router);
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntime& m_runtime;
    UserRoleInputSupport& m_input_support;
    UserRoleRequestSupport& m_support;
};

} // namespace app::handlers::user_role
