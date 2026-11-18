export module blog_api:links_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /links. */
class LinksRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::LinksRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/links"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::links::browse::BrowseLinksHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost links GET")
            .add_route(
                Route{"/bulk"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::links::bulk_edit::BulkEditLinksHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost bulk PUT")
            );
    }
};

} // namespace blog::api
