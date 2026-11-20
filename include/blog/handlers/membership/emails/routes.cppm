export module blog_handlers_routes:emails_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class EmailsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/emails"}
            .use(blog::handlers::middleware::content_type_middleware)
            .use(blog::handlers::middleware::require_auth)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.emails_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost emails GET")
            .add_router(
                Router{router, "/:id"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.emails_read_by_id().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .add_route(
                        Route{"/analytics"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.emails_analytics_get().handle(req, res, std::move(send));
    })
                            .summary("Ghost analytics GET")
                            .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.emails_analytics_put().handle(req, res, std::move(send));
    })
                            .summary("Ghost analytics PUT")
                    )
                    .add_route(
                        Route{"/batches"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.emails_batches_get().handle(req, res, std::move(send));
    })
                            .summary("Ghost batches GET")
                    )
                    .add_route(
                        Route{"/recipient-failures"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.emails_recipient_failures_get().handle(req, res, std::move(send));
    })
                            .summary("Ghost recipient-failures GET")
                    )
                    .add_route(
                        Route{"/retry"}
                            .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.emails_emails_id_retry_put().handle(req, res, std::move(send));
    })
                            .summary("Ghost retry PUT")
                    )
                    .add_route(
                        Route{"/status"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.emails_emails_id_status_get().handle(req, res, std::move(send));
    })
                            .summary("Ghost status GET")
                    )
            )
            .add_route(
                Route{"/analytics"}
                    .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.emails_analytics_delete().handle(req, res, std::move(send));
    })
                    .summary("Ghost analytics DELETE")
            );
    }
};

} // namespace blog::api
