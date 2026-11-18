export module blog_handlers_routes:members_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class MembersRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/members"}
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost members GET")
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_create().handle(req, res, std::move(send));
    })
            .summary("Ghost members POST")
            .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_bulk_destroy().handle(req, res, std::move(send));
    })
            .summary("Ghost members DELETE")
            .add_router(
                Router{router, "/:id"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_read_by_id().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_destroy().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
                    .add_router(
                        Router{router, "/commenting"}
                            .add_route(
                                Route{"/disable"}
                                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_members_id_commenting_disable_post().handle(req, res, std::move(send));
    })
                                    .summary("Ghost disable POST")
                            )
                            .add_route(
                                Route{"/enable"}
                                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_members_id_commenting_enable_post().handle(req, res, std::move(send));
    })
                                    .summary("Ghost enable POST")
                            )
                    )
                    .add_route(
                        Route{"/sessions"}
                            .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_members_id_sessions_delete().handle(req, res, std::move(send));
    })
                            .summary("Ghost sessions DELETE")
                    )
                    .add_route(
                        Route{"/signin_urls"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_signin_urls().handle(req, res, std::move(send));
    })
                            .summary("Ghost signin_urls GET")
                    )
                    .add_router(
                        Router{router, "/subscriptions"}
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_subscriptions_post().handle(req, res, std::move(send));
    })
                            .summary("Ghost subscriptions POST")
                            .add_route(
                                Route{"/:id"}
                                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_put_detail().handle(req, res, std::move(send));
    })
                                    .summary("Ghost :id PUT")
                            )
                    )
                    .add_route(
                        Route{"/suppression"}
                            .delt([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_members_id_suppression_delete().handle(req, res, std::move(send));
    })
                            .summary("Ghost suppression DELETE")
                    )
            )
            .add_route(
                Route{"/bulk"}
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_bulk_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost bulk PUT")
            )
            .add_route(
                Route{"/events"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_members_events_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost events GET")
            )
            .add_route(
                Route{"/metafields"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_members_metafields_all().handle(req, res, std::move(send));
    })
                    .summary("Ghost metafields ALL")
            )
            .add_router(
                Router{router, "/stats"}
                    .add_route(
                        Route{"/count"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_members_stats_count_get().handle(req, res, std::move(send));
    })
                            .summary("Ghost count GET")
                    )
                    .add_route(
                        Route{"/mrr"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_members_stats_mrr_get().handle(req, res, std::move(send));
    })
                            .summary("Ghost mrr GET")
                    )
            )
            .add_route(
                Route{"/stripe_connect"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_members_stripe_connect_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost stripe_connect GET")
            )
            .add_route(
                Route{"/upload"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_members_upload_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost upload GET")
                    .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.members_members_upload_post().handle(req, res, std::move(send));
    })
                    .summary("Ghost upload POST")
            );
    }
};

} // namespace blog::api
