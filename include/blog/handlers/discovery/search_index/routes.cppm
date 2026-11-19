export module blog_handlers_routes:search_index_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class SearchIndexRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/search-index"}
            .use(blog::handlers::middleware::content_type_middleware)
            .add_route(
                Route{"/authors"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.search_index_search_index_authors_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost authors GET")
            )
            .add_route(
                Route{"/pages"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.search_index_search_index_pages_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost pages GET")
            )
            .add_route(
                Route{"/posts"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.search_index_search_index_posts_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost posts GET")
            )
            .add_route(
                Route{"/tags"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.search_index_search_index_tags_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost tags GET")
            )
            .add_route(
                Route{"/users"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.search_index_search_index_users_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost users GET")
            );
    }
};

} // namespace blog::api
