export module blog_handlers_routes:authors_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

namespace detail {

// Ghost content/routes.js: the Content API router runs CORS then
// authenticatePublic; the controller then declares `permissions:true`.
// core_auth::ghost resolves public content reads for the CONTENT_API surface,
// so mark the request as that surface before the shared permission guard.
inline void mark_content_surface(interfaces::io::IRequest& req,
                                 interfaces::io::IResponse& res)
{
    res.set_header(std::string_view{"access-control-allow-origin"}, std::string_view{"*"});
    if (req.find_header(blog::handlers::middleware::kGhostContextHeader).empty()) {
        req.set_header(blog::handlers::middleware::kGhostContextHeader,
                       std::string_view{"CONTENT_API"});
    }
}

} // namespace detail

class AuthorsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/authors"}
            .use([](interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    interfaces::NextFn&& next,
                    std::function<void()> send) {
                detail::mark_content_surface(req, res);
                blog::handlers::middleware::require_ghost_permission(
                    req, res, std::move(next), std::move(send), "authors", "browse");
            })
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
                        Route{"/:slug"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authors_read_by_slug().handle(req, res, std::move(send));
    })
                            .summary("Ghost :slug GET")
                    )
            );
    }
};

} // namespace blog::api
