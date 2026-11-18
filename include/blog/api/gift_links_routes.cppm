export module blog_api:gift_links_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

/** @brief Ghost routes for /gift_links. */
class GiftLinksRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::CoreHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        auto* h = &handler;
        Router{router, "/gift_links"}
            .add_route(
                Route{"/remove_all"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost remove_all PUT")
            );
    }
};

} // namespace blog::api
