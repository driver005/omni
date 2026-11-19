export module blog_handlers_routes:comments_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class CommentsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/comments"}
            .use(blog::handlers::middleware::content_type_middleware)
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.comments_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost comments GET")
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.comments_create().handle(req, res, std::move(send));
    })
            .summary("Ghost comments POST")
            .add_router(
                Router{router, "/:id"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.comments_read_by_id().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.comments_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .add_route(
                        Route{"/dislikes"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.comments_comment_dislikes().handle(req, res, std::move(send));
    })
                            .summary("Ghost dislikes GET")
                    )
                    .add_route(
                        Route{"/likes"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.comments_comment_likes().handle(req, res, std::move(send));
    })
                            .summary("Ghost likes GET")
                    )
                    .add_route(
                        Route{"/replies"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.comments_comment_replies().handle(req, res, std::move(send));
    })
                            .summary("Ghost replies GET")
                    )
                    .add_route(
                        Route{"/reports"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.comments_comment_reports().handle(req, res, std::move(send));
    })
                            .summary("Ghost reports GET")
                    )
            )
            .add_router(
                Router{router, "/post"}
                    .add_route(
                        Route{"/:id"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.comments_get_detail().handle(req, res, std::move(send));
    })
                            .summary("Ghost :id GET")
                    )
            );
    }
};

} // namespace blog::api
