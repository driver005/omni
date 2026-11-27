export module app_auth:connector_onboarding_handler;

import std;
import interfaces;
import core_router;
import core_behavior;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_context_core;
import app_connector;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_services;
import app_keystore;

import :connector_onboarding_input;
import :connector_onboarding_result;
import :connector_onboarding_runtime;

export namespace app::handlers::connector_onboarding {

class ConnectorOnboardingRouteHandlers
{
public:
    // ── Routes ───────────────────────────────────────────────────────────────────

    void handle_get_action_url(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support.authorize(req, "get_action_url", {})) {
            auth_error->send(res, send);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        auto parsed = serde::Ser::deserialize<::app::dto::connector_onboarding::ActionUrlRequest>(
            "application/json",
            body
        );
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        if (!m_input_support.valid_id(parsed->get_connector_id())) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid connector id"
            );
            error.send(res, send);
            return;
        }
        const auto scope = m_request_support.scope(req);
        const auto merchant_id =
            scope.connected_merchant_id.empty() ? scope.merchant_id : scope.connected_merchant_id;
        GetActionUrlInput input{*parsed, merchant_id};

        auto result = m_runtime.get_action_url(input);
        if (!result) {
            const auto& error = result.error();
            const auto status_override = m_input_support.status_override_for(error.get_kind_name());
            res.set_status(status_override.value_or(error.get_status()));
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_sync_onboarding_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error =
                m_request_support.authorize(req, "sync_onboarding_status", {})) {
            auth_error->send(res, send);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        auto parsed =
            serde::Ser::deserialize<::app::dto::connector_onboarding::OnboardingSyncRequest>(
                "application/json",
                body
            );
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        if (!m_input_support.valid_id(parsed->get_connector_id()) ||
            !m_input_support.valid_id(parsed->get_profile_id())) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid id"
            );
            error.send(res, send);
            return;
        }
        const auto scope = m_request_support.scope(req);
        const auto merchant_id =
            scope.connected_merchant_id.empty() ? scope.merchant_id : scope.connected_merchant_id;
        SyncOnboardingStatusInput input{*parsed, merchant_id};

        auto result = m_runtime.sync_onboarding_status(input);
        if (!result) {
            const auto& error = result.error();
            const auto status_override = m_input_support.status_override_for(error.get_kind_name());
            res.set_status(status_override.value_or(error.get_status()));
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    void handle_reset_tracking_id(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = m_request_support.authorize(req, "reset_tracking_id", {})) {
            auth_error->send(res, send);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(res, send);
            return;
        }
        auto parsed =
            serde::Ser::deserialize<::app::dto::connector_onboarding::ResetTrackingIdRequest>(
                "application/json",
                body
            );
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        if (!m_input_support.valid_id(parsed->get_connector_id())) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid connector id"
            );
            error.send(res, send);
            return;
        }
        const auto scope = m_request_support.scope(req);
        const auto merchant_id =
            scope.connected_merchant_id.empty() ? scope.merchant_id : scope.connected_merchant_id;
        ResetTrackingIdInput input{*parsed, merchant_id};

        auto result = m_runtime.reset_tracking_id(input);
        if (!result) {
            const auto& error = result.error();
            const auto status_override = m_input_support.status_override_for(error.get_kind_name());
            res.set_status(status_override.value_or(error.get_status()));
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
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
        Router{router, "connector_onboarding"}
            .add_route(
                Route{"action_url"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_action_url(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_action_url")
            )
            .add_route(
                Route{"sync"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_sync_onboarding_status(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch sync_onboarding_status")
            )
            .add_route(
                Route{"reset_tracking_id"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_reset_tracking_id(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch reset_tracking_id")
            );
    }

    explicit ConnectorOnboardingRouteHandlers(::app::context::AppContext& context) noexcept :
        m_runtime{context},
        m_policy{context},
        m_request_support{m_policy},
        m_input_support{}
    {
    }

private:
    ConnectorOnboardingRuntime m_runtime;
    ConnectorOnboardingPolicy m_policy;
    core::behavior::RequestSupport<ConnectorOnboardingPolicy> m_request_support;
    ConnectorOnboardingInputSupport m_input_support;
};

} // namespace app::handlers::connector_onboarding
