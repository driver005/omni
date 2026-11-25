export module blog_handlers_routes:integrations_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;
import blog_handlers_middleware;

export namespace blog::api {

namespace integrations_routes_detail {

    /**
     * @brief Ghost enforces a JSON content-type on requests that carry a body but
     *        not on GET/DELETE (`authAdminApi` has no serde gate). The shared
     *        content_type_middleware rejects every verb, so restrict it here.
     */
    inline void content_type_when_body(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        interfaces::NextFn&& next,
        std::function<void()> send
    )
    {
        const auto method = req.get_method();
        const auto bare = method.empty() ? std::string_view{} : method;
        if (bare == "GET" || bare == "get" || bare == "DELETE" || bare == "delete" ||
            bare == "HEAD" || bare == "head") {
            next(req, res, std::move(send));
            return;
        }
        blog::handlers::middleware::content_type_middleware(
            req,
            res,
            std::move(next),
            std::move(send)
        );
    }

} // namespace integrations_routes_detail

class IntegrationsRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/integrations"}
            .use(integrations_routes_detail::content_type_when_body)
            .get(
                [&handler](
                    interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    std::function<void()> send
                )
                {
                    handler.integrations_browse().handle(req, res, std::move(send));
                }
            )
            .summary("Ghost integrations GET")
            .post(
                [&handler](
                    interfaces::io::IRequest& req,
                    interfaces::io::IResponse& res,
                    std::function<void()> send
                )
                {
                    handler.integrations_create().handle(req, res, std::move(send));
                }
            )
            .summary("Ghost integrations POST")
            .add_router(
                Router{router, "/:id"}
                    .get(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.integrations_read_by_id().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost :id GET")
                    .put(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.integrations_edit().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost :id PUT")
                    .delt(
                        [&handler](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handler.integrations_destroy().handle(req, res, std::move(send));
                        }
                    )
                    .summary("Ghost :id DELETE")
                    .add_router(
                        Router{router, "/api_key"}.add_router(
                            Router{router, "/:keyid"}.add_route(
                                Route{"/refresh"}
                                    .post(
                                        [&handler](
                                            interfaces::io::IRequest& req,
                                            interfaces::io::IResponse& res,
                                            std::function<void()> send
                                        )
                                        {
                                            handler
                                                .integrations_integrations_id_api_key_keyid_refresh_post()
                                                .handle(req, res, std::move(send));
                                        }
                                    )
                                    .summary("Ghost refresh POST")
                            )
                        )
                    )
            );
    }
};

} // namespace blog::api
