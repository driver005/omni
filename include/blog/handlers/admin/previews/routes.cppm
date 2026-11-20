export module blog_handlers_routes:previews_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;

export namespace blog::api {

class PreviewsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Ghost mounts the frontend PreviewRouter at /p/ (preview-router.js:15,29)
        // with a public, HTML-rendered controller — no Admin API auth and no JSON
        // serde middleware. C++ drops the mount prefix; core::router has no
        // optional-segment syntax, so both GET shapes map to the same handler.
        // Deliberately no `content_type_middleware`: Ghost imposes no JSON
        // Content-Type/Accept requirement on this route.
        Router{router, "/p"}
            .add_route(
                Route{"/:uuid"}
                    .get([&handler](
                interfaces::io::IRequest& req,
                interfaces::io::IResponse& res,
                std::function<void()> send
            ) {
                handler.previews_preview_get().handle(req, res, std::move(send));
            })
                    .summary("Ghost previews read GET")
            )
            .add_route(
                Route{"/:uuid/:options"}
                    .get([&handler](
                interfaces::io::IRequest& req,
                interfaces::io::IResponse& res,
                std::function<void()> send
            ) {
                handler.previews_preview_get().handle(req, res, std::move(send));
            })
                    .summary("Ghost previews read GET (options)")
            );
    }
};

} // namespace blog::api
