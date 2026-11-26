export module app_webhook:subscription_runtime_list_subscriptions;

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

class SubscriptionListSubscriptionsRuntime
{
public:
    explicit SubscriptionListSubscriptionsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // list_subscriptions_by_profile_id(profile, limit, offset) (core.rs:741-748)
    [[nodiscard]] std::expected<ListSubscriptionsResult, core::error::AppError>
    list_subscriptions(const ListSubscriptionsInput& input) const
    {
        if (const auto failure =
                m_support.find_business_profile(input.profile_id, input.merchant_id)) {
            return std::unexpected{*failure};
        }

        std::vector<SubscriptionRecord> records{};
        for (const auto& record: m_context.repository().list_subscriptions()) {
            if (record.get_profile_id() == input.profile_id) {
                records.push_back(m_support.from_store_record(record));
            }
        }
        const std::size_t begin =
            input.offset > 0 ? std::min(static_cast<std::size_t>(input.offset), records.size())
                             : 0U;
        const std::size_t count = input.limit > 0 ? static_cast<std::size_t>(input.limit) : 0U;
        const std::size_t stop = std::min(begin + count, records.size());

        Json::Array out{};
        for (std::size_t i = begin; i < stop; ++i) {
            out.push_back(m_support.build_subscription_response(
                records[i],
                std::nullopt,
                std::nullopt,
                std::nullopt
            ));
        }
        m_support.metrics_for("subscription.list");
        return ListSubscriptionsResult{Json{std::move(out)}};
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
