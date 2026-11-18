export module blog_api:roles_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /roles. */
class RolesRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::RolesRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/roles"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::roles::browse::BrowseRolesHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost roles GET");
    }
};

} // namespace blog::api
