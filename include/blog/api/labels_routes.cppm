export module blog_api:labels_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

/** @brief Ghost routes for /labels. */
class LabelsRoutes
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
        Router{router, "/labels"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
            .summary("Ghost labels GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
            .summary("Ghost labels POST")
            .add_route(
                Route{"/:id"}
                    .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
            )
            .add_router(
                Router{router, "/slug"}
                    .add_route(
                        Route{"/:slug"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                            .summary("Ghost :slug GET")
                    )
            );
    }
};

} // namespace blog::api
