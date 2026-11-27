export module app_webhook:webhook_events_runtime;

import std;
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

    [[nodiscard]] std::
        expected<::app::dto::webhook_events::TotalEventsResponse, core::error::AppError>
        list_initial_with_jwtauth(const ListInitialWithJwtAuthInput& input) const
    {
        return m_list_initial_with_jwtauth.list_initial_with_jwtauth(input);
    }

    [[nodiscard]] std::
        expected<::app::dto::webhook_events::TotalEventsResponse, core::error::AppError>
        list_initial(const ListInitialInput& input) const
    {
        return m_list_initial.list_initial(input);
    }

    [[nodiscard]] std::expected<
        std::vector<::app::dto::webhook_events::EventRetrieveResponse>,
        core::error::AppError>
    list_attempts(const ListAttemptsInput& input) const
    {
        return m_list_attempts.list_attempts(input);
    }

    [[nodiscard]] std::
        expected<::app::dto::webhook_events::EventRetrieveResponse, core::error::AppError>
        retry(const RetryInput& input) const
    {
        return m_retry.retry(input);
    }

private:
    ::app::context::AppContext& m_context;
    WebhookEventsListInitialWithJwtAuthRuntime m_list_initial_with_jwtauth;
    WebhookEventsListInitialRuntime m_list_initial;
    WebhookEventsListAttemptsRuntime m_list_attempts;
    WebhookEventsRetryRuntime m_retry;
};

} // namespace app::handlers::webhook_events
