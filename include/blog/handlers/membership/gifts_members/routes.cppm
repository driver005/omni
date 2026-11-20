export module blog_handlers_routes:gifts_members_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

// Members-app gift redemption routes (docs/Ghost/.../web/members/app.js:159-169).
// Deliberately separate from membership/gifts/routes.cppm: Ghost registers these on
// the members app with loadMemberSession only (permissions: false), not the admin
// gift-flush router's bearer/require_auth gate. No C++ member-session middleware
// exists, so member identity/status are carried by request headers; see the two
// redeem steps. Both handlers and this installer are already wired centrally; see
// docs/parity/behavior/gifts-members.md for the strict-parity behavior report.
export namespace blog::api {

class GiftsMembersRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/gifts"}
            .add_route(
                Route{"/:token/redeem"}
                    .get([&handler](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handler.gifts_members_redeem_get().handle(req, res, std::move(send));
                    })
                    .summary("Ghost getRedeemable (gifts-members)")
                    .post([&handler](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handler.gifts_members_redeem_post().handle(req, res, std::move(send));
                    })
                    .summary("Ghost redeem (gifts-members)")
            );
    }
};

} // namespace blog::api
