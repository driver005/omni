export module app_payment:relay_runtime_relay_retrieve;

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

class RelayRetrieveRuntime
{
public:
    explicit RelayRetrieveRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream flow: Flow::RelayRetrieve (server_wrap, LockAction::NotApplicable).
    [[nodiscard]] std::expected<RelayRetrieveResult, core::error::AppError>
    retrieve(const RelayRetrieveInput& input) const
    {
        m_support.record_flow("RelayRetrieve", "relay_retrieve_total");

        const auto& relay_id = input.relay_id;
        const auto& scope = input.scope;

        // Upstream find_relay_by_id; a missing record is a GenericNotFoundError (404).
        std::optional<::app::store::relay_records::RelayRecord> selected{};
        std::int64_t numeric_id{};
        const auto [end, ec] =
            std::from_chars(relay_id.data(), relay_id.data() + relay_id.size(), numeric_id);
        (void)end;
        if (ec == std::errc{}) {
            selected = m_context.repository().find_relay_record(numeric_id);
        }
        if (!selected) {
            selected = m_context.repository().find_relay_record_by_string(relay_id);
        }
        // Legacy mirrors written before the relay table existed remain readable.
        std::optional<::app::store::refunds::RefundRecord> refund{};
        if (!selected) {
            if (ec == std::errc{}) {
                refund = m_context.repository().find_refund(numeric_id);
            }
            if (!refund) {
                for (const auto& candidate: m_context.repository().list_refunds()) {
                    if (candidate.get_refund_id() == relay_id) {
                        refund = candidate;
                        break;
                    }
                }
            }
            if (!refund) {
                return std::unexpected{
                    m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "relay not found")
                };
            }
            ::app::store::relay_records::RelayRecord migrated{};
            migrated.set_relay_id(refund->get_refund_id());
            migrated.set_merchant_id(scope.merchant_id);
            migrated.set_profile_id(scope.profile_id);
            migrated.set_payment_id(refund->get_payment_id());
            migrated.set_status(refund->get_status());
            migrated.set_payload(refund->get_reason());
            migrated.set_created_at(m_support.now_string());
            selected = migrated;
        }
        auto record = *selected;

        ::app::connector::ConnectorResponse sync_response{};
        // Upstream should_call_connector_for_relay_refund_status /
        // should_call_connector_for_relay_capture_status (core/relay.rs:947-957):
        // only non-terminal states are eligible for a forced gateway refresh.
        if (input.force_sync && !m_support.is_terminal_relay_status(record.get_status())) {
            // A process tracker records the refresh; a registered connector supplies the
            // authoritative status, otherwise the deterministic stored state is retained.
            const std::string tracker_payload = m_support.event_payload(
                relay_id,
                scope.merchant_id,
                scope.profile_id,
                record.get_status().empty() ? "Refund" : "sync",
                "PENDING"
            );
            const std::int64_t tracker_id = m_context.process_trackers().schedule(
                "relay_status_sync",
                tracker_payload,
                m_support.now_string()
            );
            auto connector = m_support.select_connector(
                record.get_merchant_id(),
                std::string{m_support.connector_name_hint(record)}
            );
            if (connector) {
                ::app::connector::ConnectorRequest connector_request{};
                connector_request.set_operation("sync");
                connector_request.set_merchant_id(record.get_merchant_id());
                connector_request.set_resource_id(record.get_payment_id());
                connector_request.set_payment_method_type(record.get_status());
                sync_response = connector->sync(connector_request);
                if (!sync_response.get_status().empty()) {
                    record.set_status(sync_response.get_status());
                    if (record.get_id() != 0) {
                        (void)m_context.repository().update_relay_record(record);
                    }
                    if (refund.has_value()) {
                        refund->set_status(sync_response.get_status());
                        (void)m_context.repository().update_refund(*refund);
                    }
                }
            }
            (void)m_context.process_trackers().mark_terminal(
                tracker_id,
                sync_response.get_status().empty() ? record.get_status()
                                                   : sync_response.get_status()
            );
            (void)m_context.events().publish(
                "relay.synced",
                m_support.event_payload(
                    relay_id,
                    scope.merchant_id,
                    scope.profile_id,
                    "sync",
                    record.get_status()
                )
            );
        }

        using Response = ::app::dto::relay::RelayResponse;
        Response response{};
        ::app::dto::relay::RelayIdValue relay_id_value{};
        relay_id_value.set_value(record.get_relay_id().empty() ? relay_id : record.get_relay_id());
        response.set_id(std::move(relay_id_value));
        ::app::dto::relay::RelayStatusValue relay_status{};
        relay_status.set_value(record.get_status());
        response.set_status(std::move(relay_status));
        response.set_connector_resource_id(record.get_payment_id());
        ::app::dto::relay::MerchantConnectorAccountIdValue response_connector_id{};
        response_connector_id.set_value(
            record.get_merchant_id().empty() ? relay_id : record.get_merchant_id()
        );
        response.set_connector_id(std::move(response_connector_id));
        ::app::dto::relay::ProfileIdValue profile_id{};
        profile_id.set_value(
            record.get_profile_id().empty() ? scope.profile_id : record.get_profile_id()
        );
        response.set_profile_id(std::move(profile_id));
        const auto kind = m_support.relay_kind(m_support.stored_kind(record.get_payload()));
        ::app::dto::relay::RelayTypeValue relay_type{};
        relay_type.set_value(
            std::string{
                m_support.relay_type_wire(kind == RelayKind::Unknown ? RelayKind::Refund : kind)
            }
        );
        response.set_relay_type(std::move(relay_type));
        response.set_data(m_support.reconstruct_relay_data(kind, record.get_payload())
                              .value_or(::app::dto::relay::RelayResponseData{}));
        if (!sync_response.get_message().empty()) {
            ::app::dto::relay::RelayError relay_error{};
            relay_error.set_code(sync_response.get_status());
            relay_error.set_message(sync_response.get_message());
            response.set_error(std::move(relay_error));
        }
        if (!sync_response.get_reference_id().empty()) {
            response.set_connector_reference_id(sync_response.get_reference_id());
        }
        return RelayRetrieveResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    RelayRuntimeSupport m_support;
};

} // namespace app::handlers::relay
