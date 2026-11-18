export module blog_api:comments_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /comments. */
class CommentsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::CommentsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/comments"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::comments::browse::BrowseCommentsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost comments GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::comments::create::CreateCommentsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost comments POST")
            .add_router(
                Router{router, "/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::comments::read_by_id::ReadByIdCommentsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::comments::edit::EditCommentsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .add_route(
                        Route{"/dislikes"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::comments::comment_dislikes::CommentDislikesCommentsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost dislikes GET")
                    )
                    .add_route(
                        Route{"/likes"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::comments::comment_likes::CommentLikesCommentsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost likes GET")
                    )
                    .add_route(
                        Route{"/replies"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::comments::comment_replies::CommentRepliesCommentsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost replies GET")
                    )
                    .add_route(
                        Route{"/reports"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::comments::comment_reports::CommentReportsCommentsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost reports GET")
                    )
            )
            .add_router(
                Router{router, "/post"}
                    .add_route(
                        Route{"/:id"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::comments::get_detail::GetDetailCommentsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost :id GET")
                    )
            );
    }
};

} // namespace blog::api
