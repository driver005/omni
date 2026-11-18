export module blog_api:schedules_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class SchedulesRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/schedules"}
            .add_router(
                Router{router, "/:id"}
                    .add_route(
                        Route{"/:id"}
                            .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.schedules_put_detail().handle(req, res, std::move(send));
    })
                            .summary("Ghost :id PUT")
                    )
            );
    }
};

} // namespace blog::api
