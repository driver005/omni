export module blog_handlers_routes:oembed_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class OembedRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Ghost: router.get('/oembed', mw.authAdminApi, http(api.oembed.read)).
        // No content-negotiation gate on this GET; controller permissions:false
        // means no docName/permission middleware — only admin API auth.
        Router{router, "/oembed"}
            .use(blog::handlers::middleware::bearer_middleware)
            .use(blog::handlers::middleware::require_auth)
            .get(
                [&handler](
                    interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    std::function<void()> send
                )
                {
                    handler.oembed_browse().handle(req, res, std::move(send));
                }
            )
            .summary("Ghost oembed GET");
    }
};

} // namespace blog::api
