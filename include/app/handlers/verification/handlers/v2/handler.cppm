export module app_handlers_verification:handlers_v2;

import std;
import interfaces;
import core_router;
import core_serde;
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

import :input;
import :result;
import :runtime;

export namespace app::handlers::verification {

class VerificationHandlersV2
{
public:
    VerificationHandlersV2(
        app::context::AppContext& ctx,
        VerificationRuntime& runtime,
        VerificationInputSupport& input_support
    ) noexcept :
        m_runtime{runtime},
        m_input_support{input_support}
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
        // 1. request -> Input
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error =
                m_input_support
                    .authorize_verification(auth, "apple_pay_merchant_registration", true)) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto merchant_id = m_input_support.merchant_id_from_path(req);
        if (!merchant_id) {
            const auto& error = merchant_id.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
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
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST,
                "empty request"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
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
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }
        ApplePayMerchantRegistrationInput input{
            *parsed,
            *merchant_id,
            std::string{auth.profile_id()}
        };

        // 2. Input -> Result
        auto result = m_runtime.apple_pay_merchant_registration(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // 3. Result -> response (typed upstream DTO, same wire fields).
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
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
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error =
                m_input_support
                    .authorize_verification(auth, "retrieve_apple_pay_verified_domains", true)) {
            const auto& error = *auth_error;
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto query = m_input_support.verified_domains_query(req);
        if (!query) {
            const auto& error = query.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        const auto authenticated = m_input_support.authenticated_merchant_id(auth, req);
        if (authenticated.empty() || authenticated != query->first) {
            const auto error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "Unauthorized"
            );
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        RetrieveApplePayVerifiedDomainsInput input{
            query->first,
            std::string{auth.profile_id()},
            query->second
        };

        // 2. Input -> Result
        auto result = m_runtime.retrieve_apple_pay_verified_domains(input);
        if (!result) {
            const auto& error = result.error();
            res.set_status(error.get_status());
            res.set_body(serde::Ser::serialize("application/json", error));
            send();
            return;
        }

        // 3. Result -> response (typed upstream DTO, same wire fields).
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
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
};

} // namespace app::handlers::verification
