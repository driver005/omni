export module blog_handlers_routes:tiers_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class TiersRoutes
{
public:
    /**
     * @brief `content=true` installs only the public GET /tiers route with
     * bearer + optional auth (content API); default installs the full admin
     * set with require_auth.
     *
     * The x-blog-context header is the single content-vs-admin signal: the
     * browse handler/step read it to apply the content-API `active:true`
     * filter and to decide whether to emit X-Cache-Invalidate.
     */
    void install(
        core::router::RouterContext<>& router,
        blog::api::BlogHandlers& handler,
        bool content = false
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        using blog::handlers::middleware::kGhostContextHeader;

        if (content) {
            Router{router, "/tiers"}
                .use(blog::handlers::middleware::content_type_middleware)
                .use(blog::handlers::middleware::bearer_middleware)
                .use(blog::handlers::middleware::optional_auth)
                .get(
                    [&handler](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        req.set_header(kGhostContextHeader, "CONTENT_API");
                        handler.tiers_browse().handle(req, res, std::move(send));
                    }
                )
                .summary("Ghost content tiers GET");
            return;
        }

        Router{router, "/tiers"}
            .use(blog::handlers::middleware::content_type_middleware)
            .use(blog::handlers::middleware::require_auth)
            .get(
                [&handler](
                    interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    std::function<void()> send
                )
                {
                    req.set_header(kGhostContextHeader, "ADMIN_API");
                    handler.tiers_browse().handle(req, res, std::move(send));
                }
            )
            .summary("Ghost tiers GET")
            .post(
                [&handler](
                    interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    std::function<void()> send
                )
                {
                    req.set_header(kGhostContextHeader, "ADMIN_API");
                    handler.tiers_create().handle(req, res, std::move(send));
                }
            )
            .summary("Ghost tiers POST")
            .add_route(
                Route{"/:id"}
                    .get(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            req.set_header(kGhostContextHeader, "ADMIN_API");
                            handler.tiers_read_by_id().handle(req, res, std::move(send));
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
                            req.set_header(kGhostContextHeader, "ADMIN_API");
                            handler.tiers_edit().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost :id PUT")
            );
    }
};

} // namespace blog::api
