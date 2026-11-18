export module blog_api:themes_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /themes. */
class ThemesRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::ThemesRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/themes"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::themes::browse::BrowseThemesHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost themes GET")
            .add_router(
                Router{router, "/:id"}
                    .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::themes::destroy::DestroyThemesHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
                    .add_route(
                        Route{"/activate"}
                            .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::themes::themes_name_activate_put::ThemesNameActivatePutThemesHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost activate PUT")
                    )
                    .add_route(
                        Route{"/download"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::themes::themes_name_download_get::ThemesNameDownloadGetThemesHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost download GET")
                    )
            )
            .add_route(
                Route{"/active"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::themes::themes_active_get::ThemesActiveGetThemesHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost active GET")
            )
            .add_route(
                Route{"/install"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::themes::themes_install_post::ThemesInstallPostThemesHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost install POST")
            )
            .add_route(
                Route{"/upload"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::themes::themes_upload_post::ThemesUploadPostThemesHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api
