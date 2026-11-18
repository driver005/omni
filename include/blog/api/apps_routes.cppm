export module blog_api:apps_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /apps. */
class AppsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::AppsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/apps"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::apps::all_collection::AllCollectionAppsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost apps ALL");
    }
};

} // namespace blog::api
