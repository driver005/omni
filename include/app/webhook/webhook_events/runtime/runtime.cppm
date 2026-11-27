export module app_webhook:webhook_events_runtime;

import std;
import core_behavior;
import core_error;
import app_context;
import app_dto;

import :webhook_events_input;
import :webhook_events_result;
import :webhook_events_runtime_common;
import :webhook_events_runtime_list_initial_with_jwtauth;
import :webhook_events_runtime_list_initial;
import :webhook_events_runtime_list_attempts;
import :webhook_events_runtime_retry;

export namespace app::handlers::webhook_events {

class WebhookEventsRuntime
{
public:
    explicit WebhookEventsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_list_initial_with_jwtauth{context},
        m_list_initial{context},
        m_list_attempts{context},
        m_retry{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::webhook_events::TotalEventsResponse>
    list_initial_with_jwtauth(
        const ::app::dto::webhook_events::EventListConstraints& constraints,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_list_initial_with_jwtauth.list_initial_with_jwtauth(constraints, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::webhook_events::TotalEventsResponse>
    list_initial(
        const ::app::dto::webhook_events::EventListConstraints& constraints,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_list_initial.list_initial(constraints, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<
        std::vector<::app::dto::webhook_events::EventRetrieveResponse>>
    list_attempts(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_attempts.list_attempts(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::webhook_events::EventRetrieveResponse>
    retry(const ::core::behavior::CallContext& ctx) const
    {
        return m_retry.retry(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    WebhookEventsListInitialWithJwtAuthRuntime m_list_initial_with_jwtauth;
    WebhookEventsListInitialRuntime m_list_initial;
    WebhookEventsListAttemptsRuntime m_list_attempts;
    WebhookEventsRetryRuntime m_retry;
};

} // namespace app::handlers::webhook_events
