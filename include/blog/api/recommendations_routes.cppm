export module blog_api:recommendations_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /recommendations. */
class RecommendationsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::RecommendationsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/recommendations"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::recommendations::browse::BrowseRecommendationsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost recommendations GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::recommendations::create::CreateRecommendationsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost recommendations POST")
            .add_route(
                Route{"/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::recommendations::read_by_id::ReadByIdRecommendationsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::recommendations::edit::EditRecommendationsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::recommendations::destroy::DestroyRecommendationsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
            )
            .add_route(
                Route{"/check"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::recommendations::recommendations_check_post::RecommendationsCheckPostRecommendationsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost check POST")
            );
    }
};

} // namespace blog::api
