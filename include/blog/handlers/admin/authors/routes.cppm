export module blog_handlers_routes:authors_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class AuthorsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/authors"}
            .use(blog::handlers::middleware::content_type_middleware)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authors_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost authors GET")
            .add_route(
                Route{"/:id"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authors_read_by_id().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
            )
            .add_router(
                Router{router, "/slug"}
                    .add_route(
                        Route{"/:id"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authors_read_by_slug().handle(req, res, std::move(send));
    })
                            .summary("Ghost :id GET")
                    )
            );
    }
};

} // namespace blog::api
