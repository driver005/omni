export module app_user:user_role_handlers_v2_list_info;

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

// Distinct versioned info methods (/role/v2 and /role/v3). These are not
// aliases — they call their own runtime methods and serialize their own results.
class UserRoleV2ListInfoHandlers
{
public:
    UserRoleV2ListInfoHandlers(
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

    // ── GET /role/v2 — core::role::get_groups_and_resources_for_role_from_token ─
    void handle_get_groups_and_resources_for_role_from_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "get_groups_and_resources_for_role_from_token", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.get_groups_and_resources_for_role_from_token(ctx).handle(sink);
    }

    // ── GET /role/v3 — core::role::get_parent_groups_info_for_role_from_token ──
    void handle_get_parent_groups_info_for_role_from_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        if (const auto auth_error =
                m_support.authorize(req, "get_parent_groups_info_for_role_from_token", {})) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }
        auto ctx = m_support.context(req);
        m_runtime.get_parent_groups_info_for_role_from_token(ctx).handle(sink);
    }

    // original install() route #03
    void install_route_03(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "role"}.add_route(
                Route{"v2"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_groups_and_resources_for_role_from_token(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch get_groups_and_resources_for_role_from_token")
            )
        );
    }

    // original install() route #04
    void install_route_04(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "user"}.add_router(
            Router{router, "role"}.add_route(
                Route{"v3"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_parent_groups_info_for_role_from_token(
                                req,
                                res,
                                std::move(send)
                            );
                        }
                    )
                    .summary("Hyperswitch get_parent_groups_info_for_role_from_token")
            )
        );
    }

    void install(core::router::RouterContext<>& router)
    {
        install_route_03(router);
        install_route_04(router);
    }

private:
    ::app::context::AppContext& m_context;
    UserRoleRuntime& m_runtime;
    UserRoleInputSupport& m_input_support;
    UserRoleRequestSupport& m_support;
};

} // namespace app::handlers::user_role
