export module app_handlers_webhook_events:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_list_initial_with_jwtauth;
import :runtime_list_initial;
import :runtime_list_attempts;
import :runtime_retry;

export namespace app::handlers::webhook_events {

class WebhookEventsRuntime
{
public:
    explicit WebhookEventsRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_list_initial_with_jwtauth{context},
        m_list_initial{context},
        m_list_attempts{context},
        m_retry{context}
    {
    }

    [[nodiscard]] std::expected<ListInitialWithJwtAuthResult, core::error::AppError>
    list_initial_with_jwtauth(const ListInitialWithJwtAuthInput& input) const
    {
        return m_list_initial_with_jwtauth.list_initial_with_jwtauth(input);
    }

    [[nodiscard]] std::expected<ListInitialResult, core::error::AppError>
    list_initial(const ListInitialInput& input) const
    {
        return m_list_initial.list_initial(input);
    }

    [[nodiscard]] std::expected<ListAttemptsResult, core::error::AppError>
    list_attempts(const ListAttemptsInput& input) const
    {
        return m_list_attempts.list_attempts(input);
    }

    [[nodiscard]] std::expected<RetryResult, core::error::AppError>
    retry(const RetryInput& input) const
    {
        return m_retry.retry(input);
    }

private:
    app::context::AppContext& m_context;
    WebhookEventsListInitialWithJwtAuthRuntime m_list_initial_with_jwtauth;
    WebhookEventsListInitialRuntime m_list_initial;
    WebhookEventsListAttemptsRuntime m_list_attempts;
    WebhookEventsRetryRuntime m_retry;
};

} // namespace app::handlers::webhook_events
