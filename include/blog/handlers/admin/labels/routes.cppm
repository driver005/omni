export module blog_handlers_routes:labels_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class LabelsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Ghost's admin app uses body parsers, not a Content-Type gate, and each
        // controller applies authAdminApi + permissions:true in its pipeline.
        // Router-level content_type_middleware / require_auth are removed; each
        // labels handler now enforces the Ghost permission service via
        // core::auth::ghost + require_ghost_permission (docName "labels").
        Router{router, "/labels"}
            .get(
                [&handler](
                    interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    std::function<void()> send
                )
                {
                    handler.labels_browse().handle(req, res, std::move(send));
                }
            )
            .summary("Ghost labels GET")
            .post(
                [&handler](
                    interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    std::function<void()> send
                )
                {
                    handler.labels_create().handle(req, res, std::move(send));
                }
            )
            .summary("Ghost labels POST")
            .add_route(
                Route{"/:id"}
                    .get(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.labels_read_by_id().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost :id GET")
                    .put(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.labels_edit().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost :id PUT")
                    .delt(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.labels_destroy().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost :id DELETE")
            )
            .add_router(
                Router{router, "/slug"}.add_route(
                    Route{"/:slug"}
                        .get(
                            [&handler](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handler.labels_read_by_slug().handle(req, res, std::move(send));
                            }
                        )
                        .summary("Ghost :slug GET")
                )
            );
    }
};

} // namespace blog::api
