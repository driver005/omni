export module blog_api:automated_emails_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;

export namespace blog::api {

class AutomatedEmailsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/automated_emails"}
            .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automated_emails_browse().handle(req, res, std::move(send));
    })
            .summary("Ghost automated_emails GET")
            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automated_emails_create().handle(req, res, std::move(send));
    })
            .summary("Ghost automated_emails POST")
            .add_router(
                Router{router, "/:id"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automated_emails_read_by_id().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automated_emails_edit().handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .add_route(
                        Route{"/preview"}
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automated_emails_automated_emails_id_preview_post().handle(req, res, std::move(send));
    })
                            .summary("Ghost preview POST")
                    )
                    .add_route(
                        Route{"/test"}
                            .post([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automated_emails_automated_emails_id_test_post().handle(req, res, std::move(send));
    })
                            .summary("Ghost test POST")
                    )
            )
            .add_route(
                Route{"/design"}
                    .get([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automated_emails_automated_emails_design_get().handle(req, res, std::move(send));
    })
                    .summary("Ghost design GET")
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automated_emails_automated_emails_design_put().handle(req, res, std::move(send));
    })
                    .summary("Ghost design PUT")
            )
            .add_route(
                Route{"/senders"}
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automated_emails_automated_emails_senders_put().handle(req, res, std::move(send));
    })
                    .summary("Ghost senders PUT")
            )
            .add_route(
                Route{"/verifications"}
                    .put([&handler](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        handler.automated_emails_automated_emails_verifications_put().handle(req, res, std::move(send));
    })
                    .summary("Ghost verifications PUT")
            );
    }
};

} // namespace blog::api
