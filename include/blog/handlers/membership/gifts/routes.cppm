export module blog_handlers_routes:gifts_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;

export namespace blog::api {

class GiftsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Ghost's admin app parses bodies globally and has no content-type gate on
        // these no-input custom actions (admin/app.js:33-34). Both controllers declare
        // permissions:{docName:'gifts', method:...} (gifts.js:16-19, 30-33), enforced
        // inside each handler via blog_handlers_middleware::require_ghost_permission.
        // Router-level content_type_middleware / bearer_middleware / require_auth would
        // add a 415/406 gate and a token-presence-only path Ghost does not have, so
        // they are intentionally absent here (same shape as external/slack/routes.cppm).
        Router{router, "/gifts"}
            .add_route(
                Route{"/flush_deliveries"}
                    .put(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.gifts_gifts_flush_deliveries_put()
                                .handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost flush_deliveries PUT")
            )
            .add_route(
                Route{"/flush_reminders"}
                    .put(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.gifts_gifts_flush_reminders_put()
                                .handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost flush_reminders PUT")
            );
    }
};

} // namespace blog::api
