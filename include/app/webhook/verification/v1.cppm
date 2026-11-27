export module app_webhook:verification_handlers_v1;

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

class VerificationHandlersV1
{
public:
    using RequestSupport =
        ::core::behavior::RequestSupport<::app::webhook::policy::VerificationPolicy>;

    VerificationHandlersV1(
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
     *  (routes/verification.rs:11-58 v1). */
    void handle_apple_pay_merchant_registration(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // 1. request -> Input
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = m_request_support.authorize(
                req,
                "apple_pay_merchant_registration",
                ::app::webhook::policy::kVerificationV1
            )) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }

        const auto merchant_id = m_input_support.merchant_id_from_path(req);
        if (!merchant_id) {
            const auto& error = merchant_id.error();
            error.send(res, send);
            return;
        }

        // server_wrap closure (routes/verification.rs:29-40): the
        // authenticated processor account id must equal the path merchant id,
        // otherwise ApiErrorResponse::Unauthorized.
        const auto authenticated = m_input_support.authenticated_merchant_id(auth, req);
        if (authenticated.empty() || authenticated != *merchant_id) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "Unauthorized"
            );
            error.send(res, send);
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
        // web::Json<ApplepayMerchantVerificationRequest> is strict: the plain
        // snake_case wire is the only accepted shape.
        using Input = VerificationRequestWire;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            );
            error.send(res, send);
            return;
        }
        ApplePayMerchantRegistrationInput input{
            *parsed,
            *merchant_id,
            std::string{auth.profile_id()}
        };

        // 2. Input -> DTO response
        auto result = m_runtime.apple_pay_merchant_registration(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        // 3. DTO -> response (same wire fields through Serializer).
        auto encoded = result->serialize(apple_pay_merchant_registration_wire_fields());
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::as_bytes(std::span{encoded->data(), encoded->size()}));
        send();
    }

    /** @brief Route handler for upstream `retrieve_apple_pay_verified_domains`
     *  (routes/verification.rs:109-170). */
    void handle_retrieve_apple_pay_verified_domains(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // 1. request -> Input
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = m_request_support.authorize(
                req,
                "retrieve_apple_pay_verified_domains",
                ::app::webhook::policy::kVerificationV1
            )) {
            const auto& error = *auth_error;
            error.send(res, send);
            return;
        }

        const auto query = m_input_support.verified_domains_query(req);
        if (!query) {
            const auto& error = query.error();
            error.send(res, send);
            return;
        }

        const auto authenticated = m_input_support.authenticated_merchant_id(auth, req);
        if (authenticated.empty() || authenticated != query->first) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "Unauthorized"
            );
            error.send(res, send);
            return;
        }

        RetrieveApplePayVerifiedDomainsInput input{
            query->first,
            std::string{auth.profile_id()},
            query->second
        };

        // 2. Input -> DTO response
        auto result = m_runtime.retrieve_apple_pay_verified_domains(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }

        // 3. DTO -> response (same wire fields through Serializer).
        auto encoded = result->serialize(retrieve_apple_pay_verified_domains_wire_fields());
        if (!encoded) {
            encoded.error().send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::as_bytes(std::span{encoded->data(), encoded->size()}));
        send();
    }

    void install_v1(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Upstream v1 scope is /verify (app.rs:2870). Sub-paths use the
        // upstream underscores/hyphens.
        Router{router, "verify"}
            .add_router(
                Router{router, "apple_pay"}.add_route(
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
                        .summary("Hyperswitch apple_pay_merchant_registration")
                )
            )
            .add_route(
                Route{"applepay_verified_domains"}
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
                    .summary("Hyperswitch retrieve_apple_pay_verified_domains")
            );
    }

private:
    VerificationRuntime& m_runtime;
    VerificationInputSupport& m_input_support;
    RequestSupport& m_request_support;
};

} // namespace app::handlers::verification
