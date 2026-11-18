export module blog_api:pages_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /pages. */
class PagesRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::PagesRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/pages"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::pages::browse::BrowsePagesHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost pages GET")
            .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::pages::bulk_destroy::BulkDestroyPagesHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost pages DELETE")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::pages::create::CreatePagesHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost pages POST")
            .add_router(
                Router{router, "/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::pages::read_by_id::ReadByIdPagesHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::pages::edit::EditPagesHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::pages::destroy::DestroyPagesHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
                    .add_route(
                        Route{"/copy"}
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::pages::pages_id_copy_post::PagesIdCopyPostPagesHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::pages::gift_links_get::GiftLinksGetPagesHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost gift_links GET")
                            .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::pages::gift_links_put::GiftLinksPutPagesHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost gift_links PUT")
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::pages::gift_links_post::GiftLinksPostPagesHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost gift_links POST")
                    )
            )
            .add_route(
                Route{"/bulk"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::pages::bulk_edit::BulkEditPagesHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost bulk PUT")
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
        blog::handlers::operations::pages::read_by_slug::ReadBySlugPagesHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost :id GET")
                    )
            );
    }
};

} // namespace blog::api
