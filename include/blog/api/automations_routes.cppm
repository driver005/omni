export module blog_api:automations_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class AutomationsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/automations"}
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automations_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost automations GET")
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automations_create().handle(req, res, std::move(send));
    })
            .summary("Ghost automations POST")
            .add_router(
                Router{router, "/:id"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automations_read_by_id().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automations_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .add_router(
                        Router{router, "/actions"}
                            .add_router(
                                Router{router, "/:id"}
                                    .add_route(
                                        Route{"/links"}
                                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automations_automations_automation_id_actions_action_id_links_get().handle(req, res, std::move(send));
    })
                                            .summary("Ghost links GET")
                                    )
                            )
                    )
                    .add_route(
                        Route{"/email_preview"}
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automations_automations_id_email_preview_post().handle(req, res, std::move(send));
    })
                            .summary("Ghost email_preview POST")
                    )
                    .add_route(
                        Route{"/email_test"}
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automations_automations_id_email_test_post().handle(req, res, std::move(send));
    })
                            .summary("Ghost email_test POST")
                    )
                    .add_route(
                        Route{"/performance-stats"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automations_automations_id_performance_stats_get().handle(req, res, std::move(send));
    })
                            .summary("Ghost performance-stats GET")
                    )
                    .add_router(
                        Router{router, "/runs"}
                            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automations_automations_id_runs_get().handle(req, res, std::move(send));
    })
                            .summary("Ghost runs GET")
                            .add_route(
                                Route{"/:id"}
                                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automations_get_detail().handle(req, res, std::move(send));
    })
                                    .summary("Ghost :id GET")
                            )
                    )
            )
            .add_route(
                Route{"/poll"}
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automations_automations_poll_put().handle(req, res, std::move(send));
    })
                    .summary("Ghost poll PUT")
            );
    }
};

} // namespace blog::api
