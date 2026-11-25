export module blog_handlers_routes:tinybird_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class TinybirdRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Ghost binds this route with mw.authAdminApi; admin app parses bodies
        // globally without requiring Content-Type/Accept (admin/app.js:33-34),
        // so a bodyless GET must not be 415/406 gated. require_auth covers the
        // authenticate step; the controller's members:browse permission gate
        // runs in TinybirdTokenGetHandler::handle (tinybird.js:11-14).
        Router{router, "/tinybird"}
            .use(blog::handlers::middleware::require_auth)
            .add_route(
                Route{"/token"}
                    .get(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.tinybird_tinybird_token_get().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost token GET")
            );
    }
};

} // namespace blog::api
