export module blog_handlers_routes:exports_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;

export namespace blog::api {

/**
 * Ghost mounts both operations under the Admin API prefix with `mw.authAdminApi`
 * and controller permissions `{docName:'db', method:'exportContent'}`
 * (`admin/routes.js:404-405`; `exports.js:129-132,185-188`). Authentication and
 * permission checks therefore live in the handlers via the shared
 * `blog_handlers_middleware::require_ghost_permission` gate — not in a
 * router-level Content-Type/Accept gate, which Ghost's API app does not impose.
 */
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
