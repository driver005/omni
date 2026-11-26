export module app_payment:relay_runtime_incoming_relay_webhook;

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
import app_locking;
import app_services;

import :relay_input;
import :relay_result;
import :relay_runtime_common;

export namespace app::handlers::relay {

class RelayIncomingWebhookRuntime
{
public:
    explicit RelayIncomingWebhookRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /**
     * @brief Upstream POST /webhooks/relay/{merchant_id}/{connector_id}.
     *
     * Port of `receive_incoming_relay_webhook` (routes/webhooks.rs:48-83,
     * routes/app.rs:2374-2382) and its core chain:
     *   incoming_webhooks_wrapper (core/webhooks/incoming.rs:78-119)
     *   -> incoming_webhooks_core (:177-471)
     *   -> process_webhook_business_logic (:541-803)
     *   -> relay_incoming_webhook_flow (:1789-1825)
     *   -> relay_refunds_incoming_webhook_flow (:1550-1634)
     *   -> validate_webhook_connector_account (:3557-3595).
     *
     * Connector-specific decoding/gateway and the outbound HTTP client are
     * external infra, replaced by deterministic local equivalents using the
     * connector registry and the relay record store.
     */
    [[nodiscard]] std::expected<RelayWebhookResult, core::error::AppError>
    incoming_webhook(const RelayIncomingWebhookInput& input) const
    {
        // Upstream consumes `web::Bytes` verbatim; there is no JSON requirement
        // on the wire, so the raw body is kept for verification and decoding.
        // Upstream: validate_webhook_connector_account (incoming.rs:3557-3595).
        // The v1 relay path carries a merchant connector account id; a missing
        // account maps to Unauthorized (401).
        const auto account =
            m_support.validate_webhook_connector_account(input.merchant_id, input.connector_id);
        if (account.failure) {
            return std::unexpected{*account.failure};
        }

        // Upstream metrics WEBHOOK_INCOMING_COUNT labelled by merchant
        // (incoming.rs:194-200). LockAction::NotApplicable: no lock is acquired.
        m_support.record_flow("IncomingWebhookReceive", "WEBHOOK_INCOMING_COUNT");
        (void)m_context.locks().is_held(
            "relay:webhook:" + input.merchant_id + ":" + input.connector_id
        );

        // Upstream: IncomingWebhookVerifier::verify plus the mandatory-source
        // policy (incoming.rs:234-263, :558-584). A failed signature is rejected
        // before business logic; with no verifier configured the local connector
        // registry/verifier determines the source_verified flag.
        const auto source = m_support.verify_webhook_source(
            input.signature,
            input.body,
            input.merchant_id,
            input.connector_id,
            account.connector_name
        );
        if (source.failure) {
            return std::unexpected{*source.failure};
        }
        if (!source.verified) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::FORBIDDEN,
                "Webhook authentication failed"
            )};
        }

        // Upstream: connector gateway decodes the payload into an
        // IncomingWebhookEvent + ObjectReferenceId (incoming.rs:428-443).
        const auto decoded = m_support.decode_relay_webhook(input.body);
        const WebhookIds ids{input.merchant_id, input.connector_id};
        if (decoded.setup) {
            // incoming.rs:363-372: setup webhooks are acked with StatusOk/NoEffect.
            m_support.publish_relay_webhook_event(ids, decoded, "NoEffect", "", source, {});
            return RelayWebhookResult{};
        }
        if (decoded.event == RelayWebhookEvent::Unsupported) {
            // incoming.rs:1810-1822: only the Refund flow is supported for relay;
            // incoming.rs:616-635: unsupported events are still acknowledged.
            m_support.publish_relay_webhook_event(ids, decoded, "NoEffect", "", source, {});
            return RelayWebhookResult{};
        }
        if (decoded.object_reference_id.empty()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Webhook processing failure"
            )};
        }

        // Upstream: find_relay_by_id / find_relay_by_profile_id_connector_reference_id
        // with WebhookResourceNotFound on a miss (incoming.rs:1560-1585).
        auto relay = m_support.find_relay_for_webhook(decoded, account.profile_id);
        if (!relay) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Webhook resource not found"
            )};
        }

        std::string status = relay->get_status();
        if (source.verified) {
            // Upstream: RelayUpdate::StatusUpdate with
            // RelayStatus::foreign_try_from(event_type) (incoming.rs:1588-1605;
            // types/transformers.rs:506-519) followed by db.update_relay.
            const auto mapped = m_support.relay_status_for_event(decoded.event);
            if (!mapped) {
                return std::unexpected{m_support.make_error(
                    core::error::AppErrorKind::VALIDATION_FAILED,
                    "Webhook processing failure"
                )};
            }
            status = *mapped;
            relay->set_status(status);
            (void)m_context.repository().update_relay_record(*relay);
            if (!relay->get_refund_id().empty()) {
                (void)m_context.repository().update_refund_status(relay->get_refund_id(), status);
            }
        } else {
            // Upstream: unverified webhooks trigger relay force-sync
            // (incoming.rs:1605-1628) with force_sync=true.
            m_support.sync_relay_status(*relay, input.merchant_id, account.profile_id, true);
            status = relay->get_status();
        }

        // Upstream: WebhookResponseTracker::Relay { relay_id, status } carried
        // in IncomingWebhookEventMetadata (incoming.rs:101-118, :1630-1633).
        m_support.publish_relay_webhook_event(
            ids,
            decoded,
            "Relay",
            status,
            source,
            relay->get_relay_id()
        );
        return RelayWebhookResult{};
    }

private:
    ::app::context::AppContext& m_context;
    RelayRuntimeSupport m_support;
};

} // namespace app::handlers::relay
