export module blog_api:tiers_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /tiers. */
class TiersRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::TiersRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/tiers"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::tiers::browse::BrowseTiersHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost tiers GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::tiers::create::CreateTiersHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost tiers POST")
            .add_route(
                Route{"/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::tiers::read_by_id::ReadByIdTiersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::tiers::edit::EditTiersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
            );
    }
};

} // namespace blog::api
