export module blog_handlers_routes:app_sessions_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class AppSessionsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Ghost binds this action on appsRouter with authAdminApi + the apps labs
        // gate (admin/routes.js:227-230). The token gate stays at the router; the
        // controller permission {app_installations, read} is enforced by the
        // handler (app-sessions.ts:18). No content-type/accept gate: start has
        // no body and Ghost reads no Content-Type for it.
        Router{router, "/apps"}
            .use(blog::handlers::middleware::require_auth)
            .add_route(
                Route{"/installations/:id/sessions"}
                    .post(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.app_sessions_sessions_create()
                                .handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost app session start POST")
            );
    }
};

} // namespace blog::api
