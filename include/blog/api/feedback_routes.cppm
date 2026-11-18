export module blog_api:feedback_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

/** @brief Ghost routes for /feedback. */
class FeedbackRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::FeedbackRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        auto* h = &handler;
        Router{router, "/feedback"}
            .add_route(
                Route{"/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
            );
    }
};

} // namespace blog::api
