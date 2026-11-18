export module blog_api:users_routes;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers;
import blog_handlers_middleware;
import blog_operations;

export namespace blog::api {

/** @brief Ghost routes for /users. */
class UsersRoutes
{
public:
    void install(
        core::router::RouterContext<>& router,
        blog::api::UsersRouteHandler& handler
    )
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        [[maybe_unused]] auto* h = &handler;
        Router{router, "/users"}
            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::users::browse::BrowseUsersHandler::handle(req, res, std::move(send));
    })
            .summary("Ghost users GET")
            .add_router(
                Router{router, "/:id"}
                    .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::users::read_by_id::ReadByIdUsersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id GET")
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::users::edit::EditUsersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id PUT")
                    .delt([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::users::destroy::DestroyUsersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost :id DELETE")
                    .add_route(
                        Route{"/token"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::users::users_id_token_get::UsersIdTokenGetUsersHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost token GET")
                            .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::users::users_id_token_put::UsersIdTokenPutUsersHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost token PUT")
                    )
            )
            .add_router(
                Router{router, "/email"}
                    .add_route(
                        Route{"/:id"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::users::read_by_email::ReadByEmailUsersHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost :id GET")
                    )
            )
            .add_route(
                Route{"/owner"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::users::users_owner_put::UsersOwnerPutUsersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost owner PUT")
            )
            .add_route(
                Route{"/password"}
                    .put([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::users::users_password_put::UsersPasswordPutUsersHandler::handle(req, res, std::move(send));
    })
                    .summary("Ghost password PUT")
            )
            .add_router(
                Router{router, "/slug"}
                    .add_route(
                        Route{"/:id"}
                            .get([h](
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    ) {
        blog::handlers::operations::users::read_by_slug::ReadBySlugUsersHandler::handle(req, res, std::move(send));
    })
                            .summary("Ghost :id GET")
                    )
            );
    }
};

} // namespace blog::api
