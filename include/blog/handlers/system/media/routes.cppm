export module blog_handlers_routes:media_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class MediaRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Ghost mounts these ops with `mw.authAdminApi` + multer on the admin API app
        // (admin/routes.js:469-482). There is no JSON-only gate: the bodies are
        // `multipart/form-data`. `content_type_middleware` would reject every genuine
        // upload with 415 before the step parses it, so it is deliberately not installed
        // on this router. Authentication stays (`permissions:false` in media.js:13,35
        // means authAdminApi authenticates but runs no canThis stage).
        Router{router, "/media"}
            .use(blog::handlers::middleware::require_auth)
            .add_router(
                Router{router, "/thumbnail"}
                    .use(blog::handlers::middleware::require_auth)
                    .add_route(
                        Route{"/upload"}
                            .put(
                                [&handler](
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send
                                )
                                {
                                    handler.media_media_thumbnail_upload_put()
                                        .handle(req, res, std::move(send));
                                }
                            )
                            .summary("Ghost upload PUT")
                    )
            )
            .add_route(
                Route{"/upload"}
                    .post(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.media_media_upload_post().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api
