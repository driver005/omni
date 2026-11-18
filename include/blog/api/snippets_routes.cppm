export module blog_api:snippets_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /snippets. */
class SnippetsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::SnippetsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/snippets"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::snippets::browse::BrowseSnippetsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost snippets GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::snippets::create::CreateSnippetsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost snippets POST")
            .add_route(
                Route{"/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::snippets::read_by_id::ReadByIdSnippetsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::snippets::edit::EditSnippetsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::snippets::destroy::DestroySnippetsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
            );
    }
};

} // namespace blog::api
