export module app_handlers_webhook_events:runtime_list_attempts;

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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::webhook_events {

class WebhookEventsListAttemptsRuntime
{
public:
    explicit WebhookEventsListAttemptsRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream: GET /events/{merchant_id}/{initial_attempt_id}/attempts
    // routes/webhook_events.rs:105-140 -> core list_delivery_attempts.
    [[nodiscard]] std::expected<ListAttemptsResult, core::error::AppError>
    list_attempts(const ListAttemptsInput& input) const
    {
        // list_events_by_initiator_merchant_id_initial_attempt_id scopes by the
        // unique initial_attempt_id; ownership is verified below for every row.
        std::vector<std::pair<Record, std::string>> chain{};
        for (const auto& record: m_context.repository().list_webhook_events()) {
            const auto payload = m_support.payload_of(record, input.merchant_id);
            if (m_support.initial_attempt_id_of(record, payload) != input.event_id) {
                continue;
            }
            if (m_support.initiator_of(record, payload) != input.merchant_id) {
                continue;
            }
            chain.emplace_back(record, payload);
        }
        if (chain.empty()) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "event not found")
            };
        }

        // All rows in a chain share a business profile: one lookup covers the list.
        // A missing profile or a failed lookup yields nullopt => redact every value
        // (core/webhooks/utils.rs:576-588, core:300-318).
        const auto sensitive =
            m_support.sensitive_header_names(m_support.profile_id_of(chain.front().second));

        std::vector<app::dto::webhook_events::EventRetrieveResponse> events{};
        events.reserve(chain.size());
        for (const auto& [record, payload]: chain) {
            auto item = m_support.to_list_item(
                record,
                payload,
                DeliverySource::ListDeliveryAttempts,
                input.profile_id
            );
            if (!item) {
                return std::unexpected{std::move(item.error())};
            }
            events.push_back(m_support.to_retrieve(*item, record, payload, sensitive));
        }

        m_context.metrics().record_flow("WebhookEventDeliveryAttemptList");
        m_context.metrics().counter("WEBHOOK_EVENT_DELIVERY_ATTEMPTS_LIST").increment();
        return ListAttemptsResult{std::move(events)};
    }

private:
    app::context::AppContext& m_context;
    WebhookEventsRuntimeSupport m_support;
};

} // namespace app::handlers::webhook_events
