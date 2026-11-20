export module blog_handlers_routes:webhooks_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;

export namespace blog::api {

/**
 * Ghost mounts these three routes under the Admin API prefix with a single
 * `mw.authAdminApi` middleware and `permissions:true` / `permissions.before` in the
 * controller pipeline (`routes.js:536-538`, `webhooks.js:23-123`). Authentication,
 * permission and integration-ownership checks therefore live in the handlers, not in
 * a router-level Content-Type/Accept gate — Ghost's API app uses body parsers only.
 */
class WebhooksRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        Router{router, "/webhooks"}
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.webhooks_create().handle(req, res, std::move(send));
    })
            .summary("Ghost webhooks POST")
            .add_route(
                Route{"/:id"}
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.webhooks_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.webhooks_destroy().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
            );
    }
};

} // namespace blog::api
