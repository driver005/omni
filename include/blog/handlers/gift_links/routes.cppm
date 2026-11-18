export module blog_handlers_routes:gift_links_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class GiftLinksRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/gift_links"}
            .add_route(
                Route{"/remove_all"}
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.gift_links_gift_links_remove_all_put().handle(req, res, std::move(send));
    })
                    .summary("Ghost remove_all PUT")
            );
    }
};

} // namespace blog::api
