export module blog_api:redirects_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /redirects. */
class RedirectsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::RedirectsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/redirects"}
            .add_route(
                Route{"/download"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::redirects::redirects_download_get::RedirectsDownloadGetRedirectsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost download GET")
            )
            .add_route(
                Route{"/upload"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::redirects::redirects_upload_post::RedirectsUploadPostRedirectsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api
