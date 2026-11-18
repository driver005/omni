export module blog_api:emails_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /emails. */
class EmailsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::EmailsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/emails"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::emails::browse::BrowseEmailsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost emails GET")
            .add_router(
                Router{router, "/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::emails::read_by_id::ReadByIdEmailsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .add_route(
                        Route{"/analytics"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::emails::analytics_get::AnalyticsGetEmailsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost analytics GET")
                            .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::emails::analytics_put::AnalyticsPutEmailsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost analytics PUT")
                    )
                    .add_route(
                        Route{"/batches"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::emails::batches_get::BatchesGetEmailsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost batches GET")
                    )
                    .add_route(
                        Route{"/recipient-failures"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::emails::recipient_failures_get::RecipientFailuresGetEmailsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost recipient-failures GET")
                    )
                    .add_route(
                        Route{"/retry"}
                            .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::emails::emails_id_retry_put::EmailsIdRetryPutEmailsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost retry PUT")
                    )
                    .add_route(
                        Route{"/status"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::emails::emails_id_status_get::EmailsIdStatusGetEmailsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost status GET")
                    )
            )
            .add_route(
                Route{"/analytics"}
                    .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::emails::analytics_delete::AnalyticsDeleteEmailsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost analytics DELETE")
            );
    }
};

} // namespace blog::api
