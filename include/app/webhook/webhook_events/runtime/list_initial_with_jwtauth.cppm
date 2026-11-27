export module app_webhook:webhook_events_runtime_list_initial_with_jwtauth;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :webhook_events_input;
import :webhook_events_result;
import :webhook_events_runtime_common;

export namespace app::handlers::webhook_events {

class WebhookEventsListInitialWithJwtAuthRuntime
{
public:
    explicit WebhookEventsListInitialWithJwtAuthRuntime(
        ::app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream: POST /events/profile/list
    // routes/webhook_events.rs:57-103 -> core list_initial_delivery_attempts.
    [[nodiscard]] std::
        expected<::app::dto::webhook_events::TotalEventsResponse, core::error::AppError>
        list_initial_with_jwtauth(const ListInitialWithJwtAuthInput& input) const
    {
        auto listed = m_support.list_initial_delivery_attempts(
            input.merchant_id,
            input.profile_id,
            input.constraints
        );
        if (!listed) {
            return std::unexpected{std::move(listed.error())};
        }

        m_context.metrics().record_flow("WebhookEventInitialDeliveryAttemptList");
        m_context.metrics().counter("WEBHOOK_EVENT_INITIAL_ATTEMPTS_LIST").increment();
        return m_support.to_total_events_response(std::move(*listed));
    }

private:
    ::app::context::AppContext& m_context;
    WebhookEventsRuntimeSupport m_support;
};

} // namespace app::handlers::webhook_events
