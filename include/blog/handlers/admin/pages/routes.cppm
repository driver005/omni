export module blog_handlers_routes:pages_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class PagesRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/pages"}
            .use(blog::handlers::middleware::content_type_middleware)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.pages_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost pages GET")
            .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.pages_bulk_destroy().handle(req, res, std::move(send));
    })
            .summary("Ghost pages DELETE")
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.pages_create().handle(req, res, std::move(send));
    })
            .summary("Ghost pages POST")
            .add_router(
                Router{router, "/:id"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.pages_read_by_id().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.pages_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.pages_destroy().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
                    .add_route(
                        Route{"/copy"}
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.pages_pages_id_copy_post().handle(req, res, std::move(send));
    })
                            .summary("Ghost copy POST")
                    )
                    .add_route(
                        Route{"/gift_links"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.pages_gift_links_get().handle(req, res, std::move(send));
    })
                            .summary("Ghost gift_links GET")
                            .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.pages_gift_links_put().handle(req, res, std::move(send));
    })
                            .summary("Ghost gift_links PUT")
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.pages_gift_links_post().handle(req, res, std::move(send));
    })
                            .summary("Ghost gift_links POST")
                    )
            )
            .add_route(
                Route{"/bulk"}
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.pages_bulk_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost bulk PUT")
            )
            .add_router(
                Router{router, "/slug"}
                    .add_route(
                        Route{"/:id"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.pages_read_by_slug().handle(req, res, std::move(send));
    })
                            .summary("Ghost :id GET")
                    )
            );
    }
};

} // namespace blog::api
