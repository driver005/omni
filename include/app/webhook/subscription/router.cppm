export module app_webhook:subscription_router;

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
import :subscription_handlers_lifecycle;

export namespace app::handlers::subscription {

// Facade ("handler of handlers"). Owns the single shared SubscriptionRuntime;
// each family child holds a reference to it, never its own runtime instance.
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
        interfaces::io::ResponseSink sink{res, std::move(send)};
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        Json payload = m_input_support.json_from_query(query);
        if (const auto failure =
                m_request_support
                    .authorize(req, "sdk_query", ::app::webhook::policy::kSubscriptionRead)) {
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
        ctx.client_secret = m_input_support.effective_client_secret(auth, payload);
        m_runtime.get_subscription_items(ctx).handle(sink);
    }

    // ── GET /subscriptions/estimate — get_estimate ──────────────────────────────
    void handle_get_estimate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        const auto query = ::app::http::RequestQuery{}(req);
        Json payload = m_input_support.json_from_query(query);

        // check_sdk_auth_and_get_auth (routes/subscription.rs:432-457)
        if (const auto failure =
                m_request_support
                    .authorize(req, "sdk_query", ::app::webhook::policy::kSubscriptionWrite)) {
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
        ctx.client_secret = m_input_support.effective_client_secret(auth, payload);
        m_runtime.get_estimate(payload, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntime& m_runtime;
    SubscriptionInputSupport m_input_support{};
    RequestSupport& m_request_support;
};

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

class UpdateHandlers
{
public:
    using RequestSupport =
        ::core::behavior::RequestSupport<::app::webhook::policy::SubscriptionPolicy>;

    UpdateHandlers(
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
        Router{router, "/subscriptions"}.add_router(
            Router{router, ":subscription_id"}.add_route(
                Route{"update"}
                    .put(
                        [this](
                            interfaces::io::IRequest& req,
                            interfaces::io::IResponse& res,
                            std::function<void()> send
                        )
                        {
                            handle_update_subscription(req, res, std::move(send));
                        }
                    )
                    .summary("Hyperswitch update_subscription")
            )
        );
    }

    // ── PUT /subscriptions/:id/update — update_subscription ─────────────────────
    void handle_update_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        interfaces::io::ResponseSink sink{res, std::move(send)};
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        if (const auto failure = m_request_support.authorize(req, "api_key", {})) {
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
        auto payload = m_input_support.parse_json_body(req);
        if (!payload) {
            const auto& error = payload.error();
            error.send(sink);
            return;
        }
        const auto plan_id = m_input_support.json_string(payload->root, "plan_id");
        const auto item_price_id = m_input_support.json_string(payload->root, "item_price_id");
        if (!plan_id || !item_price_id) {
            const auto error = m_input_support.json_body_error(
                "missing field `plan_id` or `item_price_id` at line 1 column 2"
            );
            error.send(sink);
            return;
        }

        auto ctx = m_request_support.context(req);
        ctx.profile_id = profile.value();
        ctx.merchant_id = m_input_support.merchant_id(req, auth);
        m_runtime.update_subscription(*payload, ctx).handle(sink);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntime& m_runtime;
    SubscriptionInputSupport m_input_support{};
    RequestSupport& m_request_support;
};

class SubscriptionRouteHandlers
{
public:
    using RequestSupport =
        ::core::behavior::RequestSupport<::app::webhook::policy::SubscriptionPolicy>;

    explicit SubscriptionRouteHandlers(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_runtime{context},
        m_policy{},
        m_request_support{m_policy},
        m_lifecycle{context, m_runtime, m_request_support},
        m_estimate_items{context, m_runtime, m_request_support},
        m_list{context, m_runtime, m_request_support},
        m_update{context, m_runtime, m_request_support}
    {
    }

    void install_routes(core::router::RouterContext<>& router)
    {
        m_lifecycle.install(router);
        m_estimate_items.install(router);
        m_list.install(router);
        m_update.install(router);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntime m_runtime; // SINGLE runtime — shared state not duplicated
    ::app::webhook::policy::SubscriptionPolicy m_policy;
    RequestSupport m_request_support;
    LifecycleHandlers m_lifecycle; // holds AppContext& + SubscriptionRuntime&
    EstimateItemsHandlers m_estimate_items;
    ListHandlers m_list;
    UpdateHandlers m_update;
};

} // namespace app::handlers::subscription
