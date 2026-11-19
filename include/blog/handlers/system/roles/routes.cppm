export module blog_handlers_routes:roles_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class RolesRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/roles"}
            .use(blog::handlers::middleware::content_type_middleware)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.roles_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost roles GET");
    }
};

} // namespace blog::api
