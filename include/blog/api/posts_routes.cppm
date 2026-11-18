export module blog_api:posts_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_handlers_posts_browse;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /posts. */
class PostsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::PostsRouteHandler& handler,
        blog::handlers::posts::browse::PostsBrowseHandler& browse_handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        auto* h = &handler;
        auto* browse = &browse_handler;
        Router{router, "/posts"}
            .use(blog::handlers::middleware::bearer_middleware)
            .use(blog::handlers::middleware::optional_auth)
            .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::posts::bulk_destroy::BulkDestroyPostsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost posts DELETE")
            .get([browse](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        browse->handle(req, res, std::move(send));
    })
            .summary("Ghost posts GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::posts::upload::UploadPostsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost posts POST")
            .add_router(
                Router{router, "/:id"}
                    .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::posts::destroy::DestroyPostsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::posts::read_by_id::ReadByIdPostsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::posts::edit::EditPostsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .add_route(
                        Route{"/copy"}
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::posts::copy::CopyPostsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost copy POST")
                    )
                    .add_route(
                        Route{"/gift_links"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::posts::gift_links_browse::GiftLinksBrowsePostsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost gift_links GET")
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::posts::gift_links_create::GiftLinksCreatePostsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost gift_links POST")
                            .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::posts::gift_links_edit::GiftLinksEditPostsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost gift_links PUT")
                    )
            )
            .add_route(
                Route{"/bulk"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::posts::bulk_edit::BulkEditPostsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost bulk PUT")
            )
            .add_route(
                Route{"/export"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::posts::export_op::ExportPostsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost export GET")
            )
            .add_router(
                Router{router, "/slug"}
                    .add_route(
                        Route{"/:id"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::posts::read_by_slug::ReadBySlugPostsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost slug GET")
                    )
            )
            .add_route(
                Route{"/upload"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::posts::upload::UploadPostsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api
