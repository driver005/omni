export module blog_handlers_routes:announcements_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;

export namespace blog::api {

/** @brief Members-app announcement route: GET /api/announcement. */
class AnnouncementsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = core::router::Route<>;
        using Router = core::router::Router<>;

        // Ghost mounts the announcement router at /api/announcement with an
        // optional member-session loader (members/app.js:172). The C++ server
        // has no per-app mount layer, so the path is bound verbatim; member
        // context is carried by X-Member-Status (see the execute step). No
        // JSON serde gate: Ghost's GET route accepts any Content-Type/Accept,
        // so content_type_middleware (415/406) is deliberately not installed.
        Router{router, "/api/announcement"}.add_route(
            Route{"/"}.get(
                [&handler](
                    interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    std::function<void()> send
                )
                {
                    handler.announcements_announcement_get().handle(req, res, std::move(send));
                }
            )
        );
    }
};

} // namespace blog::api
