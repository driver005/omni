export module blog_api:exports_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class ExportsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/exports"}
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.exports_create().handle(req, res, std::move(send));
    })
            .summary("Ghost exports POST")
            .add_route(
                Route{"/download"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.exports_exports_download_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost download GET")
            );
    }
};

} // namespace blog::api
