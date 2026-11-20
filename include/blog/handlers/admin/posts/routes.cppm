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
    /**
     * @brief Installs the posts routes. `content=true` installs only the three
     * public GET counterparts (content API); the default installs the full
     * admin API set.
     */
    void install(
        core::router::RouterContext<>& router,
        blog::api::BlogHandlers& handler,
        bool content = false
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        if (content) {
            // Content API requests arrive anonymously; mark the context so the
            // shared steps use the public option/transformer/gift behavior and
            // core::auth::ghost grants the public read pass-through.
            const auto content_context = +[](interfaces::io::IRequest& req,
                                             interfaces::io::IResponse& res,
                                             interfaces::NextFn&& next,
                                             std::function<void()> send) {
                req.set_header(blog::handlers::middleware::kGhostContextHeader, "CONTENT_API");
                next(req, res, std::move(send));
            };
            Router{router, "/posts"}
                .use(blog::handlers::middleware::content_type_middleware)
                .use(content_context)
                .use(blog::handlers::middleware::bearer_middleware)
                .use(blog::handlers::middleware::optional_auth)
                .get([&handler](
            interfaces::io::IRequest& req,
            interfaces::io::IResponse& res,
            std::function<void()> send
        ) {
            handler.posts_browse().handle(req, res, std::move(send));
        })
                .summary("Ghost content posts GET")
                .add_router(
                    Router{router, "/:id"}
                        .get([&handler](
            interfaces::io::IRequest& req,
            interfaces::io::IResponse& res,
            std::function<void()> send
        ) {
            handler.posts_read_by_id().handle(req, res, std::move(send));
        })
                        .summary("Ghost content :id GET")
                )
                .add_router(
                    Router{router, "/slug"}
                        .add_route(
                            Route{"/:slug"}
                                .get([&handler](
            interfaces::io::IRequest& req,
            interfaces::io::IResponse& res,
            std::function<void()> send
        ) {
            handler.posts_read_by_slug().handle(req, res, std::move(send));
        })
                                .summary("Ghost content :slug GET")
                        )
                );
            return;
        }

        Router{router, "/posts"}
            .use(blog::handlers::middleware::content_type_middleware)
            .use(blog::handlers::middleware::require_auth)
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
                        Route{"/:slug"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.posts_read_by_slug().handle(req, res, std::move(send));
    })
                            .summary("Ghost :slug GET")
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
