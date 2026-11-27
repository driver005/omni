export module app_webhook:verification_handlers_v2;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_auth;
import app_connector;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_keystore;
import app_services;

import :verification_input;
import :webhook_policy;
import :verification_result;
import :verification_runtime;

export namespace app::handlers::verification {

class VerificationHandlersV2
{
public:
    using RequestSupport =
        ::core::behavior::RequestSupport<::app::webhook::policy::VerificationPolicy>;

    VerificationHandlersV2(
        ::app::context::AppContext& ctx,
        VerificationRuntime& runtime,
        VerificationInputSupport& input_support,
        RequestSupport& request_support
    ) noexcept :
        m_runtime{runtime},
        m_input_support{input_support},
        m_request_support{request_support}
    {
    }

    /** @brief Route handler for upstream `apple_pay_merchant_registration`
     *  (routes/verification.rs:60-107 v2). */
    void handle_apple_pay_merchant_registration(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // 1. request -> Input
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = m_request_support.authorize(
                req,
                "apple_pay_merchant_registration",
                ::app::webhook::policy::kVerificationV2
            )) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        const auto merchant_id = m_input_support.merchant_id_from_path(req);
        if (!merchant_id) {
            const auto& error = merchant_id.error();
            error.send(sink);
            return;
        }

        // server_wrap closure (routes/verification.rs:78-89): the
        // authenticated processor account id must equal the path merchant id,
        // otherwise ApiErrorResponse::Unauthorized.
        const auto authenticated = m_input_support.authenticated_merchant_id(auth, req);
        if (authenticated.empty() || authenticated != *merchant_id) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "Unauthorized"
            );
            error.send(sink);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            error.send(sink);
            return;
        }
        // web::Json<ApplepayMerchantVerificationRequest> is strict: the plain
        // snake_case wire is the only accepted shape.
        auto parsed = core::behavior::Serializer<VerificationRequestWire>::pass(req, sink);
        if (!parsed) {
            return;
        }
        auto ctx = m_request_support.context(req);
        ctx.merchant_id = *merchant_id;
        ctx.profile_id = auth.profile_id();

        m_runtime.apple_pay_merchant_registration(*parsed, ctx).handle(sink);
    }

    /** @brief Route handler for upstream `retrieve_apple_pay_verified_domains`
     *  (routes/verification.rs:109-170). */
    void handle_retrieve_apple_pay_verified_domains(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        // 1. request -> Input
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = m_request_support.authorize(
                req,
                "retrieve_apple_pay_verified_domains",
                ::app::webhook::policy::kVerificationV2
            )) {
            const auto& error = *auth_error;
            error.send(sink);
            return;
        }

        const auto query = m_input_support.verified_domains_query(req);
        if (!query) {
            const auto& error = query.error();
            error.send(sink);
            return;
        }

        const auto authenticated = m_input_support.authenticated_merchant_id(auth, req);
        if (authenticated.empty() || authenticated != query->first) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "Unauthorized"
            );
            error.send(sink);
            return;
        }

        auto ctx = m_request_support.context(req);
        ctx.merchant_id = query->first;
        ctx.profile_id = auth.profile_id();
        ctx.query["merchant_connector_account_id"] = query->second;

        m_runtime.retrieve_apple_pay_verified_domains(ctx).handle(sink);
    }

    void install_v2(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Upstream v2 scope is /v2/verify (app.rs:2886). Sub-paths use the
        // upstream hyphens.
        Router{router, "v2/verify"}
            .add_router(
                Router{router, "apple-pay"}.add_route(
                    Route{":merchant_id"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_apple_pay_merchant_registration(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch apple_pay_merchant_registration (v2)")
                )
            )
            .add_route(
                Route{"applepay-verified-domains"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_retrieve_apple_pay_verified_domains(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch retrieve_apple_pay_verified_domains (v2)")
            );
    }

private:
    VerificationRuntime& m_runtime;
    VerificationInputSupport& m_input_support;
    RequestSupport& m_request_support;
};

} // namespace app::handlers::verification
