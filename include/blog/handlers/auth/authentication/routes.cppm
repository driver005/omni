export module blog_handlers_routes:authentication_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class AuthenticationRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/authentication"}
            .use(blog::handlers::middleware::content_type_middleware)
            .add_route(
                Route{"/invitation"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authentication_authentication_invitation_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost invitation POST")
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authentication_authentication_invitation_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost invitation GET")
            )
            .add_route(
                Route{"/password_reset"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authentication_authentication_password_reset_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost password_reset POST")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authentication_authentication_password_reset_put().handle(req, res, std::move(send));
    })
                    .summary("Ghost password_reset PUT")
            )
            .add_route(
                Route{"/reset"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authentication_authentication_reset_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost reset POST")
            )
            .add_route(
                Route{"/setup"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authentication_authentication_setup_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost setup POST")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authentication_authentication_setup_put().handle(req, res, std::move(send));
    })
                    .summary("Ghost setup PUT")
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authentication_authentication_setup_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost setup GET")
            );
    }
};

} // namespace blog::api
