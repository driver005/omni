export module blog_handlers_routes:member_metafields_members_routes;
import blog_handlers;

import std;
import interfaces;
import core_router;
import utils_openapi;

export namespace blog::api {

class MemberMetafieldsMembersRoutes
{
public:
    void install(core::router::RouterContext<>& router, blog::api::BlogHandlers& handler)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Ghost binds GET /metafields/:namespace on the member-account router with
        // rejectWhenAnonymous (web/members/account/routes.ts:57-61); no bodyParser,
        // so no JSON Content-Type/Accept gate on this GET. The C++ flat router has
        // no per-app mount layer, so the path is bound verbatim under /api/member;
        // member identity is carried by X-Member-Id (see the execute step).
        Router{router, "/api/member"}.add_route(
            Route{"/metafields/:namespace"}
                .get(
                    [&handler](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        handler.member_metafields_members_metafields_browse()
                            .handle(req, res, std::move(send));
                    }
                )
                .summary("Ghost memberMetafieldsMembers.browse")
        );
    }
};

} // namespace blog::api
