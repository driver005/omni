export module blog_api:upgrades_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /upgrades. */
class UpgradesRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::UpgradesRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/upgrades"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::upgrades::browse::BrowseUpgradesHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost upgrades GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::upgrades::create::CreateUpgradesHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost upgrades POST")
            .add_route(
                Route{"/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::upgrades::read_by_id::ReadByIdUpgradesHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
            );
    }
};

} // namespace blog::api
