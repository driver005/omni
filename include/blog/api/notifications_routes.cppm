export module blog_api:notifications_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /notifications. */
class NotificationsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::NotificationsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/notifications"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::notifications::browse::BrowseNotificationsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost notifications GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::notifications::create::CreateNotificationsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost notifications POST")
            .add_route(
                Route{"/:id"}
                    .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::notifications::destroy::DestroyNotificationsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
            );
    }
};

} // namespace blog::api
