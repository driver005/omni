export module blog_handlers_routes:notifications_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

namespace notifications_routes_detail {

// Ghost's API framework runs `permissions: true` for
// {docName:'notifications', method:browse|add|destroy} after authAdminApi.
// Dispatch by verb because GET/POST share one Router and DELETE is a sub-route.
inline void require_permission(interfaces::io::IRequest& req,
                               interfaces::io::IResponse& res,
                               interfaces::NextFn&& next,
                               std::function<void()> send)
{
    const auto method = std::string{req.find_header(interfaces::io::types::Token::METHOD)};
    if (method == "POST") {
        blog::handlers::middleware::require_ghost_permission(
            req, res, std::move(next), std::move(send), "notifications", "add");
    } else if (method == "DELETE") {
        blog::handlers::middleware::require_ghost_permission(
            req, res, std::move(next), std::move(send), "notifications", "destroy");
    } else {
        blog::handlers::middleware::require_ghost_permission(
            req, res, std::move(next), std::move(send), "notifications", "browse");
    }
}

} // namespace notifications_routes_detail

class NotificationsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/notifications"}
            .use(blog::handlers::middleware::content_type_middleware)
            .use(blog::handlers::middleware::require_auth)
            .use(notifications_routes_detail::require_permission)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.notifications_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost notifications GET")
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.notifications_create().handle(req, res, std::move(send));
    })
            .summary("Ghost notifications POST")
            .add_route(
                Route{"/:notification_id"}
                    .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.notifications_destroy().handle(req, res, std::move(send));
    })
                    .summary("Ghost :notification_id DELETE")
            );
    }
};

} // namespace blog::api
