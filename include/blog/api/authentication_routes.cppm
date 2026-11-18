export module blog_api:authentication_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /authentication. */
class AuthenticationRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::AuthenticationRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/authentication"}
            .add_route(
                Route{"/invitation"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::authentication::authentication_invitation_post::AuthenticationInvitationPostAuthenticationHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost invitation POST")
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::authentication::authentication_invitation_get::AuthenticationInvitationGetAuthenticationHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost invitation GET")
            )
            .add_route(
                Route{"/password_reset"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::authentication::authentication_password_reset_post::AuthenticationPasswordResetPostAuthenticationHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost password_reset POST")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::authentication::authentication_password_reset_put::AuthenticationPasswordResetPutAuthenticationHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost password_reset PUT")
            )
            .add_route(
                Route{"/reset"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::authentication::authentication_reset_post::AuthenticationResetPostAuthenticationHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost reset POST")
            )
            .add_route(
                Route{"/setup"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::authentication::authentication_setup_post::AuthenticationSetupPostAuthenticationHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost setup POST")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::authentication::authentication_setup_put::AuthenticationSetupPutAuthenticationHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost setup PUT")
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::authentication::authentication_setup_get::AuthenticationSetupGetAuthenticationHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost setup GET")
            );
    }
};

} // namespace blog::api
