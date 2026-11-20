export module blog_handlers_routes:config_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class ConfigRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Ghost parses bodies globally and serves GET /config without a Content-Type
        // requirement (admin/app.js:33-44); no content_type_middleware 415/406 gate.
        // config.read declares permissions:false, so no require_ghost_permission stage.
        Router{router, "/config"}
            .use(blog::handlers::middleware::require_auth)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.config_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost config GET")
            .add_route(
                Route{"/featurebase"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.config_config_featurebase_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost featurebase GET")
            );
    }
};

} // namespace blog::api
