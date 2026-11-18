export module blog_handlers_routes:settings_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class SettingsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/settings"}
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.settings_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost settings GET")
            .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.settings_bulk_edit().handle(req, res, std::move(send));
    })
            .summary("Ghost settings PUT")
            .add_router(
                Router{router, "/access_code"}
                    .add_route(
                        Route{"/regenerate"}
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.settings_settings_access_code_regenerate_post().handle(req, res, std::move(send));
    })
                            .summary("Ghost regenerate POST")
                    )
            )
            .add_router(
                Router{router, "/routes"}
                    .add_route(
                        Route{"/yaml"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.settings_settings_routes_yaml_get().handle(req, res, std::move(send));
    })
                            .summary("Ghost yaml GET")
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.settings_settings_routes_yaml_post().handle(req, res, std::move(send));
    })
                            .summary("Ghost yaml POST")
                    )
            )
            .add_router(
                Router{router, "/stripe"}
                    .add_route(
                        Route{"/connect"}
                            .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.settings_settings_stripe_connect_delete().handle(req, res, std::move(send));
    })
                            .summary("Ghost connect DELETE")
                    )
            )
            .add_route(
                Route{"/verifications"}
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.settings_settings_verifications_put().handle(req, res, std::move(send));
    })
                    .summary("Ghost verifications PUT")
            );
    }
};

} // namespace blog::api
