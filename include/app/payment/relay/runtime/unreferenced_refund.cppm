export module app_payment:relay_runtime_unreferenced_refund;

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

class RelayUnreferencedRefundRuntime
{
public:
    explicit RelayUnreferencedRefundRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream: process_relay_unreferenced_refund (core/relay.rs:46-95).
    [[nodiscard]] std::
        expected<::app::dto::unreferenced_refund::UnreferencedRefundResponse, core::error::AppError>
        unreferenced_refund(const RelayUnreferencedRefundInput& input) const
    {
        const auto amount = m_support.parse_amount(input.request.get_amount().get_value());
        if (!amount.has_value() || *amount <= 0) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Amount must be greater than 0"
            )};
        }

        // Upstream: min/max payout amount from dimension_state::Dimensions. The local
        // superposition config client supplies the same bounds as persisted config values.
        const auto connector_id = input.request.get_connector_id().get_value();
        const auto bounds = m_support.payout_bounds(connector_id);
        if (*amount < bounds.first) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                std::format(
                    "Amount {} is below the minimum payout amount {}",
                    *amount,
                    bounds.first
                )
            )};
        }
        if (*amount > bounds.second) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                std::format(
                    "Amount {} exceeds the maximum payout amount {}",
                    *amount,
                    bounds.second
                )
            )};
        }

        auto scope = input.scope;
        const auto connector_account =
            m_context.repository().find_merchant_connector_by_string(connector_id);
        if (!connector_account) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "merchant connector account not found"
            )};
        }
        if (scope.profile_id.empty()) {
            scope.profile_id = connector_account->get_profile_id();
        }
        // The v2 branch performs an accounts-store profile lookup and reports ProfileNotFound.
        if (m_support.feature_enabled("v2") && !scope.profile_id.empty() &&
            !m_context.repository().find_profile_by_string(scope.profile_id)) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "profile not found")
            };
        }
        if (scope.profile_id.empty()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing profile_id"
            )};
        }
        if (scope.merchant_id.empty()) {
            scope.merchant_id = connector_account->get_merchant_id();
        }
        const std::string connector_name = connector_account->get_connector_name().empty()
                                               ? std::string{connector_id}
                                               : connector_account->get_connector_name();
        if (!m_support.connector_supported(connector_name)) {
            // Upstream RelayConnectors::from_connector_name -> InvalidRequestData
            // (core/relay.rs:1052-1056).
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                std::format("Connector {} does not support unreferenced refund", connector_name)
            )};
        }

        auto connector = m_support.select_connector(connector_id, connector_name);
        if (!connector) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::CONNECTOR_ERROR,
                "no relay connector available"
            )};
        }

        // Upstream flow: Flow::RelayUnreferencedRefund with LockAction::NotApplicable.
        m_support.record_flow("RelayUnreferencedRefund", "relay_unreferenced_refund_total");

        // Upstream: RelayId::generate() and internal_{uuid_v7} resource id (core/relay.rs:1260).
        const auto relay_id = m_context.ids().payment_id("relay_");
        const auto resource_id =
            input.request.get_connector_resource_id().value_or("internal_" + m_support.uuid_v7());
        const auto currency = input.request.get_currency().get_value();

        // Upstream relay connector base_url + access token acquisition. Local stand-in:
        // read the connector base url from config store and the access token from the key store.
        const auto base_url =
            m_context.config_store().get_value("relay_connectors", connector_name + ".base_url");
        const auto access_token = m_context.key_store().get("relay_access_token", connector_id);
        const std::string tracker_payload = m_support.event_payload(
            relay_id,
            scope.merchant_id,
            scope.profile_id,
            "UnreferencedRefund",
            "Created"
        );
        const std::int64_t tracker_id = m_context.process_trackers().schedule(
            "relay_unreferenced_refund",
            tracker_payload,
            m_support.now_string()
        );
        if (base_url.has_value() && access_token.has_value()) {
            // Credentials are deliberately kept out of the persisted payload and all logs.
            (void)m_context.metrics().counter("relay_connector_credential_resolved").increment();
        }

        // Upstream insert_relay with processor_merchant_id / created_by. The local RelayRecord
        // carries the processor merchant and the caller's profile in dedicated columns.
        ::app::store::relay_records::RelayRecord relay{};
        relay.set_relay_id(relay_id);
        relay.set_merchant_id(scope.merchant_id);
        relay.set_profile_id(scope.profile_id);
        relay.set_payment_id(resource_id);
        relay.set_refund_id(resource_id);
        relay.set_status("Created");
        relay.set_payload(std::string{"UnreferencedRefund|"} + input.body);
        relay.set_created_at(m_support.now_string());
        auto stored = m_context.repository().create_relay_record(relay);
        if (!stored) {
            return std::unexpected{stored.error()};
        }
        relay = std::move(*stored);

        // Refund mirror retained for the existing refund-facing read paths.
        ::app::store::refunds::RefundRecord refund{};
        refund.set_refund_id(relay_id);
        refund.set_payment_id(resource_id);
        refund.set_merchant_id(connector_id);
        refund.set_amount(*amount);
        refund.set_currency(currency);
        refund.set_reason("UnreferencedRefund");
        refund.set_status("pending");
        refund = m_context.repository().save_refund(refund);

        ::app::connector::ConnectorResponse connector_response{};
        {
            ::app::connector::ConnectorRequest connector_request{};
            connector_request.set_operation("unreferenced_refund");
            connector_request.set_merchant_id(connector_id);
            connector_request.set_amount(*amount);
            connector_request.set_currency(currency);
            connector_request.set_resource_id(resource_id);
            connector_request.set_raw_payload(input.body);
            connector_response = connector->refund(connector_request);
            if (!connector_response.get_reference_id().empty()) {
                relay.set_refund_id(connector_response.get_reference_id());
            }
            refund.set_connector(connector_name);
            if (!connector_response.get_status().empty()) {
                relay.set_status(connector_response.get_status());
                refund.set_status(connector_response.get_status());
            }
            (void)m_context.repository().update_relay_record(relay);
            (void)m_context.repository().update_refund(refund);
        }

        if (connector_response.get_retryable()) {
            (void)m_context.process_trackers().retry(tracker_id, ::app::workflow::RetryPolicy{});
        } else {
            (void)m_context.process_trackers().mark_terminal(tracker_id, relay.get_status());
        }
        (void)m_context.events().publish(
            "relay.unreferenced_refund",
            m_support.event_payload(
                relay_id,
                scope.merchant_id,
                scope.profile_id,
                "UnreferencedRefund",
                relay.get_status()
            )
        );

        using Response = ::app::dto::unreferenced_refund::UnreferencedRefundResponse;
        Response response{};
        ::app::dto::unreferenced_refund::RelayIdValue relay_id_value{};
        relay_id_value.set_value(relay_id);
        response.set_id(std::move(relay_id_value));
        ::app::dto::unreferenced_refund::RelayStatusValue relay_status{};
        relay_status.set_value(relay.get_status());
        response.set_status(std::move(relay_status));
        response.set_connector(connector_name);
        ::app::dto::unreferenced_refund::MerchantConnectorAccountIdValue connector_id_value{};
        connector_id_value.set_value(connector_id);
        response.set_connector_id(std::move(connector_id_value));
        response.set_connector_resource_id(resource_id);
        if (!connector_response.get_reference_id().empty()) {
            response.set_connector_reference_id(connector_response.get_reference_id());
        }
        ::app::dto::unreferenced_refund::ProfileIdValue profile_id{};
        profile_id.set_value(scope.profile_id);
        response.set_profile_id(std::move(profile_id));
        if (!connector_response.get_message().empty()) {
            ::app::dto::unreferenced_refund::RelayErrorValue relay_error{};
            relay_error.set_value(connector_response.get_message());
            response.set_error(std::move(relay_error));
        }
        if (!connector_response.get_raw_json().empty()) {
            response.set_raw_connector_response(connector_response.get_raw_json());
        }
        return ::app::dto::unreferenced_refund::UnreferencedRefundResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    RelayRuntimeSupport m_support;
};

} // namespace app::handlers::relay
