export module blog_handlers_routes:posts_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class PostsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/posts"}
            .use(blog::handlers::middleware::content_type_middleware)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost posts GET")
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_create().handle(req, res, std::move(send));
    })
            .summary("Ghost posts POST")
            .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_bulk_destroy().handle(req, res, std::move(send));
    })
            .summary("Ghost posts DELETE")
            .add_router(
                Router{router, "/:id"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_read_by_id().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_destroy().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
                    .add_route(
                        Route{"/copy"}
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_copy().handle(req, res, std::move(send));
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
        handler.posts_gift_links_browse().handle(req, res, std::move(send));
    })
                            .summary("Ghost gift_links GET")
                            .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_gift_links_edit().handle(req, res, std::move(send));
    })
                            .summary("Ghost gift_links PUT")
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_gift_links_create().handle(req, res, std::move(send));
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
        handler.posts_bulk_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost bulk PUT")
            )
            .add_route(
                Route{"/export"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_export().handle(req, res, std::move(send));
    })
                    .summary("Ghost export GET")
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
        handler.posts_read_by_slug().handle(req, res, std::move(send));
    })
                            .summary("Ghost :id GET")
                    )
            )
            .add_route(
                Route{"/upload"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_upload().handle(req, res, std::move(send));
    })
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api
