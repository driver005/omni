export module blog_handlers_routes:roles_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class RolesRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Router = utils::openapi::ApiRouter;
        using blog::handlers::middleware::kGhostContextHeader;

        // Ghost GET /roles/ (admin/routes.js:353). Authenticate + authorize +
        // tokenPermissionCheck are represented by require_auth plus the
        // handler's ghost permission gate (docName 'roles', browse).
        // No content_type_middleware: Ghost reads no body on this GET and does
        // not require application/json (serde.cppm:97-108 was a divergence).
        Router{router, "/roles"}
            .use(blog::handlers::middleware::require_auth)
            .get(
                [&handler](
                    interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    std::function<void()> send
                )
                {
                    req.set_header(kGhostContextHeader, "ADMIN_API");
                    handler.roles_browse().handle(req, res, std::move(send));
                }
            )
            .summary("Ghost roles GET");
    }
};

} // namespace blog::api
