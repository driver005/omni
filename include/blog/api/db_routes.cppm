export module blog_api:db_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

/** @brief Ghost routes for /db. */
class DbRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::DbRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        auto* h = &handler;
        Router{router, "/db"}
            .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
            .summary("Ghost db DELETE")
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
            .summary("Ghost db GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
            .summary("Ghost db POST")
            .add_route(
                Route{"/backup"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost backup POST")
            )
            .add_router(
                Router{router, "/media"}
                    .add_route(
                        Route{"/inline"}
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                            .summary("Ghost inline POST")
                    )
            );
    }
};

} // namespace blog::api
