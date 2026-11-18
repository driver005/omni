export module blog_api:media_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /media. */
class MediaRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::MediaRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/media"}
            .add_router(
                Router{router, "/thumbnail"}
                    .add_route(
                        Route{"/upload"}
                            .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::media::media_thumbnail_upload_put::MediaThumbnailUploadPutMediaHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost upload PUT")
                    )
            )
            .add_route(
                Route{"/upload"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::media::media_upload_post::MediaUploadPostMediaHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api
