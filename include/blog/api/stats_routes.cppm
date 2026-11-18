export module blog_api:stats_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /stats. */
class StatsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::StatsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/stats"}
            .add_route(
                Route{"/member_count"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::stats::stats_member_count_get::StatsMemberCountGetStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_mrr_get::StatsMrrGetStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_newsletter_basic_stats_get::StatsNewsletterBasicStatsGetStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_newsletter_click_stats_get::StatsNewsletterClickStatsGetStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_newsletter_stats_get::StatsNewsletterStatsGetStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_posts_id_growth_get::StatsPostsIdGrowthGetStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_posts_id_stats_get::StatsPostsIdStatsGetStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_posts_id_top_referrers_get::StatsPostsIdTopReferrersGetStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_posts_member_counts_post::StatsPostsMemberCountsPostStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_posts_visitor_counts_post::StatsPostsVisitorCountsPostStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_referrers_get::StatsReferrersGetStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_subscriber_count_get::StatsSubscriberCountGetStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::subscriptions_get::SubscriptionsGetStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_top_content_get::StatsTopContentGetStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_top_posts_get::StatsTopPostsGetStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_top_posts_views_get::StatsTopPostsViewsGetStatsHandler::handle(req, res, std::move(send));
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
        blog::handlers::operations::stats::stats_top_sources_growth_get::StatsTopSourcesGrowthGetStatsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost top-sources-growth GET")
            );
    }
};

} // namespace blog::api
