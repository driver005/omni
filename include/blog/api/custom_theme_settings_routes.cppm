export module blog_api:custom_theme_settings_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

/** @brief Ghost routes for /custom_theme_settings. */
class CustomThemeSettingsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::CustomThemeSettingsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        auto* h = &handler;
        Router{router, "/custom_theme_settings"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
            .summary("Ghost custom_theme_settings GET")
            .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
            .summary("Ghost custom_theme_settings PUT");
    }
};

} // namespace blog::api
