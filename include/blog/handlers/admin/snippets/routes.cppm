export module blog_handlers_routes:snippets_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class SnippetsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/snippets"}
            .use(blog::handlers::middleware::content_type_middleware)
            .use(blog::handlers::middleware::require_auth)
            .get(
                [&handler](
                    interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    std::function<void()> send
                )
                {
                    handler.snippets_browse().handle(req, res, std::move(send));
                }
            )
            .summary("Ghost snippets GET")
            .post(
                [&handler](
                    interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    std::function<void()> send
                )
                {
                    handler.snippets_create().handle(req, res, std::move(send));
                }
            )
            .summary("Ghost snippets POST")
            .add_route(
                Route{"/:id"}
                    .get(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.snippets_read_by_id().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost :id GET")
                    .put(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.snippets_edit().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost :id PUT")
                    .delt(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.snippets_destroy().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost :id DELETE")
            );
    }
};

} // namespace blog::api
