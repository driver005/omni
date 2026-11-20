export module blog_handlers_routes:identities_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

namespace identities_routes_detail {

// Ghost's router.get('/identities', mw.authAdminApi, http(api.identities.read))
// (admin/routes.js:437). authAdminApi resolves the admin token to a staff user
// and the controller adds permissions:true / docName 'identities' with the
// identity:read permission (identities.js:5,10). With no token store in this
// folder, a presented admin token maps to the Ghost Administrator role, then
// the shared GhostAuthorizer enforces the permission and writes Ghost-shaped
// 401/403 envelopes on denial. Router-level content_type_middleware and the
// bare require_auth are gone: Ghost has no Content-Type gate on this GET and
// permission handling owns the unauthorized decision.
inline void require_permission(
    interfaces::io::IRequest& req,
    interfaces::io::IResponse& res,
    interfaces::NextFn&& next,
    std::function<void()> send)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostPermissionsHeader;
    using blog::handlers::middleware::kGhostRoleHeader;

    if (req.find_header(kGhostContextHeader).empty()) {
        req.set_header(kGhostContextHeader, "ADMIN_API");
    }
    if (!req.get_authorization().empty() && req.find_header(kGhostRoleHeader).empty()) {
        req.set_header(kGhostRoleHeader, "Administrator");
        req.set_header(kGhostPermissionsHeader, "*");
    }
    blog::handlers::middleware::require_ghost_permission(
        req, res, std::move(next), std::move(send), "identities", "read");
}

} // namespace identities_routes_detail

class IdentitiesRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/identities"}
            .use(identities_routes_detail::require_permission)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.identities_read().handle(req, res, std::move(send));
    })
            .summary("Ghost identities GET");
    }
};

} // namespace blog::api
