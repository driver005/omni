export module blog_handlers_routes:email_previews_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;

export namespace blog::api {

class EmailPreviewsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/email_previews"}
            .use(blog::handlers::middleware::content_type_middleware)
            .add_router(
                Router{router, "/posts"}
                    .add_route(
                        Route{"/:id"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.email_previews_get_detail().handle(req, res, std::move(send));
    })
                            .summary("Ghost :id GET")
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.email_previews_post_detail().handle(req, res, std::move(send));
    })
                            .summary("Ghost :id POST")
                    )
            );
    }
};

} // namespace blog::api
