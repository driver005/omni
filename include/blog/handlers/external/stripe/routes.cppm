export module blog_handlers_routes:stripe_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class StripeRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/stripe"}
            .use(blog::handlers::middleware::content_type_middleware)
            .use(blog::handlers::middleware::require_auth)
            .add_router(
                Router{router, "/checkout"}
                    .add_route(
                        Route{"/branding"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stripe_stripe_checkout_branding_get().handle(req, res, std::move(send));
    })
                            .summary("Ghost branding GET")
                    )
                    .add_route(
                        Route{"/config"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stripe_stripe_checkout_config_get().handle(req, res, std::move(send));
    })
                            .summary("Ghost config GET")
                            .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stripe_stripe_checkout_config_put().handle(req, res, std::move(send));
    })
                            .summary("Ghost config PUT")
                    )
                    .add_route(
                        Route{"/preview"}
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stripe_stripe_checkout_preview_post().handle(req, res, std::move(send));
    })
                            .summary("Ghost preview POST")
                    )
            );
    }
};

} // namespace blog::api
