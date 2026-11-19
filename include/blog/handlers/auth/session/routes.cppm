export module blog_handlers_routes:session_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;

export namespace blog::api {

class SessionRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/session"}
            .use(blog::handlers::middleware::content_type_middleware)
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.session_create().handle(req, res, std::move(send));
    })
            .summary("Ghost session POST")
            .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.session_bulk_destroy().handle(req, res, std::move(send));
    })
            .summary("Ghost session DELETE")
            .add_route(
                Route{"/verify"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.session_session_verify_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost verify POST")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.session_session_verify_put().handle(req, res, std::move(send));
    })
                    .summary("Ghost verify PUT")
            );
    }
};

} // namespace blog::api
