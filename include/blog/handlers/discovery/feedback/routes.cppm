export module blog_handlers_routes:feedback_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class FeedbackRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // admin API: GET /feedback/:id -> feedbackMembers.browse
        // Ghost binds mw.authAdminApi + permissions:{posts,browse}; both are
        // enforced by ReadByIdHandler via core::auth::ghost +
        // require_ghost_permission. Router-level content_type_middleware is
        // deliberately absent (the Ghost route has no body / Content-Type gate);
        // require_auth mirrors mw.authAdminApi's missing-token 401.
        Router{router, "/feedback"}
            .use(blog::handlers::middleware::require_auth)
            .add_route(
                Route{"/:id"}
                    .get(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.feedback_read_by_id().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost feedback members browse")
            );

        // members app: POST /api/feedback -> feedbackMembers.add.
        // permissions:false — no permission middleware; the authenticated
        // member identity is resolved by the members-app session (modelled by
        // the ambient member-id header) inside the step.
        Router{router, "/api"}
            .use(blog::handlers::middleware::bearer_middleware)
            .use(blog::handlers::middleware::optional_auth)
            .add_route(
                Route{"/feedback"}
                    .post(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.feedback_add().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost feedback members add")
            );
    }
};

} // namespace blog::api
