export module blog_handlers_routes:mentions_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class MentionsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/mentions"}
            .use(blog::handlers::middleware::content_type_middleware)
            .use(blog::handlers::middleware::require_auth)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.mentions_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost mentions GET");

        // Ghost's webmentions router parses application/x-www-form-urlencoded and
        // gates with webmentionsLimiter + cacheControl('private'); no JSON
        // Content-Type/Accept requirement and no authentication on this route.
        Router{router, "/receive"}
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.mentions_receive().handle(req, res, std::move(send));
    })
            .summary("Ghost webmention receive POST");
    }
};

} // namespace blog::api
