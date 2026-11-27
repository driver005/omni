export module app_webhook:subscription_handlers_lifecycle;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_context;
import core_error;
import app_auth;

import :subscription_input;
import :webhook_policy;
import :subscription_runtime;

export namespace app::handlers::subscription {

class LifecycleHandlers
{
public:
    using RequestSupport =
        ::core::behavior::RequestSupport<::app::webhook::policy::SubscriptionPolicy>;

    LifecycleHandlers(
        ::app::context::AppContext& ctx,
        SubscriptionRuntime& runtime,
        RequestSupport& request_support
    ) noexcept :
        m_context{ctx},
        m_runtime{runtime},
        m_request_support{request_support}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/subscriptions"}
            .add_route(
                Route{"create"}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_create_subscription(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch create_subscription")
            )
            .add_router(
                Router{router, ":subscription_id"}.add_route(
                    Route{"pause"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_pause_subscription(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch pause_subscription")
                )
            )
            .add_router(
                Router{router, ":subscription_id"}.add_route(
                    Route{"resume"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_resume_subscription(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch resume_subscription")
                )
            )
            .add_router(
                Router{router, ":subscription_id"}.add_route(
                    Route{"cancel"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_cancel_subscription(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch cancel_subscription")
                )
            )
            .add_router(
                Router{router, ":subscription_id"}.add_route(
                    Route{"confirm"}
                        .post(
                            [this](
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send
                            )
                            {
                                handle_confirm_subscription(req, res, std::move(send));
                            }
                        )
                        .summary("Hyperswitch confirm_subscription")
                )
            )
            .add_route(
                Route{""}
                    .post(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_create_and_confirm_subscription(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch create_and_confirm_subscription")
            );
    }

    // ── POST /subscriptions/create — create_subscription ────────────────────────
    void handle_create_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto failure =
                m_request_support
                    .authorize(req, "api_key_or_jwt", ::app::webhook::policy::kSubscriptionWrite)) {
            const auto& error = *failure;
            error.send(res, send);
            return;
        }
        auto profile = m_input_support.extract_profile_id(req);
        if (!profile) {
            const auto& error = profile.error();
            error.send(res, send);
            return;
        }
        auto payload = m_input_support.parse_json_body(req);
        if (!payload) {
            const auto& error = payload.error();
            error.send(res, send);
            return;
        }
        const auto customer_id = m_input_support.json_string(payload->root, "customer_id");
        const auto item_price_id = m_input_support.json_string(payload->root, "item_price_id");
        if (!customer_id || customer_id->empty() || !item_price_id || item_price_id->empty()) {
            const auto error = m_input_support.json_body_error(
                "missing field `item_price_id` or `customer_id` at line 1 column 2"
            );
            error.send(res, send);
            return;
        }

        const auto merchant = m_input_support.merchant_id(req, auth);
        CreateSubscriptionInput
            input{profile.value(), merchant, *customer_id, *item_price_id, *payload};
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

    // ── POST /subscriptions/:id/pause — pause_subscription ──────────────────────
    void handle_pause_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto failure = m_request_support.authorize(req, "api_key", {})) {
            const auto& error = *failure;
            error.send(res, send);
            return;
        }
        auto profile = m_input_support.extract_profile_id(req);
        if (!profile) {
            const auto& error = profile.error();
            error.send(res, send);
            return;
        }
        const auto id = m_input_support.subscription_id_from_request(req);
        if (id.empty()) {
            const auto error = m_input_support.generic_not_found("subscription not found for id: ");
            error.send(res, send);
            return;
        }
        auto payload = m_input_support.parse_json_body_optional(req);

        PauseSubscriptionInput
            input{profile.value(), m_input_support.merchant_id(req, auth), id, payload};
        auto result = m_runtime.pause(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // ── POST /subscriptions/:id/resume — resume_subscription ────────────────────
    void handle_resume_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto failure = m_request_support.authorize(req, "api_key", {})) {
            const auto& error = *failure;
            error.send(res, send);
            return;
        }
        auto profile = m_input_support.extract_profile_id(req);
        if (!profile) {
            const auto& error = profile.error();
            error.send(res, send);
            return;
        }
        const auto id = m_input_support.subscription_id_from_request(req);
        if (id.empty()) {
            const auto error = m_input_support.generic_not_found("subscription not found for id: ");
            error.send(res, send);
            return;
        }
        auto payload = m_input_support.parse_json_body_optional(req);

        ResumeSubscriptionInput
            input{profile.value(), m_input_support.merchant_id(req, auth), id, payload};
        auto result = m_runtime.resume(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // ── POST /subscriptions/:id/cancel — cancel_subscription ────────────────────
    void handle_cancel_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto failure = m_request_support.authorize(req, "api_key", {})) {
            const auto& error = *failure;
            error.send(res, send);
            return;
        }
        auto profile = m_input_support.extract_profile_id(req);
        if (!profile) {
            const auto& error = profile.error();
            error.send(res, send);
            return;
        }
        const auto id = m_input_support.subscription_id_from_request(req);
        if (id.empty()) {
            const auto error = m_input_support.generic_not_found("subscription not found for id: ");
            error.send(res, send);
            return;
        }
        auto payload = m_input_support.parse_json_body_optional(req);

        CancelSubscriptionInput
            input{profile.value(), m_input_support.merchant_id(req, auth), id, payload};
        auto result = m_runtime.cancel(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // ── POST /subscriptions/:id/confirm — confirm_subscription ──────────────────
    void handle_confirm_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto raw_body = req.body_as_string();
        const auto auth = ::app::auth::AuthContextBuilder{}(req, raw_body);
        auto payload = m_input_support.parse_json_body(req);
        if (!payload) {
            const auto& error = payload.error();
            error.send(res, send);
            return;
        }

        // check_sdk_auth_and_get_auth + auth_type (routes/subscription.rs:214-265)
        if (const auto failure =
                m_request_support
                    .authorize(req, "sdk_body", ::app::webhook::policy::kSubscriptionWrite)) {
            const auto& error = *failure;
            error.send(res, send);
            return;
        }
        auto profile = m_input_support.extract_profile_id(req);
        if (!profile) {
            const auto& error = profile.error();
            error.send(res, send);
            return;
        }

        // request.validate() -> ConfirmSubscriptionPaymentDetails::validate
        // (api_models:217-231,396-409)
        const auto* payment_details =
            m_input_support.nested_object(payload->root, "payment_details");
        if (!m_input_support.has_payment_method_data_or_token(payment_details)) {
            const auto error = m_input_support.invalid_request_data(
                "Either payment_method_data or payment_token must be present"
            );
            error.send(res, send);
            return;
        }

        const auto id = m_input_support.subscription_id_from_request(req);
        if (id.empty()) {
            const auto error = m_input_support.generic_not_found("subscription not found for id: ");
            error.send(res, send);
            return;
        }

        // The SDK flow overwrites payload.client_secret with the authenticated claim.
        const std::string client_secret =
            m_input_support.effective_client_secret(auth, payload->root);
        const auto merchant = m_input_support.merchant_id(req, auth);
        ConfirmSubscriptionInput input{profile.value(), merchant, id, client_secret, *payload};
        auto result = m_runtime.confirm(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // ── POST /subscriptions — create_and_confirm_subscription ──────────────────
    void handle_create_and_confirm_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto failure =
                m_request_support
                    .authorize(req, "api_key_or_jwt", ::app::webhook::policy::kSubscriptionWrite)) {
            const auto& error = *failure;
            error.send(res, send);
            return;
        }
        auto profile = m_input_support.extract_profile_id(req);
        if (!profile) {
            const auto& error = profile.error();
            error.send(res, send);
            return;
        }
        auto payload = m_input_support.parse_json_body(req);
        if (!payload) {
            const auto& error = payload.error();
            error.send(res, send);
            return;
        }

        // request.validate() -> PaymentDetails::validate (api_models:261-275,440-452)
        const auto* payment_details =
            m_input_support.nested_object(payload->root, "payment_details");
        if (!m_input_support.has_payment_method_data_or_id(payment_details)) {
            const auto error = m_input_support.invalid_request_data(
                "Either payment_method_data or payment_method_id must be present"
            );
            error.send(res, send);
            return;
        }
        const auto customer_id = m_input_support.json_string(payload->root, "customer_id");
        const auto item_price_id = m_input_support.json_string(payload->root, "item_price_id");
        if (!customer_id || customer_id->empty() || !item_price_id || item_price_id->empty()) {
            const auto error = m_input_support.json_body_error(
                "missing field `item_price_id` or `customer_id` at line 1 column 2"
            );
            error.send(res, send);
            return;
        }

        const auto merchant = m_input_support.merchant_id(req, auth);
        CreateAndConfirmSubscriptionInput
            input{profile.value(), merchant, *customer_id, *item_price_id, *payload};
        auto result = m_runtime.create_and_confirm(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntime& m_runtime;
    SubscriptionInputSupport m_input_support{};
    RequestSupport& m_request_support;
};

} // namespace app::handlers::subscription
