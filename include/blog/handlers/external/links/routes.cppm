export module blog_handlers_routes:links_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class LinksRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/links"}
            // Ghost api-framework content negotiation is relevant to the PUT body,
            // not to the token-authenticated GET browse (which has no request body).
            .use(blog::handlers::middleware::require_auth)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.links_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost links GET")
            .add_route(
                Route{"/bulk"}
                    .use(blog::handlers::middleware::content_type_middleware)
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.links_bulk_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost bulk PUT")
            );
    }
};

} // namespace blog::api
