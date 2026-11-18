export module blog_handlers_routes:slack_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class SlackRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/slack"}
            .add_route(
                Route{"/test"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.slack_slack_test_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost test POST")
            );
    }
};

} // namespace blog::api
