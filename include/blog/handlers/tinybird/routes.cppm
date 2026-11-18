export module blog_handlers_routes:tinybird_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class TinybirdRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/tinybird"}
            .add_route(
                Route{"/token"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.tinybird_tinybird_token_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost token GET")
            );
    }
};

} // namespace blog::api
