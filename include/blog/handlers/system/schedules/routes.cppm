export module blog_handlers_routes:schedules_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;

export namespace blog::api {

class SchedulesRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Ghost: router.put('/schedules/:resource/:id', mw.authAdminApiWithUrl,
        // http(api.schedules.publish)). Ghost admin uses body parsers, not a
        // Content-Type gate; the auth/permission pipeline (validation ->
        // permissions -> query) is enforced inside PutDetailHandler through the
        // registered core_auth:ghost helper, so no router-level middleware is
        // installed here.
        Router{router, "/schedules"}
            .add_router(
                Router{router, "/:resource"}
                    .add_route(
                        Route{"/:id"}
                            .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.schedules_put_detail().handle(req, res, std::move(send));
    })
                            .summary("Ghost :resource/:id PUT")
                    )
            );
    }
};

} // namespace blog::api
