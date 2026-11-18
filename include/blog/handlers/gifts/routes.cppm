export module blog_handlers_routes:gifts_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class GiftsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/gifts"}
            .add_route(
                Route{"/flush_deliveries"}
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.gifts_gifts_flush_deliveries_put().handle(req, res, std::move(send));
    })
                    .summary("Ghost flush_deliveries PUT")
            )
            .add_route(
                Route{"/flush_reminders"}
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.gifts_gifts_flush_reminders_put().handle(req, res, std::move(send));
    })
                    .summary("Ghost flush_reminders PUT")
            );
    }
};

} // namespace blog::api
