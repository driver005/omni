export module blog_api:newsletters_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /newsletters. */
class NewslettersRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::NewslettersRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/newsletters"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::newsletters::browse::BrowseNewslettersHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost newsletters GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::newsletters::create::CreateNewslettersHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost newsletters POST")
            .add_route(
                Route{"/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::newsletters::read_by_id::ReadByIdNewslettersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::newsletters::edit::EditNewslettersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
            )
            .add_route(
                Route{"/verifications"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::newsletters::newsletters_verifications_put::NewslettersVerificationsPutNewslettersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost verifications PUT")
            );
    }
};

} // namespace blog::api
