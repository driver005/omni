export module blog_api:posts_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

/** @brief Ghost routes for /posts. */
class PostsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::BlogHandlers& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        auto* h = &handler;
        Router{router, "/posts"}
            .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_bulk_destroy().handle(req, res, std::move(send));

    })
            .summary("Ghost posts DELETE")
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
        handler.posts_upload().handle(req, res, std::move(send));

    })
            .summary("Ghost posts POST")
            .add_router(
                Router{router, "/:id"}
                    .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_destroy().handle(req, res, std::move(send));

    })
                    .summary("Ghost :id DELETE")
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
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_gift_links_create().handle(req, res, std::move(send));

    })
                            .summary("Ghost gift_links POST")
                            .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_gift_links_edit().handle(req, res, std::move(send));

    })
                            .summary("Ghost gift_links PUT")
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
