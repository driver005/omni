export module app_handlers_profiles:handlers_v1;

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
import :handlers_v1_crud;
import :handlers_v1_list;
import :handlers_v1_toggles;

export namespace app::handlers::profiles {

/**
 * v1 surface: `account/:account_id/business_profile` plus the
 * `account/:account_id/profile` list. Routes are registered here in the exact
 * order of the original install(); the handle_* adapters live in the CRUD,
 * list and toggles partitions and are shared objects, not copied code paths.
 */
class ProfilesHandlersV1
{
public:
    ProfilesHandlersV1(
        app::context::AppContext& context,
        ProfilesRuntime& runtime,
        ProfilesInputSupport& input_support
    ) noexcept :
        m_crud{context, runtime, input_support},
        m_list{context, runtime, input_support},
        m_toggles{context, runtime, input_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Registration — app.rs:2727-2784 (v1), :2796-2800 (v1 profile-level list).
        // C++-only routes removed: /profiles/payment_connector_list_profile
        // (upstream handler unregistered) and /account/:account_id/profile/connectors
        // (owned by admin.cppm:1707).
        Router{router, "account/:account_id/business_profile"}
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
                    .post(
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
            )
            .add_route(
                // delt() is the ApiRoute DELETE binding (core/router/builder.cppm:542-548).
                Route{":profile_id"}
                    .delt(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            m_crud.handle_profile_delete(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch profile_delete")
            )
            .add_router(
                Router{router, ":profile_id"}.add_route(
                    Route{"toggle_extended_card_info"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                m_toggles
                                    .handle_toggle_extended_card_info(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch toggle_extended_card_info")
                )
            )
            .add_router(
                Router{router, ":profile_id"}.add_route(
                    Route{"toggle_connector_agnostic_mit"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                m_toggles.handle_toggle_connector_agnostic_mit(
                                    req,
                                    res,
                                    std::move(send)
                                );
                            }
                        )
                        .summary("Hyperswitch toggle_connector_agnostic_mit")
                )
            );

        Router{router, "account/:account_id/profile"}.add_route(
            Route{""}
                .get(
                    [this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    )
                    {
                        m_list.handle_profiles_list_at_profile_level(req, res, std::move(send));
                    }
                )
                .summary("Hyperswitch profiles_list_at_profile_level")
        );
    }

private:
    ProfilesHandlersV1Crud m_crud;
    ProfilesHandlersV1List m_list;
    ProfilesHandlersV1Toggles m_toggles;
};

} // namespace app::handlers::profiles
