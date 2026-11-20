export module blog_handlers_routes:incoming_recommendations_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class IncomingRecommendationsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Ghost's admin app uses body parsers, not a Content-Type gate, and the
        // controller applies `mw.authAdminApi` + `permissions: true` (docName
        // "recommendations", method "browse") inside its own pipeline. The old
        // router-level content_type_middleware / bearer_middleware / require_auth
        // chain is removed; BrowseHandler now enforces the Ghost permission service
        // via core::auth::ghost + require_ghost_permission. See
        // docs/parity/behavior/incoming-recommendations.md.
        Router{router, "/incoming_recommendations"}
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.incoming_recommendations_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost incoming_recommendations GET");
    }
};

} // namespace blog::api
