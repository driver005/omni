export module blog_handlers_routes:redirects_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;

export namespace blog::api {

/**
 * Ghost `admin/routes.js:505-512` installs exactly two redirects routes:
 *
 *   GET  /redirects/download  -> mw.authAdminApi + http(api.redirects.download)
 *   POST /redirects/upload    -> mw.authAdminApi + upload.single('redirects')
 *                                + upload.validation({type:'redirects'})
 *                                + http(api.redirects.upload)
 *
 * Neither route declares an input-shape middleware on the C++ side: the shared
 * Ghost permission stage and the multipart/file validation live in the handler
 * and step partitions (see the group 53 implementation record). In particular
 * there is no JSON `Content-Type` gate — Ghost's download is a plain GET with
 * no body, and its upload is multipart/form-data.
 */
class RedirectsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/redirects"}
            .add_route(
                Route{"/download"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.redirects_redirects_download_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost download GET")
            )
            .add_route(
                Route{"/upload"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.redirects_redirects_upload_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api
