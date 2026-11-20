export module blog_handlers_routes:tags_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;

export namespace blog::api {

class TagsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Ghost's API app uses body parsers rather than a Content-Type gate, and each
        // controller applies authAdminApi / authenticatePublic + permissions:true in its
        // own pipeline. Router-level optional_auth / content_type_middleware are removed;
        // the handlers now enforce the Ghost permission service (core::auth::ghost via
        // require_ghost_permission). See docs/parity/behavior/tags{,-public}.md.
        Router{router, "/tags"}
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.tags_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost tags GET")
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.tags_create().handle(req, res, std::move(send));
    })
            .summary("Ghost tags POST")
            .add_route(
                Route{"/:id"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.tags_read_by_id().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.tags_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.tags_destroy().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
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
        handler.tags_read_by_slug().handle(req, res, std::move(send));
    })
                            .summary("Ghost :slug GET")
                    )
            );
    }
};

} // namespace blog::api
