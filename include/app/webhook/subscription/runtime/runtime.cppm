export module app_webhook:subscription_runtime;

import std;
import core_behavior;
import core_serde;
import core_error;
import app_context;

import :subscription_input;
import :subscription_runtime_common;
import :subscription_runtime_create;
import :subscription_runtime_pause;
import :subscription_runtime_resume;
import :subscription_runtime_cancel;
import :subscription_runtime_confirm;
import :subscription_runtime_get_subscription_items;
import :subscription_runtime_get_subscription;
import :subscription_runtime_create_and_confirm;
import :subscription_runtime_get_estimate;
import :subscription_runtime_update_subscription;
import :subscription_runtime_list_subscriptions;

export namespace app::handlers::subscription {

class SubscriptionRuntime
{
public:
    explicit SubscriptionRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_pause{context},
        m_resume{context},
        m_cancel{context},
        m_confirm{context},
        m_get_subscription_items{context},
        m_get_subscription{context},
        m_create_and_confirm{context},
        m_get_estimate{context},
        m_update_subscription{context},
        m_list_subscriptions{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    create(const ParsedPayload& payload, const ::core::behavior::CallContext& ctx) const
    {
        return m_create.create(payload, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> pause(
        const std::optional<ParsedPayload>& payload,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_pause.pause(payload, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> resume(
        const std::optional<ParsedPayload>& payload,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_resume.resume(payload, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> cancel(
        const std::optional<ParsedPayload>& payload,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_cancel.cancel(payload, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    confirm(const ParsedPayload& payload, const ::core::behavior::CallContext& ctx) const
    {
        return m_confirm.confirm(payload, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_subscription_items(const ::core::behavior::CallContext& ctx) const
    {
        return m_get_subscription_items.get_subscription_items(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_subscription(const ::core::behavior::CallContext& ctx) const
    {
        return m_get_subscription.get_subscription(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    create_and_confirm(const ParsedPayload& payload, const ::core::behavior::CallContext& ctx) const
    {
        return m_create_and_confirm.create_and_confirm(payload, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_estimate(const Json& payload, const ::core::behavior::CallContext& ctx) const
    {
        return m_get_estimate.get_estimate(payload, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> update_subscription(
        const ParsedPayload& payload,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_update_subscription.update_subscription(payload, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_subscriptions(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_subscriptions.list_subscriptions(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionCreateRuntime m_create;
    SubscriptionPauseRuntime m_pause;
    SubscriptionResumeRuntime m_resume;
    SubscriptionCancelRuntime m_cancel;
    SubscriptionConfirmRuntime m_confirm;
    SubscriptionGetSubscriptionItemsRuntime m_get_subscription_items;
    SubscriptionGetSubscriptionRuntime m_get_subscription;
    SubscriptionCreateAndConfirmRuntime m_create_and_confirm;
    SubscriptionGetEstimateRuntime m_get_estimate;
    SubscriptionUpdateSubscriptionRuntime m_update_subscription;
    SubscriptionListSubscriptionsRuntime m_list_subscriptions;
};

} // namespace app::handlers::subscription
