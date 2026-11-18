export module blog_api:site_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /site. */
class SiteRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::SiteRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/site"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::site::browse::BrowseSiteHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost site GET");
    }
};

} // namespace blog::api
