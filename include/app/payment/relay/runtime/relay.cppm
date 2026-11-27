module;
#include <uuid.h>

export module app_payment:relay_runtime_relay;
import core_behavior;

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

class RelayCreateRuntime
{
public:
    explicit RelayCreateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream RelayedRequestInner<T> is a tagged union; the local DTO folds `data` into an
    // enum while upstream sends an object. Normalize through the generic JSON tree so both
    // shapes decode and the per-variant amount fields remain available for validation.
    [[nodiscard]] core::behavior::Outcome<::app::dto::relay::RelayResponse>
    relay(const core::behavior::CallContext& ctx) const
    {
        const auto input = RelayCreateData{ctx};
        auto document = serde::Ser::decode_generic("application/json", input.body);
        if (!document) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")
            };
        }
        const auto relay_type = m_support.field_string(*document, "type");
        if (!relay_type.has_value() || relay_type->empty()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing relay type"
            )};
        }
        const auto kind = m_support.relay_kind(*relay_type);
        if (kind == RelayKind::Unknown) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "unsupported relay type"
            )};
        }

        auto scope = input.scope;
        const auto connector_id_value = m_support.field_string(*document, "connector_id");
        const std::string connector_id = connector_id_value.value_or("");
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
                                               ? connector_id
                                               : connector_account->get_connector_name();
        if (!m_support.connector_supported(connector_name)) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::CONNECTOR_ERROR,
                std::format("Connector {} does not support relay", connector_name)
            )};
        }

        // relay_flow_decider per-variant dispatch and validation
        // (core/relay.rs:46-95, 132-149, 670-724): RelayRequestInner::from_relay_request
        // requires the data variant to match the relay type, then Validate checks the
        // variant-specific amount constraints with exact upstream messages.
        const auto data = m_support.field_value(*document, "data");
        const auto variant = data.has_value()
                                 ? m_support.field_value(*data, m_support.relay_data_key(kind))
                                 : std::nullopt;
        if (!variant.has_value()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                std::format(
                    "Relay data is required for relay type {}",
                    m_support.relay_type_wire(kind)
                )
            )};
        }
        const auto& relay_data = *variant;
        std::int64_t amount{0};
        switch (kind) {
            case RelayKind::Refund:
                {
                    const auto parsed_amount = m_support.field_number(relay_data, "amount");
                    if (!parsed_amount.has_value() || *parsed_amount <= 0) {
                        return std::unexpected{m_support.make_error(
                            core::error::AppErrorKind::PRECONDITION_FAILED,
                            "Amount should be greater than 0"
                        )};
                    }
                    amount = *parsed_amount;
                    break;
                }
            case RelayKind::Capture:
                {
                    const auto authorized = m_support.field_number(relay_data, "authorized_amount");
                    const auto to_capture = m_support.field_number(relay_data, "amount_to_capture");
                    if (!to_capture.has_value() || *to_capture <= 0) {
                        return std::unexpected{m_support.make_error(
                            core::error::AppErrorKind::PRECONDITION_FAILED,
                            "Amount should be greater than 0"
                        )};
                    }
                    if (authorized.has_value() && *to_capture > *authorized) {
                        return std::unexpected{m_support.make_error(
                            core::error::AppErrorKind::PRECONDITION_FAILED,
                            "Capture Amount should be less than or equal to Authorized Amount"
                        )};
                    }
                    amount = *to_capture;
                    break;
                }
            case RelayKind::IncrementalAuthorization:
                {
                    const auto additional = m_support.field_number(relay_data, "additional_amount");
                    if (!additional.has_value() || *additional <= 0) {
                        return std::unexpected{m_support.make_error(
                            core::error::AppErrorKind::PRECONDITION_FAILED,
                            "Amount should be greater than 0"
                        )};
                    }
                    amount = *additional;
                    break;
                }
            default:
                break;
        }

        auto connector = m_support.select_connector(connector_id, connector_name);
        if (!connector) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::CONNECTOR_ERROR,
                "no relay connector available"
            )};
        }

        // Upstream flow: Flow::Relay (server_wrap, LockAction::NotApplicable).
        m_support.record_flow("Relay", "relay_create_total");

        const auto relay_id = m_context.ids().payment_id("relay_");
        const auto specified_resource = m_support.field_string(*document, "connector_resource_id");
        const auto resource_id = specified_resource.has_value() && !specified_resource->empty()
                                     ? *specified_resource
                                     : "internal_" + m_support.uuid_v7();
        const auto currency = m_support.field_string(relay_data, "currency").value_or("");

        ::app::store::relay_records::RelayRecord relay{};
        relay.set_relay_id(relay_id);
        relay.set_merchant_id(scope.merchant_id);
        relay.set_profile_id(scope.profile_id);
        relay.set_payment_id(resource_id);
        relay.set_status("Created");
        relay.set_payload(m_support.canonical_kind(kind) + "|" + input.body);
        relay.set_created_at(m_support.now_string());
        auto stored = m_context.repository().create_relay_record(relay);
        if (!stored) {
            return std::unexpected{stored.error()};
        }
        relay = std::move(*stored);

        ::app::model::Refund refund{};
        refund.id = uuids::uuid::from_string(m_context.ids().uuid_v4()).value_or(uuids::uuid{});
        refund.refund_id = relay_id;
        refund.payment_id = uuids::uuid::from_string(resource_id).value_or(uuids::uuid{});
        refund.merchant_id = connector_id;
        refund.amount = amount;
        refund.currency = currency;
        refund.reason = *relay_type;
        refund.status = "pending";
        refund = m_context.refunds().save_refund(refund);

        ::app::connector::ConnectorResponse connector_response{};
        {
            ::app::connector::ConnectorRequest connector_request{};
            connector_request.set_operation(m_support.canonical_kind(kind));
            connector_request.set_merchant_id(connector_id);
            connector_request.set_amount(amount);
            connector_request.set_currency(currency);
            connector_request.set_resource_id(resource_id);
            connector_request.set_raw_payload(input.body);
            connector_response = m_support.dispatch(kind, *connector, connector_request);
            // Upstream RelayUpdate::from_refund_response carries connector_reference_id;
            // mirror it into the relay record so relay webhooks can locate the relay by
            // connector refund id (core/webhooks/incoming.rs:1574-1582).
            if (!connector_response.get_reference_id().empty()) {
                relay.set_refund_id(connector_response.get_reference_id());
            }
            refund.connector = connector_name;
            if (!connector_response.get_status().empty()) {
                relay.set_status(connector_response.get_status());
                refund.status = connector_response.get_status();
            }
            (void)m_context.repository().update_relay_record(relay);
            (void)m_context.refunds().update_refund(refund);
        }

        (void)m_context.events().publish(
            "relay.created",
            m_support.event_payload(
                relay_id,
                scope.merchant_id,
                scope.profile_id,
                *relay_type,
                relay.get_status()
            )
        );
        if (m_support.feature_enabled("oltp")) {
            (void)m_context.events().publish(
                "relay.analytics",
                m_support.event_payload(
                    relay_id,
                    scope.merchant_id,
                    scope.profile_id,
                    *relay_type,
                    relay.get_status()
                )
            );
        }

        using Response = ::app::dto::relay::RelayResponse;
        Response response{};
        ::app::dto::relay::RelayIdValue relay_id_value{};
        relay_id_value.set_value(relay_id);
        response.set_id(std::move(relay_id_value));
        ::app::dto::relay::RelayStatusValue relay_status{};
        relay_status.set_value(relay.get_status());
        response.set_status(std::move(relay_status));
        response.set_connector_resource_id(resource_id);
        if (!connector_response.get_message().empty()) {
            ::app::dto::relay::RelayError relay_error{};
            relay_error.set_code(
                connector_response.get_status().empty() ? std::string{"CONNECTOR_ERROR"}
                                                        : connector_response.get_status()
            );
            relay_error.set_message(connector_response.get_message());
            response.set_error(std::move(relay_error));
        }
        if (!connector_response.get_reference_id().empty()) {
            response.set_connector_reference_id(connector_response.get_reference_id());
        }
        ::app::dto::relay::MerchantConnectorAccountIdValue response_connector_id{};
        response_connector_id.set_value(connector_id);
        response.set_connector_id(std::move(response_connector_id));
        ::app::dto::relay::ProfileIdValue profile_id{};
        profile_id.set_value(scope.profile_id);
        response.set_profile_id(std::move(profile_id));
        ::app::dto::relay::RelayTypeValue response_type{};
        response_type.set_value(std::string{m_support.relay_type_wire(kind)});
        response.set_relay_type(std::move(response_type));
        response.set_data(m_support.tagged_relay_data(kind, relay_data)
                              .value_or(::app::dto::relay::RelayResponseData{}));
        return ::app::dto::relay::RelayResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    RelayRuntimeSupport m_support;
};

} // namespace app::handlers::relay
