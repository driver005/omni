export module blog_api:incoming_recommendations_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /incoming_recommendations. */
class IncomingRecommendationsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::IncomingRecommendationsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/incoming_recommendations"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::incoming_recommendations::browse::BrowseIncomingRecommendationsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost incoming_recommendations GET");
    }
};

} // namespace blog::api
