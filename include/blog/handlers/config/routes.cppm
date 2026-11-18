export module blog_handlers_routes:config_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class ConfigRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/config"}
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
