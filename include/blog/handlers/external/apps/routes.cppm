export module blog_handlers_routes:apps_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class AppsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Ghost mounts `authAdminApi` over /apps and /apps/* (admin/routes.js:228)
        // and uses bodyParser.json/urlencoded for write bodies. There is no
        // content-type/accept 415 gate on this router, so only require_auth
        // (the 401 gate) is kept here; per-op permissions are enforced by each
        // handler through require_ghost_permission. The labs `apps` flag has no
        // allowed shared primitive and remains a documented residual.
        Router{router, "/apps"}
            .use(blog::handlers::middleware::require_auth)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.apps_all_collection().handle(req, res, std::move(send));
    })
            .summary("Ghost apps ALL")
            .add_route(
                Route{"/installations"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.apps_app_installations_browse().handle(req, res, std::move(send));
    })
                    .summary("Ghost app installations browse")
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.apps_app_installations_add().handle(req, res, std::move(send));
    })
                    .summary("Ghost app installations add")
            )
            // app-sessions.ts:6-23: custom action, options:['id'], no body.
            // The controller permission {app_installations, read} is enforced by
            // AppSessionsCreateHandler through require_ghost_permission, mirroring
            // apiFramework's permissions stage (app-sessions.ts:18). No session state
            // is stored; the handler rechecks then returns the Ghost app_sessions
            // envelope. cacheInvalidate:false means no X-Cache-Invalidate header.
            .add_route(
                Route{"/installations/:id/sessions"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.app_sessions_sessions_create().handle(req, res, std::move(send));
    })
                    .summary("Ghost app session start POST")
            )
            .add_route(
                Route{"/installations/preview"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.apps_app_installation_previews_add().handle(req, res, std::move(send));
    })
                    .summary("Ghost app installation previews add")
            )
            .add_route(
                Route{"/installations/:id"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.apps_app_installations_read().handle(req, res, std::move(send));
    })
                    .summary("Ghost app installations read")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.apps_app_installations_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost app installations edit")
                    .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.apps_app_installations_destroy().handle(req, res, std::move(send));
    })
                    .summary("Ghost app installations destroy")
            );
    }
};

} // namespace blog::api
