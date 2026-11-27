export module app_user:profile_acquirer_handler;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :profile_acquirer_input;
import :profile_acquirer_result;
import :profile_acquirer_runtime;

export namespace app::handlers::profile_acquirer {

class ProfileAcquirerRouteHandlers
{
public:
    // ---- route: POST /profile_acquirer -------------------------------------
    // Upstream route: routes/profile_acquirer.rs:11-50 (create_profile_acquirer).
    // Upstream core:  core/profile_acquirer.rs:12-102.
    void handle_create_profile_acquirer(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        const auto body = req.body_as_string();
        auto parsed = core::behavior::Serializer<::app::dto::profile_acquirer::ProfileAcquirerCreate>::pass(req, sink);
        if (!parsed) return;

        // Upstream extracts payload.profile_id before `server_wrap` and binds it to
        // JWTAuthProfileFromRoute (routes/profile_acquirer.rs:20,39-44).
        const std::string profile_id = std::string{parsed->get_profile_id().get_value()};
        m_policy.bind_profile_id(profile_id);
        if (const auto denied = m_support.authorize(req, "create_profile_acquirer", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("profile_id", std::string{profile_id});
        m_runtime.create(*parsed, ctx).handle(sink);
    }

    // ---- route: POST /profile_acquirer/:profile_id/:profile_acquirer_id -----
    // Upstream route: routes/profile_acquirer.rs:52-97 (profile_acquirer_update).
    // Upstream core:  core/profile_acquirer.rs:105-215.
    void handle_profile_acquirer_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // Route ids come from the path, never from the request body.
        const auto ids = m_input_support.profile_and_acquirer_ids_from_path(req);
        if (!ids.has_value()) {
            const auto& error = ids.error();
            error.send(sink);
            return;
        }
        const std::string profile_id = ids->first;
        const std::string profile_acquirer_id = ids->second;

        auto parsed = core::behavior::Serializer<::app::dto::profile_acquirer::ProfileAcquirerUpdate>::pass(req, sink);
        if (!parsed) return;

        // Upstream binds the route profile_id to JWTAuthProfileFromRoute
        // (routes/profile_acquirer.rs:87-91).
        m_policy.bind_profile_id(profile_id);
        if (const auto denied = m_support.authorize(req, "profile_acquirer_update", {})) {
            const auto& error = *denied;
            error.send(sink);
            return;
        }

        auto ctx = m_support.context(req);
        ctx.claims.insert_or_assign("profile_id", std::string{profile_id});
        ctx.claims.insert_or_assign("profile_acquirer_id", std::string{profile_acquirer_id});
        m_runtime.update(*parsed, ctx).handle(sink);
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "profile_acquirer"}
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_create_profile_acquirer(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch create_profile_acquirer")
            )
            .add_router(
                Router{router, ":profile_id"}.add_route(
                    Route{":profile_acquirer_id"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_profile_acquirer_update(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch profile_acquirer_update")
                )
            );
    }

    explicit ProfileAcquirerRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_input_support{context}
    {
    }

private:
    ProfileAcquirerRuntime m_runtime;
    ProfileAcquirerInputSupport m_input_support;

    ProfileAcquirerPolicy m_policy{m_input_support};

    ProfileAcquirerRequestSupport m_support{m_policy};
};

} // namespace app::handlers::profile_acquirer
