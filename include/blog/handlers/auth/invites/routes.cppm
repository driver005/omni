export module blog_handlers_routes:invites_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class InvitesRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/invites"}
            .use(blog::handlers::middleware::content_type_middleware)
            .use(blog::handlers::middleware::bearer_middleware)
            .use(blog::handlers::middleware::require_auth)
            .get(
                [&handler](
                    interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    std::function<void()> send
                )
                {
                    handler.invites_browse().handle(req, res, std::move(send));
                }
            )
            .summary("Ghost invites GET")
            .post(
                [&handler](
                    interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    std::function<void()> send
                )
                {
                    handler.invites_create().handle(req, res, std::move(send));
                }
            )
            .summary("Ghost invites POST")
            .add_route(
                Route{"/:id"}
                    .get(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.invites_read_by_id().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost :id GET")
                    .delt(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.invites_destroy().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost :id DELETE")
            );
    }
};

} // namespace blog::api
