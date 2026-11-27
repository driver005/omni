export module app_webhook:webhook_events_runtime_retry;

import std;
import core_behavior;
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

class WebhookEventsRetryRuntime
{
public:
    explicit WebhookEventsRetryRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream: POST /events/{merchant_id}/{event_id}/retry (v1 only)
    // routes/webhook_events.rs:142-178 -> core retry_delivery_attempt.
    [[nodiscard]] core::behavior::Outcome<::app::dto::webhook_events::EventRetrieveResponse>
    retry(const ::core::behavior::CallContext& ctx) const
    {
        const std::string event_id = ctx.path_segments.size() > 2 ? ctx.path_segments.at(2) : "";
        const std::string lock_key = "webhook_retry:" + ctx.merchant_id + ":" + event_id;
        const auto lock = m_context.locks().acquire(lock_key, std::int64_t{30}, "webhook_events");
        if (!lock) {
            m_context.logger().counter("WEBHOOK_EVENT_RETRY_LOCK_CONTENTED").increment();
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::DUPLICATE_REQUEST,
                "retry already in progress"
            )};
        }

        // find_event_by_event_id + owner check with legacy merchant_id fallback
        // (core:364-371).
        const auto original = m_context.repository().find_webhook_event_by_event_id(event_id);
        const auto original_payload =
            original ? m_support.payload_of(*original, ctx.merchant_id) : std::string{};
        if (!original || m_support.initiator_of(*original, original_payload) != ctx.merchant_id) {
            m_context.locks().release(lock_key);
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "event not found")
            };
        }

        // provider / processor / profile resolution (core:373-395).
        const std::string provider_merchant_id =
            original->get_merchant_id().empty() ? ctx.merchant_id : original->get_merchant_id();
        const std::string stored_processor =
            m_support.payload_field(original_payload, "processor_merchant_id");
        const std::string processor_merchant_id =
            stored_processor.empty() ? provider_merchant_id : stored_processor;
        const std::string business_profile_id =
            m_support.profile_id_of(original_payload, ctx.profile_id);
        if (const auto profile_error =
                m_support.validate_profile(business_profile_id, ctx.merchant_id)) {
            m_context.locks().release(lock_key);
            return std::unexpected{*profile_error};
        }
        const auto sensitive = m_support.sensitive_header_names(business_profile_id);

        // generate_event_id + get_idempotent_event_id (core:397-429; utils.rs:229-261).
        const std::string new_event_id = m_support.generate_event_id();
        const std::string idempotent_event_id = m_support.get_idempotent_event_id(
            m_support.object_id_of(original_payload),
            original->get_event_type(),
            "manual_retry"
        );

        // get_idempotent_event_id: a repeated retry returns the event already created.
        for (const auto& candidate: m_context.repository().list_webhook_events(
                 ::app::store::WebhookEventQuery{}.merchant_id(ctx.merchant_id)
             )) {
            const auto payload = m_support.payload_of(candidate, ctx.merchant_id);
            if (m_support.payload_field(payload, "idempotent_event_id") == idempotent_event_id) {
                m_context.locks().release(lock_key);
                m_context.logger().record_flow("WebhookEventDeliveryRetry");
                m_context.logger().counter("WEBHOOK_EVENT_DELIVERY_RETRY").increment();
                auto item = m_support.to_list_item(
                    candidate,
                    payload,
                    DeliverySource::ListDeliveryAttempts,
                    ctx.profile_id
                );
                if (!item) {
                    return std::unexpected{std::move(item.error())};
                }
                return m_support.to_retrieve(*item, candidate, payload, sensitive);
            }
        }

        Record retry{};
        retry.set_event_id(new_event_id);
        retry.set_merchant_id(provider_merchant_id);
        retry.set_event_type(original->get_event_type());
        retry.set_event_class(original->get_event_class());
        retry.set_status("initial_attempt");
        retry.set_payload(m_support.build_retry_payload(
            new_event_id,
            m_support.initial_attempt_id_of(*original, original_payload),
            idempotent_event_id,
            processor_merchant_id,
            provider_merchant_id,
            business_profile_id,
            ctx.merchant_id,
            m_support.recipient_of(original_payload),
            original_payload
        ));
        retry.set_delivery_attempts(original->get_delivery_attempts() + 1);
        retry.set_created_at(m_support.now_value());

        auto inserted = m_context.repository().insert_webhook_event(std::move(retry));
        if (!inserted) {
            m_context.locks().release(lock_key);
            return std::unexpected{inserted.error()};
        }

        // trigger_webhook_and_raise_event (core:431-435): publish the bus event,
        // then drive one deterministic local delivery attempt.
        const std::string bus_event_id =
            m_context.events().publish("webhook.delivery.retry", inserted->get_payload());
        const std::string endpoint =
            m_context.config_store().get_value("webhook_endpoints", ctx.merchant_id).value_or("");
        m_context.webhook_delivery().enqueue(bus_event_id, endpoint);
        if (!m_context.webhook_delivery().attempt(bus_event_id)) {
            m_context.webhook_delivery().retry(bus_event_id, ::app::workflow::RetryPolicy{});
        }

        // Re-fetch the saved event (core:450-478).
        const auto stored = m_context.repository().find_webhook_event_by_event_id(new_event_id);
        if (!stored) {
            m_context.locks().release(lock_key);
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::NOT_FOUND, "event not found")
            };
        }
        const auto stored_payload = m_support.payload_of(*stored, ctx.merchant_id);
        m_context.locks().release(lock_key);

        auto item = m_support.to_list_item(
            *stored,
            stored_payload,
            DeliverySource::ListDeliveryAttempts,
            ctx.profile_id
        );
        if (!item) {
            return std::unexpected{std::move(item.error())};
        }
        m_context.logger().record_flow("WebhookEventDeliveryRetry");
        m_context.logger().counter("WEBHOOK_EVENT_DELIVERY_RETRY").increment();
        return m_support.to_retrieve(*item, *stored, stored_payload, sensitive);
    }

private:
    ::app::context::AppContext& m_context;
    WebhookEventsRuntimeSupport m_support;
};

} // namespace app::handlers::webhook_events
