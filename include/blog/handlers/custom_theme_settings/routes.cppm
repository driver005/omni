export module blog_handlers_routes:custom_theme_settings_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class CustomThemeSettingsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/custom_theme_settings"}
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.custom_theme_settings_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost custom_theme_settings GET")
            .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.custom_theme_settings_bulk_edit().handle(req, res, std::move(send));
    })
            .summary("Ghost custom_theme_settings PUT");
    }
};

} // namespace blog::api
