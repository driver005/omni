export module blog_handlers_routes:actions_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class ActionsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/actions"}
            // Ghost routes.js:544 registers only mw.authAdminApi; there is no
            // body/content-type requirement on this GET (no 415 gate).
            .use(blog::handlers::middleware::require_auth)
            .get(
                [&handler](
                    interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    std::function<void()> send
                )
                {
                    handler.actions_browse().handle(req, res, std::move(send));
                }
            )
            .summary("Ghost actions GET");
    }
};

} // namespace blog::api
