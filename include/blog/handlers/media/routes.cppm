export module blog_handlers_routes:media_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;

export namespace blog::api {

class MediaRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/media"}
            .use(blog::handlers::middleware::content_type_middleware)
            .add_router(
                Router{router, "/thumbnail"}
                    .add_route(
                        Route{"/upload"}
                            .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.media_media_thumbnail_upload_put().handle(req, res, std::move(send));
    })
                            .summary("Ghost upload PUT")
                    )
            )
            .add_route(
                Route{"/upload"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.media_media_upload_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api
