module;
export module app_handlers:relay;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_auth;
import app_store;
import app_connector;
import app_http;
import app_crypto;
import app_metrics;
import app_services;
import app_workflow;
import app_events;
import app_locking;
import app_keystore;

export namespace app::handlers::relay {

class RelayRouteHandlers
{
public:
    void handle_unreferenced_refund(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream route: ApiKeyAuth with a required profile (relay.rs:14-47).
        const auto rule = m_context->auth_rules().rule_for("unreferenced_refund");
        const auto auth = m_context->authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = ::app::dto::unreferenced_refund::UnreferencedRefundRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream: process_relay_unreferenced_refund rejects amount <= 0 (core/relay.rs:46-95).
        const auto amount = parse_amount(parsed->get_amount().get_value());
        if (!amount.has_value() || *amount <= 0) {
            send_error(res, send, error(
                core::error::AppErrorKind::PRECONDITION_FAILED, "Amount must be greater than 0"));
            return;
        }

        // Upstream: min/max payout amount from dimension_state::Dimensions. The local
        // superposition config client supplies the same bounds as persisted config values.
        const auto connector_id = parsed->get_connector_id().get_value();
        const auto bounds = payout_bounds(connector_id);
        if (*amount < bounds.first) {
            send_error(res, send, error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                std::format("Amount {} is below the minimum payout amount {}", *amount, bounds.first)));
            return;
        }
        if (*amount > bounds.second) {
            send_error(res, send, error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                std::format("Amount {} exceeds the maximum payout amount {}", *amount, bounds.second)));
            return;
        }

        auto scope = resolve_scope(req);
        const auto connector_account = m_context->repository().find_merchant_connector_by_string(connector_id);
        if (!connector_account) {
            send_error(res, send, error(
                core::error::AppErrorKind::NOT_FOUND, "merchant connector account not found"));
            return;
        }
        if (scope.profile_id.empty()) {
            scope.profile_id = connector_account->get_profile_id();
        }
        // The v2 branch performs an accounts-store profile lookup and reports ProfileNotFound.
        if (feature_enabled("v2") && !scope.profile_id.empty() &&
            !m_context->repository().find_profile_by_string(scope.profile_id)) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "profile not found"));
            return;
        }
        if (scope.profile_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing profile_id"));
            return;
        }
        if (scope.merchant_id.empty()) {
            scope.merchant_id = connector_account->get_merchant_id();
        }
        const std::string connector_name = connector_account->get_connector_name().empty()
            ? std::string{connector_id}
            : connector_account->get_connector_name();
        if (!connector_supported(connector_name)) {
            send_error(res, send, error(
                core::error::AppErrorKind::CONNECTOR_ERROR, "connector does not support relay"));
            return;
        }

        auto connector = select_connector(connector_id, connector_name);
        if (!connector) {
            send_error(res, send, error(core::error::AppErrorKind::CONNECTOR_ERROR, "no relay connector available"));
            return;
        }

        // Upstream flow: Flow::RelayUnreferencedRefund with LockAction::NotApplicable.
        record_flow("RelayUnreferencedRefund", "relay_unreferenced_refund_total");

        // Upstream: RelayId::generate() and internal_{uuid_v7} resource id (core/relay.rs:1260).
        const auto relay_id = m_context->ids().payment_id("relay_");
        const auto resource_id = parsed->get_connector_resource_id().value_or("internal_" + uuid_v7());
        const auto currency = parsed->get_currency().get_value();

        // Upstream: relay connector base_url + access token acquisition. Local stand-in:
        // read the connector base url from config store and the access token from the key store.
        const auto base_url = m_context->config_store().get_value("relay_connectors", connector_name + ".base_url");
        const auto access_token = m_context->key_store().get("relay_access_token", connector_id);
        const std::string tracker_payload = event_payload(
            relay_id, scope.merchant_id, scope.profile_id, "UnreferencedRefund", "Created");
        const std::int64_t tracker_id = m_context->process_trackers().schedule(
            "relay_unreferenced_refund", tracker_payload, now_string());
        if (base_url.has_value() && access_token.has_value()) {
            // Credentials are deliberately kept out of the persisted payload and all logs.
            (void)m_context->metrics().counter("relay_connector_credential_resolved").increment();
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
        relay.set_payload(std::string{"UnreferencedRefund|"} + body);
        relay.set_created_at(now_string());
        auto stored = m_context->repository().create_relay_record(relay);
        if (!stored) { send_error(res, send, stored.error()); return; }
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
        refund = m_context->repository().save_refund(refund);

        ::app::connector::ConnectorResponse connector_response{};
        {
            ::app::connector::ConnectorRequest connector_request{};
            connector_request.set_operation("unreferenced_refund");
            connector_request.set_merchant_id(connector_id);
            connector_request.set_amount(*amount);
            connector_request.set_currency(currency);
            connector_request.set_resource_id(resource_id);
            connector_request.set_raw_payload(body);
            connector_response = connector->refund(connector_request);
            refund.set_connector(connector_name);
            if (!connector_response.get_status().empty()) {
                relay.set_status(connector_response.get_status());
                refund.set_status(connector_response.get_status());
            }
            (void)m_context->repository().update_relay_record(relay);
            (void)m_context->repository().update_refund(refund);
        }

        if (connector_response.get_retryable()) {
            (void)m_context->process_trackers().retry(tracker_id, ::app::workflow::RetryPolicy{});
        } else {
            (void)m_context->process_trackers().mark_terminal(tracker_id, relay.get_status());
        }
        (void)m_context->events().publish(
            "relay.unreferenced_refund",
            event_payload(relay_id, scope.merchant_id, scope.profile_id, "UnreferencedRefund", relay.get_status()));

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
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_relay(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream route: ApiKeyAuth with connected scope optional (relay.rs:51-81).
        const auto rule = m_context->auth_rules().rule_for("relay");
        const auto auth = m_context->authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        // Upstream RelayedRequestInner<T> is a tagged union; the local DTO folds `data` into an
        // enum while upstream sends an object. Normalize through the generic JSON tree so both
        // shapes decode and the per-variant amount fields remain available for validation.
        auto document = serde::Ser::decode_generic("application/json", body);
        if (!document) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const auto relay_type = field_string(*document, "type");
        if (!relay_type.has_value() || relay_type->empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing relay type"));
            return;
        }
        const auto kind = relay_kind(*relay_type);
        if (kind == RelayKind::Unknown) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "unsupported relay type"));
            return;
        }

        auto scope = resolve_scope(req);
        const auto connector_id_value = field_string(*document, "connector_id");
        const std::string connector_id = connector_id_value.value_or("");
        const auto connector_account = m_context->repository().find_merchant_connector_by_string(connector_id);
        if (!connector_account) {
            send_error(res, send, error(
                core::error::AppErrorKind::NOT_FOUND, "merchant connector account not found"));
            return;
        }
        if (scope.profile_id.empty()) {
            scope.profile_id = connector_account->get_profile_id();
        }
        if (feature_enabled("v2") && !scope.profile_id.empty() &&
            !m_context->repository().find_profile_by_string(scope.profile_id)) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "profile not found"));
            return;
        }
        if (scope.profile_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing profile_id"));
            return;
        }
        if (scope.merchant_id.empty()) {
            scope.merchant_id = connector_account->get_merchant_id();
        }
        const std::string connector_name = connector_account->get_connector_name().empty()
            ? connector_id
            : connector_account->get_connector_name();
        if (!connector_supported(connector_name)) {
            send_error(res, send, error(
                core::error::AppErrorKind::CONNECTOR_ERROR, "connector does not support relay"));
            return;
        }

        // relay_flow_decider per-variant validation (core/relay.rs:46-95, 670-724).
        const auto data = field_value(*document, "data");
        std::int64_t amount{0};
        if (data.has_value()) {
            switch (kind) {
                case RelayKind::Refund: {
                    const auto parsed_amount = field_number(*data, "amount");
                    if (!parsed_amount.has_value() || *parsed_amount <= 0) {
                        send_error(res, send, error(
                            core::error::AppErrorKind::PRECONDITION_FAILED,
                            "Relay refund amount must be greater than 0"));
                        return;
                    }
                    amount = *parsed_amount;
                    break;
                }
                case RelayKind::Capture: {
                    const auto authorized = field_number(*data, "authorized_amount");
                    const auto to_capture = field_number(*data, "amount_to_capture");
                    if (!to_capture.has_value() || *to_capture <= 0) {
                        send_error(res, send, error(
                            core::error::AppErrorKind::PRECONDITION_FAILED,
                            "Amount to capture must be greater than 0"));
                        return;
                    }
                    if (authorized.has_value() && *to_capture > *authorized) {
                        send_error(res, send, error(
                            core::error::AppErrorKind::PRECONDITION_FAILED,
                            "Capture amount cannot exceed the authorized amount"));
                        return;
                    }
                    amount = *to_capture;
                    break;
                }
                case RelayKind::IncrementalAuthorization: {
                    const auto additional = field_number(*data, "additional_amount");
                    const auto total = field_number(*data, "total_amount");
                    if (!additional.has_value() || *additional <= 0) {
                        send_error(res, send, error(
                            core::error::AppErrorKind::PRECONDITION_FAILED,
                            "Additional amount must be greater than 0"));
                        return;
                    }
                    if (total.has_value() && *additional > *total) {
                        send_error(res, send, error(
                            core::error::AppErrorKind::PRECONDITION_FAILED,
                            "Additional amount cannot exceed the total amount"));
                        return;
                    }
                    amount = *additional;
                    break;
                }
                default:
                    break;
            }
        }

        auto connector = select_connector(connector_id, connector_name);
        if (!connector) {
            send_error(res, send, error(core::error::AppErrorKind::CONNECTOR_ERROR, "no relay connector available"));
            return;
        }

        // Upstream flow: Flow::Relay (server_wrap, LockAction::NotApplicable).
        record_flow("Relay", "relay_create_total");

        const auto relay_id = m_context->ids().payment_id("relay_");
        const auto specified_resource = field_string(*document, "connector_resource_id");
        const auto resource_id = specified_resource.has_value() && !specified_resource->empty()
            ? *specified_resource
            : "internal_" + uuid_v7();
        const auto currency = data.has_value() ? field_string(*data, "currency").value_or("") : std::string{};

        ::app::store::relay_records::RelayRecord relay{};
        relay.set_relay_id(relay_id);
        relay.set_merchant_id(scope.merchant_id);
        relay.set_profile_id(scope.profile_id);
        relay.set_payment_id(resource_id);
        relay.set_status("Created");
        relay.set_payload(canonical_kind(kind) + "|" + body);
        relay.set_created_at(now_string());
        auto stored = m_context->repository().create_relay_record(relay);
        if (!stored) { send_error(res, send, stored.error()); return; }
        relay = std::move(*stored);

        ::app::store::refunds::RefundRecord refund{};
        refund.set_refund_id(relay_id);
        refund.set_payment_id(resource_id);
        refund.set_merchant_id(connector_id);
        refund.set_amount(amount);
        refund.set_currency(currency);
        refund.set_reason(*relay_type);
        refund.set_status("pending");
        refund = m_context->repository().save_refund(refund);

        ::app::connector::ConnectorResponse connector_response{};
        {
            ::app::connector::ConnectorRequest connector_request{};
            connector_request.set_operation(canonical_kind(kind));
            connector_request.set_merchant_id(connector_id);
            connector_request.set_amount(amount);
            connector_request.set_currency(currency);
            connector_request.set_resource_id(resource_id);
            connector_request.set_raw_payload(body);
            connector_response = dispatch(kind, *connector, connector_request);
            refund.set_connector(connector_name);
            if (!connector_response.get_status().empty()) {
                relay.set_status(connector_response.get_status());
                refund.set_status(connector_response.get_status());
            }
            (void)m_context->repository().update_relay_record(relay);
            (void)m_context->repository().update_refund(refund);
        }

        (void)m_context->events().publish(
            "relay.created",
            event_payload(relay_id, scope.merchant_id, scope.profile_id, *relay_type, relay.get_status()));
        if (feature_enabled("oltp")) {
            (void)m_context->events().publish(
                "relay.analytics",
                event_payload(relay_id, scope.merchant_id, scope.profile_id, *relay_type, relay.get_status()));
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
            relay_error.set_code(connector_response.get_status().empty()
                ? std::string{"CONNECTOR_ERROR"} : connector_response.get_status());
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
        response_type.set_value(canonical_kind(kind));
        response.set_relay_type(std::move(response_type));
        response.set_data(kind_data(kind));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_relay_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream route: ApiKeyAuth with connected scope (relay.rs:85-119).
        const auto rule = m_context->auth_rules().rule_for("relay_retrieve");
        const auto auth = m_context->authorizer().authorize_route(::app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        auto relay_id = relay_id_from_path(req);
        if (!relay_id.has_value() || relay_id->empty()) {
            using Input = ::app::dto::relay::RelayRetrieveRequest;
            auto parsed = serde::Ser::deserialize<Input>("application/json", req.body_as_string());
            if (parsed && !parsed->get_id().get_value().empty()) {
                relay_id = parsed->get_id().get_value();
            }
        }
        if (!relay_id.has_value() || relay_id->empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing relay id in path"));
            return;
        }
        const auto force_sync = force_sync_from_query(req);
        const auto scope = resolve_scope(req);

        // Upstream flow: Flow::RelayRetrieve (server_wrap, LockAction::NotApplicable).
        record_flow("RelayRetrieve", "relay_retrieve_total");

        // Upstream find_relay_by_id; a missing record is a GenericNotFoundError (404).
        std::optional<::app::store::relay_records::RelayRecord> selected{};
        std::int64_t numeric_id{};
        const auto [end, ec] = std::from_chars(relay_id->data(), relay_id->data() + relay_id->size(), numeric_id);
        (void)end;
        if (ec == std::errc{}) {
            selected = m_context->repository().find_relay_record(numeric_id);
        }
        if (!selected) {
            selected = m_context->repository().find_relay_record_by_string(*relay_id);
        }
        // Legacy mirrors written before the relay table existed remain readable.
        std::optional<::app::store::refunds::RefundRecord> refund{};
        if (!selected) {
            if (ec == std::errc{}) {
                refund = m_context->repository().find_refund(numeric_id);
            }
            if (!refund) {
                for (const auto& candidate : m_context->repository().list_refunds()) {
                    if (candidate.get_refund_id() == *relay_id) {
                        refund = candidate;
                        break;
                    }
                }
            }
            if (!refund) {
                send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "relay not found"));
                return;
            }
            ::app::store::relay_records::RelayRecord migrated{};
            migrated.set_relay_id(refund->get_refund_id());
            migrated.set_merchant_id(scope.merchant_id);
            migrated.set_profile_id(scope.profile_id);
            migrated.set_payment_id(refund->get_payment_id());
            migrated.set_status(refund->get_status());
            migrated.set_payload(refund->get_reason());
            migrated.set_created_at(now_string());
            selected = migrated;
        }
        auto record = *selected;

        ::app::connector::ConnectorResponse sync_response{};
        if (force_sync) {
            // Upstream should_call_connector_for_relay_*_status / sync_relay_*_with_gateway.
            // A process tracker records the refresh; a registered connector supplies the
            // authoritative status, otherwise the deterministic stored state is retained.
            const std::string tracker_payload = event_payload(
                *relay_id, scope.merchant_id, scope.profile_id,
                record.get_status().empty() ? "Refund" : "sync", "PENDING");
            const std::int64_t tracker_id = m_context->process_trackers().schedule(
                "relay_status_sync", tracker_payload, now_string());
            auto connector = select_connector(record.get_merchant_id(), std::string{connector_name_hint(record)});
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
                        (void)m_context->repository().update_relay_record(record);
                    }
                    if (refund.has_value()) {
                        refund->set_status(sync_response.get_status());
                        (void)m_context->repository().update_refund(*refund);
                    }
                }
            }
            (void)m_context->process_trackers().mark_terminal(
                tracker_id, sync_response.get_status().empty() ? record.get_status() : sync_response.get_status());
            (void)m_context->events().publish(
                "relay.synced",
                event_payload(*relay_id, scope.merchant_id, scope.profile_id, "sync", record.get_status()));
        }

        using Response = ::app::dto::relay::RelayResponse;
        Response response{};
        ::app::dto::relay::RelayIdValue relay_id_value{};
        relay_id_value.set_value(record.get_relay_id().empty() ? *relay_id : record.get_relay_id());
        response.set_id(std::move(relay_id_value));
        ::app::dto::relay::RelayStatusValue relay_status{};
        relay_status.set_value(record.get_status());
        response.set_status(std::move(relay_status));
        response.set_connector_resource_id(record.get_payment_id());
        ::app::dto::relay::MerchantConnectorAccountIdValue response_connector_id{};
        response_connector_id.set_value(record.get_merchant_id().empty() ? *relay_id : record.get_merchant_id());
        response.set_connector_id(std::move(response_connector_id));
        ::app::dto::relay::ProfileIdValue profile_id{};
        profile_id.set_value(record.get_profile_id().empty() ? scope.profile_id : record.get_profile_id());
        response.set_profile_id(std::move(profile_id));
        const auto kind = relay_kind(stored_kind(record.get_payload()));
        ::app::dto::relay::RelayTypeValue relay_type{};
        relay_type.set_value(kind == RelayKind::Unknown ? std::string{"Refund"} : canonical_kind(kind));
        response.set_relay_type(std::move(relay_type));
        response.set_data(kind_data(kind == RelayKind::Unknown ? RelayKind::Refund : kind));
        if (!sync_response.get_message().empty()) {
            ::app::dto::relay::RelayError relay_error{};
            relay_error.set_code(sync_response.get_status());
            relay_error.set_message(sync_response.get_message());
            response.set_error(std::move(relay_error));
        }
        if (!sync_response.get_reference_id().empty()) {
            response.set_connector_reference_id(sync_response.get_reference_id());
        }
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/relay"}
            .add_route(
                Route{"/"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_relay(req, res, std::move(send));
                    })
                    .summary("Hyperswitch relay")
            )
            .add_route(
                Route{"/unreferenced_refund"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_unreferenced_refund(req, res, std::move(send));
                    })
                    .summary("Hyperswitch unreferenced_refund")
            )
            .add_route(
                Route{"/:relay_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_relay_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch relay_retrieve")
            );
    }

    explicit RelayRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    /** @brief Local mirror of upstream RelayType variants (core/relay.rs:132-149). */
    enum class RelayKind
    {
        Refund,
        Capture,
        IncrementalAuthorization,
        Void,
        UnreferencedRefund,
        Unknown
    };

    /** @brief Identity scope carried by the authenticated headers on relay routes. */
    struct RelayScope
    {
        std::string merchant_id{};
        std::string org_id{};
        std::string profile_id{};
    };

    [[nodiscard]] RelayKind relay_kind(std::string_view value) const
    {
        if (value == "Refund" || value == "refund") { return RelayKind::Refund; }
        if (value == "Capture" || value == "capture") { return RelayKind::Capture; }
        if (value == "IncrementalAuthorization" || value == "incremental_authorization" ||
            value == "incrementalAuthorization") { return RelayKind::IncrementalAuthorization; }
        if (value == "Void" || value == "void") { return RelayKind::Void; }
        if (value == "UnreferencedRefund" || value == "unreferenced_refund") {
            return RelayKind::UnreferencedRefund;
        }
        return RelayKind::Unknown;
    }

    /** @brief Reads the canonical variant prefix stored ahead of the raw payload. */
    [[nodiscard]] std::string_view stored_kind(std::string_view payload) const
    {
        const auto separator = payload.find('|');
        return separator == std::string_view::npos ? payload : payload.substr(0, separator);
    }

    [[nodiscard]] std::string canonical_kind(RelayKind kind) const
    {
        switch (kind) {
            case RelayKind::Refund: return "Refund";
            case RelayKind::Capture: return "Capture";
            case RelayKind::IncrementalAuthorization: return "IncrementalAuthorization";
            case RelayKind::Void: return "Void";
            case RelayKind::UnreferencedRefund: return "UnreferencedRefund";
            default: return "Unknown";
        }
    }

    [[nodiscard]] std::optional<::app::dto::relay::RelayData> kind_data(RelayKind kind) const
    {
        switch (kind) {
            case RelayKind::Refund: return ::app::dto::relay::RelayData::Refund;
            case RelayKind::Capture: return ::app::dto::relay::RelayData::Capture;
            case RelayKind::IncrementalAuthorization: return ::app::dto::relay::RelayData::IncrementalAuthorization;
            case RelayKind::Void: return ::app::dto::relay::RelayData::Void;
            case RelayKind::UnreferencedRefund: return ::app::dto::relay::RelayData::UnreferencedRefund;
            default: return std::nullopt;
        }
    }

    [[nodiscard]] std::optional<serde::Value> field_value(
        const serde::Value& value,
        std::string_view name
    ) const
    {
        const auto* object = std::get_if<serde::Value::Object>(&value.get());
        if (object == nullptr) { return std::nullopt; }
        const auto field = object->get(std::string{name});
        if (!field) { return std::nullopt; }
        return *field;
    }

    [[nodiscard]] std::optional<std::string> field_string(
        const serde::Value& value,
        std::string_view name
    ) const
    {
        const auto raw = field_value(value, name);
        if (!raw) { return std::nullopt; }
        if (const auto* text = std::get_if<std::string>(&raw->get())) { return *text; }
        if (const auto* object = std::get_if<serde::Value::Object>(&raw->get())) {
            const auto inner = object->get(std::string{"value"});
            if (inner) {
                if (const auto* text = std::get_if<std::string>(&inner.value().get())) { return *text; }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::int64_t> field_number(
        const serde::Value& value,
        std::string_view name
    ) const
    {
        const auto raw = field_value(value, name);
        if (!raw) { return std::nullopt; }
        if (const auto* text = std::get_if<std::string>(&raw->get())) { return parse_amount(*text); }
        if (const auto* number = std::get_if<std::int64_t>(&raw->get())) { return *number; }
        if (const auto* number = std::get_if<double>(&raw->get())) {
            return static_cast<std::int64_t>(*number);
        }
        if (const auto* object = std::get_if<serde::Value::Object>(&raw->get())) {
            const auto inner = object->get(std::string{"value"});
            if (inner) {
                if (const auto* text = std::get_if<std::string>(&inner.value().get())) {
                    return parse_amount(*text);
                }
                if (const auto* number = std::get_if<std::int64_t>(&inner.value().get())) {
                    return *number;
                }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::int64_t> parse_amount(std::string_view text) const
    {
        std::int64_t value{};
        const auto [end, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
        if (ec != std::errc{} || end != text.data() + text.size()) { return std::nullopt; }
        return value;
    }

    [[nodiscard]] std::string now_string() const
    {
        const std::int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return std::to_string(now);
    }

    /** @brief Local v7-shaped UUID substitute: time-ordered prefix plus IdGenerator entropy. */
    [[nodiscard]] std::string uuid_v7() const
    {
        std::string id = m_context->ids().uuid_v4();
        const auto millis = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
        constexpr std::string_view digits{"0123456789abcdef"};
        std::uint64_t value = millis;
        for (int index = 11; index >= 0; --index) {
            const std::size_t position = index < 8
                ? static_cast<std::size_t>(index)
                : static_cast<std::size_t>(index + 1);
            id[position] = digits[static_cast<std::size_t>(value & 0xFULL)];
            value >>= 4;
        }
        id[14] = '7';
        return id;
    }

    [[nodiscard]] bool feature_enabled(std::string_view flag) const
    {
        const auto value = m_context->config_store().get_value("relay_features", flag);
        return value.has_value() && (*value == "true" || *value == "1");
    }

    /** @brief Local stand-in for RelayConnectors::from_connector_name support checks. */
    [[nodiscard]] bool connector_supported(std::string_view connector_name) const
    {
        const auto value = m_context->config_store().get_value("relay_connectors", connector_name);
        return !value.has_value() || (*value != "unsupported" && *value != "false");
    }

    /** @brief Local stand-in for dimension_state::Dimensions min/max payout limits. */
    [[nodiscard]] std::pair<std::int64_t, std::int64_t> payout_bounds(std::string_view connector_id) const
    {
        std::int64_t minimum{0};
        std::int64_t maximum{std::numeric_limits<std::int64_t>::max()};
        const auto low = m_context->superposition().resolve("relay_payout_limits", "min_amount", connector_id);
        if (low) {
            if (const auto parsed = parse_amount(low->get_value())) { minimum = *parsed; }
        }
        const auto high = m_context->superposition().resolve("relay_payout_limits", "max_amount", connector_id);
        if (high) {
            if (const auto parsed = parse_amount(high->get_value())) { maximum = *parsed; }
        }
        return {minimum, maximum};
    }

    [[nodiscard]] std::string event_payload(
        std::string_view relay_id,
        std::string_view merchant_id,
        std::string_view profile_id,
        std::string_view kind,
        std::string_view status
    ) const
    {
        serde::Value::Object fields{};
        fields["relay_id"] = serde::Value{std::string{relay_id}};
        fields["merchant_id"] = serde::Value{std::string{merchant_id}};
        fields["profile_id"] = serde::Value{std::string{profile_id}};
        fields["relay_type"] = serde::Value{std::string{kind}};
        fields["status"] = serde::Value{std::string{status}};
        return serde::Ser::encode_json(serde::Value{std::move(fields)});
    }

    void record_flow(std::string_view flow, std::string_view counter) const
    {
        // Upstream server_wrap declares LockAction::NotApplicable for the relay scope;
        // no lock is acquired. The registry is consulted so a stale relay lock is observable.
        (void)m_context->locks().is_held(std::string{"relay:"} + std::string{flow});
        m_context->metrics().record_flow(flow);
        m_context->metrics().counter(counter).increment();
    }

    [[nodiscard]] std::shared_ptr<::app::connector::IConnector> select_connector(
        std::string_view connector_id,
        std::string_view connector_name
    ) const
    {
        auto connector = m_context->connector_router().select(connector_name);
        if (!connector && !connector_id.empty()) {
            connector = m_context->connector_router().select(connector_id);
        }
        return connector;
    }

    [[nodiscard]] ::app::connector::ConnectorResponse dispatch(
        RelayKind kind,
        ::app::connector::IConnector& connector,
        const ::app::connector::ConnectorRequest& request
    ) const
    {
        switch (kind) {
            case RelayKind::Refund:
            case RelayKind::UnreferencedRefund:
                return connector.refund(request);
            case RelayKind::Capture:
                return connector.capture(request);
            case RelayKind::Void:
                return connector.cancel(request);
            case RelayKind::IncrementalAuthorization:
                // IConnector has no incremental-authorization method; authorize is the
                // deterministic stand-in for the upstream incremental connector call.
                return connector.authorize(request);
            default:
                return {};
        }
    }

    [[nodiscard]] std::string_view connector_name_hint(
        const ::app::store::relay_records::RelayRecord& record
    ) const
    {
        // The relay table has no connector-name column; the merchant id carries the
        // connector account id, which the router accepts for deterministic selection.
        return record.get_merchant_id();
    }

    [[nodiscard]] RelayScope resolve_scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        const auto header_merchant_id = ::app::http::merchant_id_from_header{}(req);
        const auto header_org_id = ::app::http::org_id_from_header{}(req);
        RelayScope scope{};
        scope.merchant_id = header_merchant_id.empty() ? auth.merchant_id() : header_merchant_id;
        scope.org_id = header_org_id.empty() ? auth.org_id() : header_org_id;
        scope.profile_id = auth.profile_id();
        return scope;
    }

    /** @brief Extract the `{relay_id}` segment from "GET /relay/{relay_id}". */
    [[nodiscard]] std::optional<std::string> relay_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i{0}; i < segments.count(); ++i) {
            if (segments.at(i).value_or(std::string_view{}) != "relay") {
                continue;
            }
            for (std::size_t j{i + 1}; j < segments.count(); ++j) {
                const auto candidate = segments.at(j).value_or(std::string_view{});
                if (!candidate.empty()) {
                    return std::string{candidate};
                }
            }
        }
        if (segments.count() > 0) {
            return std::string{segments.at(segments.count() - 1).value_or(std::string_view{})};
        }
        return std::nullopt;
    }

    [[nodiscard]] bool force_sync_from_query(interfaces::io::IRequest& req) const
    {
        const auto value = ::app::http::RequestQuery{}(req).get("force_sync");
        return value.has_value() && (*value == "true" || *value == "1");
    }

    [[nodiscard]] core::error::AppError error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error_value
    ) const
    {
        auto text = serde::Ser::encode_json(error_value);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error_value.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    ::app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::relay
