export module app_user:user_role_handlers_v2_aliases;

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
        m_request_support{request_support}
    {
    }

    // Alias of handle_accept_invitations_v2 (POST /user/invite/accept/v2).
    void handle_accept_invitations_v2_alias(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support.authorize(req, "accept_invitations_v2", {})) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        const auto identity = m_input_support.identity_from(req);
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request");
            error.send(res, send);
            return;
        }
        auto parsed =
            serde::Ser::deserialize<std::vector<AcceptEntityWire>>("application/json", body);
        if (!parsed || parsed->empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid invitation payload"
            );
            error.send(res, send);
            return;
        }
        AcceptInvitationsV2Input input{identity, *parsed};
        auto result = m_runtime.accept_invitations_v2(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        auto encoded = result->serialize(kStatusOkWireFields);
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_body(serde::Ser::serialize_raw("application/json", *encoded));
        send();
    }

    // Alias of handle_accept_invitations_pre_auth (POST .../accept/v2/pre_auth).
    void handle_accept_invitations_pre_auth_alias(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_request_support.authorize(req, "accept_invitations_pre_auth", {})) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        const auto identity = m_input_support.identity_from(req);
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error =
                m_input_support.error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request");
            error.send(res, send);
            return;
        }
        auto parsed =
            serde::Ser::deserialize<std::vector<AcceptEntityWire>>("application/json", body);
        if (!parsed || parsed->empty()) {
            const auto error = m_input_support.error(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid invitation payload"
            );
            error.send(res, send);
            return;
        }
        AcceptInvitationsPreAuthInput input{identity, *parsed};
        auto result = m_runtime.accept_invitations_pre_auth(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_header(std::string_view{"set-cookie"}, result->second);
        res.set_status(interfaces::io::types::Status::OK);
        auto encoded = result->first.serialize(kTokenResponseWireFields);
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_body(serde::Ser::serialize_raw("application/json", *encoded));
        send();
    }

    // Alias of handle_list_users_in_lineage (GET /user/v2/list).
    void handle_list_users_in_lineage_alias(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support.authorize(req, "list_users_in_lineage", {})) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        ListUsersInLineageInput input{
            m_input_support.identity_from(req),
            m_request_support.query(req, "entity_type").value_or("")
        };
        auto result = m_runtime.list_users_in_lineage(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        auto encoded = serde::Serializer<std::vector<ListUsersInEntityWire>>::serialize_values(
            *result,
            kListUsersInEntityWireFields
        );
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_body(serde::Ser::serialize_raw("application/json", *encoded));
        send();
    }

    // Alias of handle_list_roles_with_info (GET /role/v2/list).
    void handle_list_roles_with_info_alias(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support.authorize(req, "list_roles_with_info", {})) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }
        ListRolesWithInfoInput input{
            m_input_support.identity_from(req),
            m_request_support.query(req, "entity_type").value_or(""),
            m_request_support.query(req, "groups").value_or("") == "true"
        };
        auto result = m_runtime.list_roles_with_info(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
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
    UserRoleRequestSupport& m_request_support;
};

} // namespace app::handlers::user_role
