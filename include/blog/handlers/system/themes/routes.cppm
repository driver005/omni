export module blog_handlers_routes:themes_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;

export namespace blog::api {

class ThemesRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/themes"}
            .use(blog::handlers::middleware::content_type_middleware)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.themes_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost themes GET")
            .add_router(
                Router{router, "/:id"}
                    .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.themes_destroy().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
                    .add_route(
                        Route{"/activate"}
                            .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.themes_themes_name_activate_put().handle(req, res, std::move(send));
    })
                            .summary("Ghost activate PUT")
                    )
                    .add_route(
                        Route{"/download"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.themes_themes_name_download_get().handle(req, res, std::move(send));
    })
                            .summary("Ghost download GET")
                    )
            )
            .add_route(
                Route{"/active"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.themes_themes_active_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost active GET")
            )
            .add_route(
                Route{"/install"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.themes_themes_install_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost install POST")
            )
            .add_route(
                Route{"/upload"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.themes_themes_upload_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api
