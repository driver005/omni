export module app_handlers_subscription:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_create;
import :runtime_pause;
import :runtime_resume;
import :runtime_cancel;
import :runtime_confirm;
import :runtime_get_subscription_items;
import :runtime_get_subscription;
import :runtime_create_and_confirm;
import :runtime_get_estimate;
import :runtime_update_subscription;
import :runtime_list_subscriptions;

export namespace app::handlers::subscription {

class SubscriptionRuntime
{
public:
    explicit SubscriptionRuntime(app::context::AppContext& context) noexcept :
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

    [[nodiscard]] std::expected<CreateSubscriptionResult, core::error::AppError>
    create(const CreateSubscriptionInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<PauseSubscriptionResult, core::error::AppError>
    pause(const PauseSubscriptionInput& input) const
    {
        return m_pause.pause(input);
    }

    [[nodiscard]] std::expected<ResumeSubscriptionResult, core::error::AppError>
    resume(const ResumeSubscriptionInput& input) const
    {
        return m_resume.resume(input);
    }

    [[nodiscard]] std::expected<CancelSubscriptionResult, core::error::AppError>
    cancel(const CancelSubscriptionInput& input) const
    {
        return m_cancel.cancel(input);
    }

    [[nodiscard]] std::expected<ConfirmSubscriptionResult, core::error::AppError>
    confirm(const ConfirmSubscriptionInput& input) const
    {
        return m_confirm.confirm(input);
    }

    [[nodiscard]] std::expected<GetSubscriptionItemsResult, core::error::AppError>
    get_subscription_items(const GetSubscriptionItemsInput& input) const
    {
        return m_get_subscription_items.get_subscription_items(input);
    }

    [[nodiscard]] std::expected<GetSubscriptionResult, core::error::AppError>
    get_subscription(const GetSubscriptionInput& input) const
    {
        return m_get_subscription.get_subscription(input);
    }

    [[nodiscard]] std::expected<CreateAndConfirmSubscriptionResult, core::error::AppError>
    create_and_confirm(const CreateAndConfirmSubscriptionInput& input) const
    {
        return m_create_and_confirm.create_and_confirm(input);
    }

    [[nodiscard]] std::expected<GetEstimateResult, core::error::AppError>
    get_estimate(const GetEstimateInput& input) const
    {
        return m_get_estimate.get_estimate(input);
    }

    [[nodiscard]] std::expected<UpdateSubscriptionResult, core::error::AppError>
    update_subscription(const UpdateSubscriptionInput& input) const
    {
        return m_update_subscription.update_subscription(input);
    }

    [[nodiscard]] std::expected<ListSubscriptionsResult, core::error::AppError>
    list_subscriptions(const ListSubscriptionsInput& input) const
    {
        return m_list_subscriptions.list_subscriptions(input);
    }

private:
    app::context::AppContext& m_context;
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
