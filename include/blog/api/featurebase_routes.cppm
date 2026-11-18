export module blog_api:featurebase_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /featurebase. */
class FeaturebaseRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::FeaturebaseRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/featurebase"}
            .add_route(
                Route{"/token"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::featurebase::featurebase_token_get::FeaturebaseTokenGetFeaturebaseHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost token GET")
            );
    }
};

} // namespace blog::api
