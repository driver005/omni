export module blog_api:stats_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

/** @brief Ghost routes for /stats. */
class StatsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::CoreHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        auto* h = &handler;
        Router{router, "/stats"}
            .add_route(
                Route{"/member_count"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost member_count GET")
            )
            .add_route(
                Route{"/mrr"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost mrr GET")
            )
            .add_route(
                Route{"/newsletter-basic-stats"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost newsletter-basic-stats GET")
            )
            .add_route(
                Route{"/newsletter-click-stats"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost newsletter-click-stats GET")
            )
            .add_route(
                Route{"/newsletter-stats"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost newsletter-stats GET")
            )
            .add_router(
                Router{router, "/posts"}
                    .add_router(
                        Router{router, "/:id"}
                            .add_route(
                                Route{"/growth"}
                                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                                    .summary("Ghost growth GET")
                            )
                            .add_route(
                                Route{"/stats"}
                                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                                    .summary("Ghost stats GET")
                            )
                            .add_route(
                                Route{"/top-referrers"}
                                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                                    .summary("Ghost top-referrers GET")
                            )
                    )
            )
            .add_route(
                Route{"/posts-member-counts"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost posts-member-counts POST")
            )
            .add_route(
                Route{"/posts-visitor-counts"}
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost posts-visitor-counts POST")
            )
            .add_route(
                Route{"/referrers"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost referrers GET")
            )
            .add_route(
                Route{"/subscriber-count"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost subscriber-count GET")
            )
            .add_route(
                Route{"/subscriptions"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost subscriptions GET")
            )
            .add_route(
                Route{"/top-content"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost top-content GET")
            )
            .add_route(
                Route{"/top-posts"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost top-posts GET")
            )
            .add_route(
                Route{"/top-posts-views"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost top-posts-views GET")
            )
            .add_route(
                Route{"/top-sources-growth"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        h->handle(req, res, std::move(send));
    })
                    .summary("Ghost top-sources-growth GET")
            );
    }
};

} // namespace blog::api
