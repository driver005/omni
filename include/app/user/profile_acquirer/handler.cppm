export module app_user:profile_acquirer_handler;

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
        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        // Upstream web::Json<ProfileAcquirerCreate> extraction (routes:16, api_models:4-32)
        // deserializes `network: CardNetwork` and `acquirer_country_code: CountryAlpha2`,
        // rejecting unknown enum strings with HTTP 400 before server_wrap/core run.
        if (!m_input_support.payload_passes_serde_validation(body, false)) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        using Input = ::app::dto::profile_acquirer::ProfileAcquirerCreate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }

        // Upstream extracts payload.profile_id before `server_wrap` and binds it to
        // JWTAuthProfileFromRoute (routes/profile_acquirer.rs:20,39-44).
        const std::string profile_id = std::string{parsed->get_profile_id().get_value()};
        if (const auto denied =
                m_input_support.authorize(req, "create_profile_acquirer", profile_id)) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }

        CreateProfileAcquirerInput input{*parsed, profile_id};
        auto result = m_runtime.create(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
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
        // Route ids come from the path, never from the request body.
        const auto ids = m_input_support.profile_and_acquirer_ids_from_path(req);
        if (!ids.has_value()) {
            const auto& error = ids.error();
            error.send(res, send);
            return;
        }
        const std::string profile_id = ids->first;
        const std::string profile_acquirer_id = ids->second;

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        // Upstream web::Json<ProfileAcquirerUpdate> extraction (routes:61,
        // api_models:154-174) applies `#[serde(deny_unknown_fields)]` plus the
        // CardNetwork / CountryAlpha2 enum checks, all as HTTP 400 before core.
        if (!m_input_support.payload_passes_serde_validation(body, true)) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        using Input = ::app::dto::profile_acquirer::ProfileAcquirerUpdate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }

        // Upstream binds the route profile_id to JWTAuthProfileFromRoute
        // (routes/profile_acquirer.rs:87-91).
        if (const auto denied =
                m_input_support.authorize(req, "profile_acquirer_update", profile_id)) {
            const auto& error = *denied;
            error.send(res, send);
            return;
        }

        ProfileAcquirerUpdateInput input{*parsed, profile_id, profile_acquirer_id};
        auto result = m_runtime.update(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
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
};

} // namespace app::handlers::profile_acquirer
