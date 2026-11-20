export module blog_handlers_routes:featurebase_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class FeaturebaseRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Ghost binds this route with mw.authAdminApi but the op declares
        // permissions:false (featurebase.js:36), so no permission gate belongs
        // here — require_ghost_permission is deliberately not installed. Ghost's
        // admin app parses bodies globally without requiring Content-Type/Accept
        // (admin/app.js:33-34), so a bodyless GET must not be 415/406 gated.
        Router{router, "/featurebase"}
            .use(blog::handlers::middleware::require_auth)
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
