export module app_webhook:subscription_runtime_get_subscription_items;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_store;
import app_crypto;
import app_connector;
import app_metrics;
import app_events;

import :subscription_input;
import :subscription_result;
import :subscription_runtime_common;

export namespace app::handlers::subscription {

class SubscriptionGetSubscriptionItemsRuntime
{
public:
    explicit SubscriptionGetSubscriptionItemsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // get_subscription_items + get_subscription_item_prices (core.rs:151-172)
    [[nodiscard]] std::expected<GetSubscriptionItemsResult, core::error::AppError>
    get_subscription_items(const GetSubscriptionItemsInput& input) const
    {
        if (const auto failure =
                m_support.find_business_profile(input.profile_id, input.merchant_id)) {
            return std::unexpected{*failure};
        }
        if (!input.client_secret.empty()) {
            if (const auto failure =
                    m_support.validate_client_secret(input.client_secret, input.merchant_id)) {
                return std::unexpected{*failure};
            }
        }
        BillingProcessor billing{m_context};
        if (const auto failure = billing.create(input.profile_id, input.merchant_id)) {
            return std::unexpected{*failure};
        }

        auto items =
            billing.get_items(input.profile_id, input.limit, input.offset, input.item_type);
        m_support.metrics_for("subscription.get_subscription_items");
        return GetSubscriptionItemsResult{m_support.build_items_response(items)};
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
