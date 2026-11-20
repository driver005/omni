export module blog_handlers_routes:themes_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;

export namespace blog::api {

/**
 * Ghost mounts these seven routes under the Admin API prefix with a single
 * `mw.authAdminApi` middleware and controller-level `permissions:true` /
 * `permissions:{method:...}` (`admin/routes.js:360-378`, `themes.js:11-178`).
 * Authentication and permission checks therefore live in the handlers through
 * the registered `blog_handlers_middleware:ghost_permission` stage, not in a
 * router-level `x-blog-permissions` helper. There is no JSON `Content-Type`
 * gate: Ghost's `/themes/upload` is multipart/form-data, and the Admin API app
 * uses body parsers only (mirroring `webhooks/routes.cppm` and
 * `system/redirects/routes.cppm`).
 */
class ThemesRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/themes"}
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.themes_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost themes GET")
            .add_router(
                Router{router, "/:id"}
                    .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.themes_destroy().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
                    .add_route(
                        Route{"/activate"}
                            .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.themes_themes_name_activate_put().handle(req, res, std::move(send));
    })
                            .summary("Ghost activate PUT")
                    )
                    .add_route(
                        Route{"/download"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.themes_themes_name_download_get().handle(req, res, std::move(send));
    })
                            .summary("Ghost download GET")
                    )
            )
            .add_route(
                Route{"/active"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.themes_themes_active_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost active GET")
            )
            .add_route(
                Route{"/install"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.themes_themes_install_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost install POST")
            )
            .add_route(
                Route{"/upload"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.themes_themes_upload_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api
