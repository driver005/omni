export module app_handlers_profiles:handlers_v2;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;

import :input;
import :result;
import :runtime;
import :handlers_v2_crud;
import :handlers_v2_list;

export namespace app::handlers::profiles {

/**
 * v2 surface: `v2/profiles` plus `v2/merchant-accounts/:id/profiles`.
 * Routes are registered here in the exact order of the original install();
 * handle_* adapters live in the CRUD and list partitions.
 */
class ProfilesHandlersV2
{
public:
    ProfilesHandlersV2(
        app::context::AppContext& context,
        ProfilesRuntime& runtime,
        ProfilesInputSupport& input_support
    ) noexcept :
        m_crud{context, runtime, input_support},
        m_list{context, runtime, input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Registration — app.rs:2661-2722 (v2), :2170 (v2 list).
        Router{router, "v2/profiles"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            m_crud.handle_profile_create(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch profile_create")
            )
            .add_route(
                Route{":profile_id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            m_crud.handle_profile_retrieve(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch profile_retrieve")
            )
            .add_route(
                Route{":profile_id"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            m_crud.handle_profile_update(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch profile_update")
            );

        Router{router, "v2/merchant-accounts/:id/profiles"}.add_route(
            Route{""}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        m_list.handle_profiles_list(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch profiles_list")
        );
    }

private:
    ProfilesHandlersV2Crud m_crud;
    ProfilesHandlersV2List m_list;
};

} // namespace app::handlers::profiles
