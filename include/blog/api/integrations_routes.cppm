export module blog_api:integrations_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /integrations. */
class IntegrationsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::IntegrationsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/integrations"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::integrations::browse::BrowseIntegrationsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost integrations GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::integrations::create::CreateIntegrationsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost integrations POST")
            .add_router(
                Router{router, "/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::integrations::read_by_id::ReadByIdIntegrationsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::integrations::edit::EditIntegrationsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::integrations::destroy::DestroyIntegrationsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
                    .add_router(
                        Router{router, "/api_key"}
                            .add_router(
                                Router{router, "/:id"}
                                    .add_route(
                                        Route{"/refresh"}
                                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::integrations::integrations_id_api_key_keyid_refresh_post::IntegrationsIdApiKeyKeyidRefreshPostIntegrationsHandler::handle(req, res, std::move(send));
    })
                                            .summary("Ghost refresh POST")
                                    )
                            )
                    )
            );
    }
};

} // namespace blog::api
