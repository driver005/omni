export module blog_handlers_routes:featurebase_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;

export namespace blog::api {

class FeaturebaseRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/featurebase"}
            .use(blog::handlers::middleware::content_type_middleware)
            .add_route(
                Route{"/token"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.featurebase_featurebase_token_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost token GET")
            );
    }
};

} // namespace blog::api
