export module blog_handlers_routes:slugs_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

/**
 * @brief Ghost admin routes for `slugs.generate`
 * (web/api/endpoints/admin/routes.js:356-357).
 *
 * Strict-parity notes (group 58):
 * - No `content_type_middleware`: Ghost reads no body on this GET and does not
 *   require application/json / JSON Accept (the 415/406 gate was a divergence).
 * - `bearer_middleware` + `require_auth` stand in for `authenticateAdminApi`'s
 *   401; docName/permission enforcement (`permissions:true`, `canThis.generate.slug`)
 *   is performed in the handler through the shared Ghost permission middleware.
 * - `headers.cacheInvalidate:false` means no `X-Cache-Invalidate` header; the
 *   admin-wide `Cache-Control` is written by the handler.
 */
class SlugsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/slugs"}
            .use(blog::handlers::middleware::bearer_middleware)
            .use(blog::handlers::middleware::require_auth)
            .add_router(
                Router{router, "/:type"}.add_router(
                    Router{router, "/:name"}
                        .get(
                            [&handler](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handler.slugs_get_detail().handle(req, res, std::move(send));
                            }
                        )
                        .summary("Ghost /slugs/:type/:name GET")
                        .add_route(
                            Route{"/:id"}
                                .get(
                                    [&handler](
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send
                                    )
                                    {
                                        handler.slugs_get_detail()
                                            .handle(req, res, std::move(send));
                                    }
                                )
                                .summary("Ghost /slugs/:type/:name/:id GET")
                        )
                )
            );
    }
};

} // namespace blog::api
