export module blog_api:automated_emails_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /automated_emails. */
class AutomatedEmailsRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::AutomatedEmailsRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/automated_emails"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automated_emails::browse::BrowseAutomatedEmailsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost automated_emails GET")
            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automated_emails::create::CreateAutomatedEmailsHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost automated_emails POST")
            .add_router(
                Router{router, "/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automated_emails::read_by_id::ReadByIdAutomatedEmailsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automated_emails::edit::EditAutomatedEmailsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .add_route(
                        Route{"/preview"}
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automated_emails::automated_emails_id_preview_post::AutomatedEmailsIdPreviewPostAutomatedEmailsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost preview POST")
                    )
                    .add_route(
                        Route{"/test"}
                            .post([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automated_emails::automated_emails_id_test_post::AutomatedEmailsIdTestPostAutomatedEmailsHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost test POST")
                    )
            )
            .add_route(
                Route{"/design"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automated_emails::automated_emails_design_get::AutomatedEmailsDesignGetAutomatedEmailsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost design GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automated_emails::automated_emails_design_put::AutomatedEmailsDesignPutAutomatedEmailsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost design PUT")
            )
            .add_route(
                Route{"/senders"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automated_emails::automated_emails_senders_put::AutomatedEmailsSendersPutAutomatedEmailsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost senders PUT")
            )
            .add_route(
                Route{"/verifications"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::automated_emails::automated_emails_verifications_put::AutomatedEmailsVerificationsPutAutomatedEmailsHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost verifications PUT")
            );
    }
};

} // namespace blog::api
