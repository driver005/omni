export module blog_handlers_routes:newsletters_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class NewslettersRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/newsletters"}
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.newsletters_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost newsletters GET")
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.newsletters_create().handle(req, res, std::move(send));
    })
            .summary("Ghost newsletters POST")
            .add_route(
                Route{"/:id"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.newsletters_read_by_id().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.newsletters_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
            )
            .add_route(
                Route{"/verifications"}
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.newsletters_newsletters_verifications_put().handle(req, res, std::move(send));
    })
                    .summary("Ghost verifications PUT")
            );
    }
};

} // namespace blog::api
