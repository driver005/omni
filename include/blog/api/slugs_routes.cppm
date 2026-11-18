export module blog_api:slugs_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

/** @brief Ghost routes for /slugs. */
class SlugsRoutes
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
        Router{router, "/slugs"}
            .add_router(
                Router{router, "/:type"}
                    .add_router(
                        Router{router, "/:name"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                            .summary("Ghost :name GET")
                            .add_route(
                                Route{"/:id"}
                                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                                    .summary("Ghost :id GET")
                            )
                    )
            );
    }
};

} // namespace blog::api
