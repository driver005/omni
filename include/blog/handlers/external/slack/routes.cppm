export module blog_handlers_routes:slack_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;

export namespace blog::api {

class SlackRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Ghost's admin app parses bodies globally (admin/app.js:33-34) and has no
        // content-type rejection on POST /slack/test. The controller's only guard is
        // mw.authAdminApi (permissions:false); it is enforced inside the handler via
        // blog_handlers_middleware::require_ghost_permission (docName "slack").
        // No router-level content_type_middleware / require_auth: those would add
        // the 415/406 gate Ghost does not have on this route.
        Router{router, "/slack"}.add_route(
            Route{"/test"}
                .post(
                    [&handler](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handler.slack_slack_test_post().handle(req, res, std::move(send));
                    }
                )
                .summary("Ghost test POST")
        );
    }
};

} // namespace blog::api
