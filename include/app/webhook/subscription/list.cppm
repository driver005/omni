export module app_webhook:subscription_handlers_list;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_context;
import core_error;
import app_auth;
import app_http;

import :subscription_input;
import :subscription_result;
import :subscription_runtime;

export namespace app::handlers::subscription {

class ListHandlers
{
public:
    ListHandlers(::app::context::AppContext& ctx, SubscriptionRuntime& runtime) noexcept :
        m_context{ctx},
        m_runtime{runtime}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/subscriptions"}
            .add_route(
                Route{":subscription_id"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_get_subscription(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch get_subscription")
            )
            .add_route(
                Route{"list"}
                    .get(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_list_subscriptions(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch list_subscriptions")
            );
    }

    // ── GET /subscriptions/:id — get_subscription ───────────────────────────────
    void handle_get_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto failure =
                m_input_support.authorize_api_key_or_jwt(req, auth, kReadPermission)) {
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

        GetSubscriptionInput input{profile.value(), m_input_support.merchant_id(req, auth), id};
        auto result = m_runtime.get_subscription(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

    // ── GET /subscriptions/list — list_subscriptions ────────────────────────────
    void handle_list_subscriptions(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto failure =
                m_input_support.authorize_api_key_or_jwt(req, auth, kReadPermission)) {
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

        const auto query = ::app::http::RequestQuery{}(req);
        const std::int64_t limit = m_input_support.query_number(query, "limit", kMaxListCount);
        const std::int64_t offset = m_input_support.query_number(query, "offset", 0);

        ListSubscriptionsInput
            input{profile.value(), m_input_support.merchant_id(req, auth), limit, offset};
        auto result = m_runtime.list_subscriptions(input);
        if (!result) {
            const auto& error = result.error();
            error.send(res, send);
            return;
        }
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(serde::Ser::serialize("application/json", result->get_document()));
        send();
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntime& m_runtime;
    SubscriptionInputSupport m_input_support{};
};

} // namespace app::handlers::subscription
