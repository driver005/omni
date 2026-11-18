export module blog_api:offers_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /offers. */
class OffersRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::OffersRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/offers"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::offers::browse::BrowseOffersHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost offers GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::offers::create::CreateOffersHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost offers POST")
            .add_route(
                Route{"/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::offers::read_by_id::ReadByIdOffersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::offers::edit::EditOffersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
            );
    }
};

} // namespace blog::api
