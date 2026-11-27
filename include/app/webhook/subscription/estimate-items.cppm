export module app_webhook:subscription_handlers_estimate_items;

import std;
import interfaces;
import core_router;
import core_serde;
import core_behavior;
import utils_openapi;
import app_context;
import core_error;
import app_auth;
import app_http;

import :subscription_input;
import :webhook_policy;
import :subscription_runtime;

export namespace app::handlers::subscription {

class EstimateItemsHandlers
{
public:
    using RequestSupport =
        ::core::behavior::RequestSupport<::app::webhook::policy::SubscriptionPolicy>;

    EstimateItemsHandlers(
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
                Route{"items"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_subscription_items(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_subscription_items")
            )
            .add_route(
                Route{"estimate"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_estimate(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_estimate")
            );
    }

    // ── GET /subscriptions/items — get_subscription_items ───────────────────────
    void handle_get_subscription_items(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        Json payload = m_input_support.json_from_query(query);
        if (const auto failure =
                m_request_support
                    .authorize(req, "sdk_query", ::app::webhook::policy::kSubscriptionRead)) {
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

        const std::string client_secret = m_input_support.effective_client_secret(auth, payload);
        const auto merchant = m_input_support.merchant_id(req, auth);

        const std::uint32_t limit =
            static_cast<std::uint32_t>(m_input_support.query_unsigned(query, "limit", 10));
        const std::uint32_t offset =
            static_cast<std::uint32_t>(m_input_support.query_unsigned(query, "offset", 0));
        const std::string item_type = query.get("item_type").value_or("plan");

        GetSubscriptionItemsInput
            input{profile.value(), merchant, client_secret, limit, offset, item_type};
        auto result = m_runtime.get_subscription_items(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", *result));
        send();
    }

    // ── GET /subscriptions/estimate — get_estimate ──────────────────────────────
    void handle_get_estimate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        Json payload = m_input_support.json_from_query(query);

        // check_sdk_auth_and_get_auth (routes/subscription.rs:432-457)
        if (const auto failure =
                m_request_support
                    .authorize(req, "sdk_query", ::app::webhook::policy::kSubscriptionWrite)) {
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

        const std::string client_secret = m_input_support.effective_client_secret(auth, payload);
        const auto merchant = m_input_support.merchant_id(req, auth);
        GetEstimateInput input{profile.value(), merchant, client_secret, payload};
        auto result = m_runtime.get_estimate(input);
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
