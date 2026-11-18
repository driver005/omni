export module blog_api:oembed_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /oembed. */
class OembedRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::OembedRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/oembed"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::oembed::browse::BrowseOembedHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost oembed GET");
    }
};

} // namespace blog::api
