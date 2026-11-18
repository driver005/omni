export module blog_api:feedback_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

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
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/feedback"}
            .add_route(
                Route{"/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::feedback::read_by_id::ReadByIdFeedbackHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
            );
    }
};

} // namespace blog::api
