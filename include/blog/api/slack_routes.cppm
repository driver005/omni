export module blog_api:slack_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

/** @brief Ghost routes for /slack. */
class SlackRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::SlackRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        auto* h = &handler;
        Router{router, "/slack"}
            .add_route(
                Route{"/test"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost test POST")
            );
    }
};

} // namespace blog::api
