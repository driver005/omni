export module blog_api:settings_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /settings. */
class SettingsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::SettingsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/settings"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::settings::browse::BrowseSettingsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost settings GET")
            .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::settings::bulk_edit::BulkEditSettingsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost settings PUT")
            .add_router(
                Router{router, "/access_code"}
                    .add_route(
                        Route{"/regenerate"}
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::settings::settings_access_code_regenerate_post::SettingsAccessCodeRegeneratePostSettingsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost regenerate POST")
                    )
            )
            .add_router(
                Router{router, "/routes"}
                    .add_route(
                        Route{"/yaml"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::settings::settings_routes_yaml_get::SettingsRoutesYamlGetSettingsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost yaml GET")
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::settings::settings_routes_yaml_post::SettingsRoutesYamlPostSettingsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost yaml POST")
                    )
            )
            .add_router(
                Router{router, "/stripe"}
                    .add_route(
                        Route{"/connect"}
                            .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::settings::settings_stripe_connect_delete::SettingsStripeConnectDeleteSettingsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost connect DELETE")
                    )
            )
            .add_route(
                Route{"/verifications"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::settings::settings_verifications_put::SettingsVerificationsPutSettingsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost verifications PUT")
            );
    }
};

} // namespace blog::api
