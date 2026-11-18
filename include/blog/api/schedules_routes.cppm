export module blog_api:schedules_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /schedules. */
class SchedulesRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::SchedulesRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/schedules"}
            .add_router(
                Router{router, "/:id"}
                    .add_route(
                        Route{"/:id"}
                            .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::schedules::put_detail::PutDetailSchedulesHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost :id PUT")
                    )
            );
    }
};

} // namespace blog::api
