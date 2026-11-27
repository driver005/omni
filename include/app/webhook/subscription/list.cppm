export module app_webhook:subscription_handlers_list;

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

class ListHandlers
{
public:
    using RequestSupport =
        ::core::behavior::RequestSupport<::app::webhook::policy::SubscriptionPolicy>;

    ListHandlers(
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
        interfaces::io::ResponseSink sink{res, std::move(send)};
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto failure =
                m_request_support
                    .authorize(req, "api_key_or_jwt", ::app::webhook::policy::kSubscriptionRead)) {
            const auto& error = *failure;
            error.send(sink);
            return;
        }
        auto profile = m_input_support.extract_profile_id(req);
        if (!profile) {
            const auto& error = profile.error();
            error.send(sink);
            return;
        }
        const auto id = m_input_support.subscription_id_from_request(req);
        if (id.empty()) {
            const auto error = m_input_support.generic_not_found("subscription not found for id: ");
            error.send(sink);
            return;
        }

        auto ctx = m_request_support.context(req);
        ctx.profile_id = profile.value();
        ctx.merchant_id = m_input_support.merchant_id(req, auth);
        m_runtime.get_subscription(ctx).handle(sink);
    }

    // ── GET /subscriptions/list — list_subscriptions ────────────────────────────
    void handle_list_subscriptions(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto failure =
                m_request_support
                    .authorize(req, "api_key_or_jwt", ::app::webhook::policy::kSubscriptionRead)) {
            const auto& error = *failure;
            error.send(sink);
            return;
        }
        auto profile = m_input_support.extract_profile_id(req);
        if (!profile) {
            const auto& error = profile.error();
            error.send(sink);
            return;
        }

        auto ctx = m_request_support.context(req);
        ctx.profile_id = profile.value();
        ctx.merchant_id = m_input_support.merchant_id(req, auth);
        m_runtime.list_subscriptions(ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntime& m_runtime;
    SubscriptionInputSupport m_input_support{};
    RequestSupport& m_request_support;
};

} // namespace app::handlers::subscription
