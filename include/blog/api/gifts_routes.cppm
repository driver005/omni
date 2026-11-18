export module blog_api:gifts_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /gifts. */
class GiftsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::GiftsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/gifts"}
            .add_route(
                Route{"/flush_deliveries"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::gifts::gifts_flush_deliveries_put::GiftsFlushDeliveriesPutGiftsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost flush_deliveries PUT")
            )
            .add_route(
                Route{"/flush_reminders"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::gifts::gifts_flush_reminders_put::GiftsFlushRemindersPutGiftsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost flush_reminders PUT")
            );
    }
};

} // namespace blog::api
