export module blog_api:email_previews_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /email_previews. */
class EmailPreviewsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::EmailPreviewsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/email_previews"}
            .add_router(
                Router{router, "/posts"}
                    .add_route(
                        Route{"/:id"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::email_previews::get_detail::GetDetailEmailPreviewsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost :id GET")
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::email_previews::post_detail::PostDetailEmailPreviewsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost :id POST")
                    )
            );
    }
};

} // namespace blog::api
