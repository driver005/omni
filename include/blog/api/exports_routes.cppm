export module blog_api:exports_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /exports. */
class ExportsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::ExportsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/exports"}
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::exports::create::CreateExportsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost exports POST")
            .add_route(
                Route{"/download"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::exports::exports_download_get::ExportsDownloadGetExportsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost download GET")
            );
    }
};

} // namespace blog::api
