export module blog_api:invites_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /invites. */
class InvitesRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::InvitesRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/invites"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::invites::browse::BrowseInvitesHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost invites GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::invites::create::CreateInvitesHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost invites POST")
            .add_route(
                Route{"/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::invites::read_by_id::ReadByIdInvitesHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::invites::destroy::DestroyInvitesHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
            );
    }
};

} // namespace blog::api
