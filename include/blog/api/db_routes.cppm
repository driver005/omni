export module blog_api:db_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /db. */
class DbRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::DbRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/db"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::db::browse::BrowseDbHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost db GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::db::create::CreateDbHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost db POST")
            .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::db::bulk_destroy::BulkDestroyDbHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost db DELETE")
            .add_route(
                Route{"/backup"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::db::db_backup_post::DbBackupPostDbHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost backup POST")
            )
            .add_router(
                Router{router, "/media"}
                    .add_route(
                        Route{"/inline"}
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::db::db_media_inline_post::DbMediaInlinePostDbHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost inline POST")
                    )
            );
    }
};

} // namespace blog::api
