export module blog_api:members_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /members. */
class MembersRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::MembersRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/members"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::browse::BrowseMembersHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost members GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::create::CreateMembersHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost members POST")
            .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::bulk_destroy::BulkDestroyMembersHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost members DELETE")
            .add_router(
                Router{router, "/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::read_by_id::ReadByIdMembersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::edit::EditMembersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::destroy::DestroyMembersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
                    .add_router(
                        Router{router, "/commenting"}
                            .add_route(
                                Route{"/disable"}
                                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::members_id_commenting_disable_post::MembersIdCommentingDisablePostMembersHandler::handle(req, res, std::move(send));
    })
                                    .summary("Ghost disable POST")
                            )
                            .add_route(
                                Route{"/enable"}
                                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::members_id_commenting_enable_post::MembersIdCommentingEnablePostMembersHandler::handle(req, res, std::move(send));
    })
                                    .summary("Ghost enable POST")
                            )
                    )
                    .add_route(
                        Route{"/sessions"}
                            .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::members_id_sessions_delete::MembersIdSessionsDeleteMembersHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost sessions DELETE")
                    )
                    .add_route(
                        Route{"/signin_urls"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::signin_urls::SigninUrlsMembersHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost signin_urls GET")
                    )
                    .add_router(
                        Router{router, "/subscriptions"}
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::subscriptions_post::SubscriptionsPostMembersHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost subscriptions POST")
                            .add_route(
                                Route{"/:id"}
                                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::put_detail::PutDetailMembersHandler::handle(req, res, std::move(send));
    })
                                    .summary("Ghost :id PUT")
                            )
                    )
                    .add_route(
                        Route{"/suppression"}
                            .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::members_id_suppression_delete::MembersIdSuppressionDeleteMembersHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost suppression DELETE")
                    )
            )
            .add_route(
                Route{"/bulk"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::bulk_edit::BulkEditMembersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost bulk PUT")
            )
            .add_route(
                Route{"/events"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::members_events_get::MembersEventsGetMembersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost events GET")
            )
            .add_route(
                Route{"/metafields"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::members_metafields_all::MembersMetafieldsAllMembersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost metafields ALL")
            )
            .add_router(
                Router{router, "/stats"}
                    .add_route(
                        Route{"/count"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::members_stats_count_get::MembersStatsCountGetMembersHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost count GET")
                    )
                    .add_route(
                        Route{"/mrr"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::members_stats_mrr_get::MembersStatsMrrGetMembersHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost mrr GET")
                    )
            )
            .add_route(
                Route{"/stripe_connect"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::members_stripe_connect_get::MembersStripeConnectGetMembersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost stripe_connect GET")
            )
            .add_route(
                Route{"/upload"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::members_upload_get::MembersUploadGetMembersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost upload GET")
                    .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::members::members_upload_post::MembersUploadPostMembersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api
