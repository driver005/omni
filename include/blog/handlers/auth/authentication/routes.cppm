export module blog_handlers_routes:authentication_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

namespace authentication_route_detail {

/**
 * Ghost applies mw.authAdminApi per operation, not per path: only PUT /setup and
 * POST /reset are private. The router's trie stores one node per path (duplicate
 * path registrations would be ambiguous), so the auth guard is invoked inside
 * the private verb's handler instead of as route-level middleware.
 */
inline void with_auth(interfaces::io::IRequest& req,
                      interfaces::io::IResponse& res,
                      std::function<void()> send,
                      interfaces::HandlerFn handler)
{
    blog::handlers::middleware::require_auth(
        req, res,
        interfaces::NextFn{[handler = std::move(handler)](
            interfaces::io::IRequest& r,
            interfaces::io::IResponse& s,
            std::function<void()> snd) noexcept {
                handler(r, s, std::move(snd));
            }},
        std::move(send));
}

} // namespace authentication_route_detail

class AuthenticationRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        using authentication_route_detail::with_auth;
        Router{router, "/authentication"}
            .use(blog::handlers::middleware::content_type_middleware)
            // Ghost admin routes.js:452-453 — cors only, permissions:false, no auth.
            .add_route(
                Route{"/invitation"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authentication_authentication_invitation_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost invitation POST")
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authentication_authentication_invitation_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost invitation GET")
            )
            // Ghost admin routes.js:440-451 — public, rate-limited; no auth middleware.
            .add_route(
                Route{"/password_reset"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authentication_authentication_password_reset_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost password_reset POST")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authentication_authentication_password_reset_put().handle(req, res, std::move(send));
    })
                    .summary("Ghost password_reset PUT")
            )
            // Ghost admin routes.js:457 — mw.authAdminApi + staff-token block (in-step).
            .add_route(
                Route{"/reset"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        with_auth(req, res, std::move(send),
            [&handler](interfaces::io::IRequest& r, interfaces::io::IResponse& s,
                       std::function<void()> snd) {
                handler.authentication_authentication_reset_post().handle(r, s, std::move(snd));
            });
    })
                    .summary("Ghost reset POST")
            )
            // Ghost admin routes.js:454-456 — public POST/GET; mw.authAdminApi on PUT only.
            .add_route(
                Route{"/setup"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authentication_authentication_setup_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost setup POST")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        with_auth(req, res, std::move(send),
            [&handler](interfaces::io::IRequest& r, interfaces::io::IResponse& s,
                       std::function<void()> snd) {
                handler.authentication_authentication_setup_put().handle(r, s, std::move(snd));
            });
    })
                    .summary("Ghost setup PUT")
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.authentication_authentication_setup_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost setup GET")
            );
    }
};

} // namespace blog::api
