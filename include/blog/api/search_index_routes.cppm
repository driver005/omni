export module blog_api:search_index_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /search-index. */
class SearchIndexRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::SearchIndexRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/search-index"}
            .add_route(
                Route{"/authors"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::search_index::search_index_authors_get::SearchIndexAuthorsGetSearchIndexHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost authors GET")
            )
            .add_route(
                Route{"/pages"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::search_index::search_index_pages_get::SearchIndexPagesGetSearchIndexHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost pages GET")
            )
            .add_route(
                Route{"/posts"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::search_index::search_index_posts_get::SearchIndexPostsGetSearchIndexHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost posts GET")
            )
            .add_route(
                Route{"/tags"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::search_index::search_index_tags_get::SearchIndexTagsGetSearchIndexHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost tags GET")
            )
            .add_route(
                Route{"/users"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::search_index::search_index_users_get::SearchIndexUsersGetSearchIndexHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost users GET")
            );
    }
};

} // namespace blog::api
