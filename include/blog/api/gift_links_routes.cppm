export module blog_api:gift_links_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /gift_links. */
class GiftLinksRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::GiftLinksRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/gift_links"}
            .add_route(
                Route{"/remove_all"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::gift_links::gift_links_remove_all_put::GiftLinksRemoveAllPutGiftLinksHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost remove_all PUT")
            );
    }
};

} // namespace blog::api
