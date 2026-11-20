export module blog_handlers_routes:db_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

namespace db_routes_detail {

// Ghost mounts only mw.authAdminApi on the five DB routes
// (admin/routes.js:389-401); there is no Content-Type/Accept gate and no
// router-level permission middleware. The controller supplies the permission
// metadata: permissions:true for four ops and {method:'importContent'} for
// inlineMedia (db.js:20,60,92,117-119,141). Map each verb/path to that Ghost
// method and run it through the shared GhostAuthorizer-backed middleware.
[[nodiscard]] inline std::string_view ghost_method(const interfaces::io::IRequest& req)
{
    const auto verb = req.get_method();
    if (verb == "GET") return "exportContent";
    if (verb == "DELETE") return "deleteAllContent";
    const auto path = req.get_path();
    if (path.find("/media/inline") != std::string_view::npos) return "importContent";
    if (path.find("/backup") != std::string_view::npos) return "backupContent";
    return "importContent";
}

inline void require_permission(
    interfaces::io::IRequest& req,
    interfaces::io::IResponse& res,
    interfaces::NextFn&& next,
    std::function<void()> send)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostPermissionsHeader;
    using blog::handlers::middleware::kGhostRoleHeader;

    // No bearer token -> leave the principal ANONYMOUS so the shared authorizer
    // writes Ghost's 401 UnauthorizedError, matching authAdminApi. With a token,
    // mirror the api-framework http(context) wiring: an admin token resolves to
    // the staff Administrator role before the permission check.
    if (!req.get_authorization().empty()) {
        if (req.find_header(kGhostContextHeader).empty()) {
            req.set_header(kGhostContextHeader, "ADMIN_API");
        }
        if (req.find_header(kGhostRoleHeader).empty()) {
            req.set_header(kGhostRoleHeader, "Administrator");
            req.set_header(kGhostPermissionsHeader, "*");
        }
    }
    blog::handlers::middleware::require_ghost_permission(
        req, res, std::move(next), std::move(send), "db", ghost_method(req));
}

} // namespace db_routes_detail

class DbRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/db"}
            .use(db_routes_detail::require_permission)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.db_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost db GET")
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.db_create().handle(req, res, std::move(send));
    })
            .summary("Ghost db POST")
            .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.db_bulk_destroy().handle(req, res, std::move(send));
    })
            .summary("Ghost db DELETE")
            .add_route(
                Route{"/backup"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.db_db_backup_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost backup POST")
            )
            .add_router(
                Router{router, "/media"}
                    .use(db_routes_detail::require_permission)
                    .add_route(
                        Route{"/inline"}
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.db_db_media_inline_post().handle(req, res, std::move(send));
    })
                            .summary("Ghost inline POST")
                    )
            );
    }
};

} // namespace blog::api
