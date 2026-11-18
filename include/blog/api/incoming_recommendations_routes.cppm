export module blog_api:incoming_recommendations_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class IncomingRecommendationsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/incoming_recommendations"}
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.incoming_recommendations_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost incoming_recommendations GET");
    }
};

} // namespace blog::api
