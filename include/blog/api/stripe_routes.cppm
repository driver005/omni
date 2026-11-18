export module blog_api:stripe_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /stripe. */
class StripeRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::StripeRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/stripe"}
            .add_router(
                Router{router, "/checkout"}
                    .add_route(
                        Route{"/branding"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::stripe::stripe_checkout_branding_get::StripeCheckoutBrandingGetStripeHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost branding GET")
                    )
                    .add_route(
                        Route{"/config"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::stripe::stripe_checkout_config_get::StripeCheckoutConfigGetStripeHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost config GET")
                            .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::stripe::stripe_checkout_config_put::StripeCheckoutConfigPutStripeHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost config PUT")
                    )
                    .add_route(
                        Route{"/preview"}
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::stripe::stripe_checkout_preview_post::StripeCheckoutPreviewPostStripeHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost preview POST")
                    )
            );
    }
};

} // namespace blog::api
