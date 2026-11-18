export module blog_api:session_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /session. */
class SessionRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::SessionRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/session"}
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::session::create::CreateSessionHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost session POST")
            .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::session::bulk_destroy::BulkDestroySessionHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost session DELETE")
            .add_route(
                Route{"/verify"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::session::session_verify_post::SessionVerifyPostSessionHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost verify POST")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::session::session_verify_put::SessionVerifyPutSessionHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost verify PUT")
            );
    }
};

} // namespace blog::api
