export module blog_api:db_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class DbRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/db"}
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.db_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost db GET")
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.db_create().handle(req, res, std::move(send));
    })
            .summary("Ghost db POST")
            .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.db_bulk_destroy().handle(req, res, std::move(send));
    })
            .summary("Ghost db DELETE")
            .add_route(
                Route{"/backup"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.db_db_backup_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost backup POST")
            )
            .add_router(
                Router{router, "/media"}
                    .add_route(
                        Route{"/inline"}
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.db_db_media_inline_post().handle(req, res, std::move(send));
    })
                            .summary("Ghost inline POST")
                    )
            );
    }
};

} // namespace blog::api
