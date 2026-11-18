export module blog_api:images_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

/** @brief Ghost routes for /images. */
class ImagesRoutes
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
        Router{router, "/images"}
            .add_route(
                Route{"/upload"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api
