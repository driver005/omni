export module blog_handlers_routes:users_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

namespace users_route_detail {

/**
 * @brief Ghost permission gate for one users operation.
 *
 * Ghost applies `permissions` at the controller (operation) level, not at the
 * router level. The shared ghost-permission middleware is invoked per handler
 * with the operation's docName/method, and writes Ghost's NoPermissionError /
 * UnauthorizedError envelope itself on denial (so `next` is not called).
 *
 * The mock has no token-claims decoder, so a valid admin bearer is mapped to
 * Ghost's Administrator principal by default. An explicit `x-blog-*` header set
 * by the caller is never overwritten, which keeps restricted roles testable.
 */
inline void with_permission(interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send,
                            std::string_view docName,
                            std::string_view method,
                            interfaces::HandlerFn handler)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostRoleHeader;

    if (req.find_header(kGhostContextHeader).empty()) {
        req.set_header(kGhostContextHeader, "ADMIN_API");
    }
    if (req.find_header(kGhostRoleHeader).empty() && !req.get_authorization().empty()) {
        req.set_header(kGhostRoleHeader, "Administrator");
    }

    blog::handlers::middleware::require_ghost_permission(
        req, res,
        interfaces::NextFn{[handler = std::move(handler)](
            interfaces::io::IRequest& r,
            interfaces::io::IResponse& s,
            std::function<void()> snd) noexcept {
                handler(r, s, std::move(snd));
            }},
        std::move(send), docName, method);
}

} // namespace users_route_detail

class UsersRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        using users_route_detail::with_permission;
        Router{router, "/users"}
            .use(blog::handlers::middleware::content_type_middleware)
            .use(blog::handlers::middleware::require_auth)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        with_permission(req, res, std::move(send), "users", "browse",
            [&handler](interfaces::io::IRequest& r, interfaces::io::IResponse& s,
                       std::function<void()> snd) {
                handler.users_browse().handle(r, s, std::move(snd));
            });
    })
            .summary("Ghost users GET")
            .add_router(
                Router{router, "/:id"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        with_permission(req, res, std::move(send), "users", "read",
            [&handler](interfaces::io::IRequest& r, interfaces::io::IResponse& s,
                       std::function<void()> snd) {
                handler.users_read_by_id().handle(r, s, std::move(snd));
            });
    })
                    .summary("Ghost :id GET")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        with_permission(req, res, std::move(send), "users", "edit",
            [&handler](interfaces::io::IRequest& r, interfaces::io::IResponse& s,
                       std::function<void()> snd) {
                handler.users_edit().handle(r, s, std::move(snd));
            });
    })
                    .summary("Ghost :id PUT")
                    .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        with_permission(req, res, std::move(send), "users", "destroy",
            [&handler](interfaces::io::IRequest& r, interfaces::io::IResponse& s,
                       std::function<void()> snd) {
                handler.users_destroy().handle(r, s, std::move(snd));
            });
    })
                    .summary("Ghost :id DELETE")
                    .add_route(
                        Route{"/token"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        with_permission(req, res, std::move(send), "users", "browse",
            [&handler](interfaces::io::IRequest& r, interfaces::io::IResponse& s,
                       std::function<void()> snd) {
                handler.users_users_id_token_get().handle(r, s, std::move(snd));
            });
    })
                            .summary("Ghost token GET")
                            .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        with_permission(req, res, std::move(send), "users", "browse",
            [&handler](interfaces::io::IRequest& r, interfaces::io::IResponse& s,
                       std::function<void()> snd) {
                handler.users_users_id_token_put().handle(r, s, std::move(snd));
            });
    })
                            .summary("Ghost token PUT")
                    )
            )
            .add_router(
                Router{router, "/email"}
                    .add_route(
                        Route{"/:slug"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        with_permission(req, res, std::move(send), "users", "read",
            [&handler](interfaces::io::IRequest& r, interfaces::io::IResponse& s,
                       std::function<void()> snd) {
                handler.users_read_by_email().handle(r, s, std::move(snd));
            });
    })
                            .summary("Ghost :email GET")
                    )
            )
            .add_route(
                Route{"/owner"}
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        with_permission(req, res, std::move(send), "roles", "edit",
            [&handler](interfaces::io::IRequest& r, interfaces::io::IResponse& s,
                       std::function<void()> snd) {
                handler.users_users_owner_put().handle(r, s, std::move(snd));
            });
    })
                    .summary("Ghost owner PUT")
            )
            .add_route(
                Route{"/password"}
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        with_permission(req, res, std::move(send), "user", "edit",
            [&handler](interfaces::io::IRequest& r, interfaces::io::IResponse& s,
                       std::function<void()> snd) {
                handler.users_users_password_put().handle(r, s, std::move(snd));
            });
    })
                    .summary("Ghost password PUT")
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
        with_permission(req, res, std::move(send), "users", "read",
            [&handler](interfaces::io::IRequest& r, interfaces::io::IResponse& s,
                       std::function<void()> snd) {
                handler.users_read_by_slug().handle(r, s, std::move(snd));
            });
    })
                            .summary("Ghost :slug GET")
                    )
            );
    }
};

} // namespace blog::api
