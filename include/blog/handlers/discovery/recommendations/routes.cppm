export module blog_handlers_routes:recommendations_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class RecommendationsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Ghost enforces per-route auth in the controller pipeline: admin routes
        // use mw.authAdminApi + permissions:true, the content browse is public,
        // and the members tracking routes use loadMemberSession. Each C++ step
        // evaluates the same policy through core::auth::ghost::GhostAuthorizer,
        // so no router-wide bearer check is installed here.
        Router{router, "/recommendations"}
            .use(blog::handlers::middleware::content_type_middleware)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.recommendations_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost recommendations GET")
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.recommendations_create().handle(req, res, std::move(send));
    })
            .summary("Ghost recommendations POST")
            .add_route(
                Route{"/:id"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.recommendations_read_by_id().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.recommendations_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.recommendations_destroy().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
            )
            .add_route(
                Route{"/check"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.recommendations_recommendations_check_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost check POST")
            )
            // Members-app click side effects (members/app.js:174-186). The
            // members-app mount prefix is supplied by the host router.
            .add_route(
                Route{"/:id/clicked"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.recommendations_browse().handle_clicked(req, res, std::move(send));
    })
                    .summary("Ghost trackClicked POST")
            )
            .add_route(
                Route{"/:id/subscribed"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.recommendations_browse().handle_subscribed(req, res, std::move(send));
    })
                    .summary("Ghost trackSubscribed POST")
            );
    }
};

} // namespace blog::api
