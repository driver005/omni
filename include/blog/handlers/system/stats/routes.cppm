export module blog_handlers_routes:stats_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

class StatsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/stats"}
            .use(blog::handlers::middleware::content_type_middleware)
            .add_route(
                Route{"/member_count"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_member_count_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost member_count GET")
            )
            .add_route(
                Route{"/mrr"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_mrr_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost mrr GET")
            )
            .add_route(
                Route{"/newsletter-basic-stats"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_newsletter_basic_stats_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost newsletter-basic-stats GET")
            )
            .add_route(
                Route{"/newsletter-click-stats"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_newsletter_click_stats_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost newsletter-click-stats GET")
            )
            .add_route(
                Route{"/newsletter-stats"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_newsletter_stats_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost newsletter-stats GET")
            )
            .add_router(
                Router{router, "/posts"}
                    .add_router(
                        Router{router, "/:id"}
                            .add_route(
                                Route{"/growth"}
                                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_posts_id_growth_get().handle(req, res, std::move(send));
    })
                                    .summary("Ghost growth GET")
                            )
                            .add_route(
                                Route{"/stats"}
                                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_posts_id_stats_get().handle(req, res, std::move(send));
    })
                                    .summary("Ghost stats GET")
                            )
                            .add_route(
                                Route{"/top-referrers"}
                                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_posts_id_top_referrers_get().handle(req, res, std::move(send));
    })
                                    .summary("Ghost top-referrers GET")
                            )
                    )
            )
            .add_route(
                Route{"/posts-member-counts"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_posts_member_counts_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost posts-member-counts POST")
            )
            .add_route(
                Route{"/posts-visitor-counts"}
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_posts_visitor_counts_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost posts-visitor-counts POST")
            )
            .add_route(
                Route{"/referrers"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_referrers_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost referrers GET")
            )
            .add_route(
                Route{"/subscriber-count"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_subscriber_count_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost subscriber-count GET")
            )
            .add_route(
                Route{"/subscriptions"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_subscriptions_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost subscriptions GET")
            )
            .add_route(
                Route{"/top-content"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_top_content_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost top-content GET")
            )
            .add_route(
                Route{"/top-posts"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_top_posts_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost top-posts GET")
            )
            .add_route(
                Route{"/top-posts-views"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_top_posts_views_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost top-posts-views GET")
            )
            .add_route(
                Route{"/top-sources-growth"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.stats_stats_top_sources_growth_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost top-sources-growth GET")
            );
    }
};

} // namespace blog::api
