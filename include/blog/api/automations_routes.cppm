export module blog_api:automations_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /automations. */
class AutomationsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::AutomationsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/automations"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automations::browse::BrowseAutomationsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost automations GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automations::create::CreateAutomationsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost automations POST")
            .add_router(
                Router{router, "/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automations::read_by_id::ReadByIdAutomationsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automations::edit::EditAutomationsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .add_router(
                        Router{router, "/actions"}
                            .add_router(
                                Router{router, "/:id"}
                                    .add_route(
                                        Route{"/links"}
                                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automations::automations_automation_id_actions_action_id_links_get::AutomationsAutomationIdActionsActionIdLinksGetAutomationsHandler::handle(req, res, std::move(send));
    })
                                            .summary("Ghost links GET")
                                    )
                            )
                    )
                    .add_route(
                        Route{"/email_preview"}
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automations::automations_id_email_preview_post::AutomationsIdEmailPreviewPostAutomationsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost email_preview POST")
                    )
                    .add_route(
                        Route{"/email_test"}
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automations::automations_id_email_test_post::AutomationsIdEmailTestPostAutomationsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost email_test POST")
                    )
                    .add_route(
                        Route{"/performance-stats"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automations::automations_id_performance_stats_get::AutomationsIdPerformanceStatsGetAutomationsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost performance-stats GET")
                    )
                    .add_router(
                        Router{router, "/runs"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automations::automations_id_runs_get::AutomationsIdRunsGetAutomationsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost runs GET")
                            .add_route(
                                Route{"/:id"}
                                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automations::get_detail::GetDetailAutomationsHandler::handle(req, res, std::move(send));
    })
                                    .summary("Ghost :id GET")
                            )
                    )
            )
            .add_route(
                Route{"/poll"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automations::automations_poll_put::AutomationsPollPutAutomationsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost poll PUT")
            );
    }
};

} // namespace blog::api
