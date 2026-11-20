module;
export module app_handlers:payouts;

import std;
import interfaces;
import core_router;
import core_serde;
import core_cache;
import core_error;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_store;
import app_connector;
import app_http;
import app_metrics;
import app_crypto;
import app_db;
import app_events;
import app_keystore;
import app_locking;
import app_services;
import app_workflow;

export namespace app::handlers::payouts {

class PayoutsRouteHandlers
{
public:
    explicit PayoutsRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_payouts_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payouts_create")) {
            send_error(res, send, *denied);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payouts::PayoutCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const auto& input = *parsed;

        m_context->metrics().record_flow("payout_create");
        m_context->metrics().counter("payouts_create_total").increment();

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto header_merchant_id = app::http::merchant_id_from_header{}(req);
        (void)app::http::org_id_from_header{}(req);

        if (const auto invalid = validate_create_input(input, header_merchant_id)) {
            send_error(res, send, *invalid);
            return;
        }

        std::int64_t amount{0};
        if (input.amount) {
            const auto& amount_text = input.amount->get_value();
            std::int64_t decoded{0};
            const auto result = std::from_chars(amount_text.data(), amount_text.data() + amount_text.size(), decoded);
            if (result.ec == std::errc{}) {
                amount = decoded;
            }
        }
        if (input.amount && amount <= 0) {
            send_error(res, send, error(
                core::error::AppErrorKind::PRECONDITION_FAILED, "payout amount must be positive"));
            return;
        }

        std::string merchant_id = input.merchant_id ? input.merchant_id->get_value() : std::string{};
        if (!header_merchant_id.empty()) {
            merchant_id = header_merchant_id;
        } else if (merchant_id.empty()) {
            merchant_id = auth.merchant_id();
        }
        const auto currency = input.currency ? input.currency->get_value() : std::string{};
        std::string payout_id = input.payout_id ? input.payout_id->get_value()
                                                : m_context->ids().payment_id("po_");
        const auto profile_id = resolve_profile_id(req, input.profile_id);
        const auto browser_info = browser_info_from_headers(req, input.browser_info);
        const auto requested_connector = first_requested_connector(input.connector);

        if (m_context->repository().find_payout_by_string(payout_id)) {
            send_error(res, send, error(
                core::error::AppErrorKind::ALREADY_EXISTS, "payout already exists"));
            return;
        }

        std::string payout_link_id{};
        if (input.payout_link.value_or(false) || input.payout_link_config.has_value()) {
            payout_link_id = m_context->ids().payment_id("pl_");
        }

        std::string note{};
        if (input.customer_id && !input.customer_id->get_value().empty()) {
            const auto customer = m_context->repository().find_customer_by_string(
                input.customer_id->get_value());
            note = customer.has_value() ? "customer_resolved" : "customer_unresolved";
        }

        auto connector_request = app::connector::ConnectorRequestBuilder{}
                                     .operation("payout")
                                     .merchant_id(merchant_id)
                                     .amount(amount)
                                     .currency(currency)
                                     .resource_id(payout_id)
                                     .raw_payload(body)();

        // Upstream initial status (`core/payouts.rs:3668-3679`): with method data a payout
        // starts in RequiresCreation (confirm) / RequiresConfirmation; otherwise
        // RequiresPayoutMethodData.
        const bool has_method_data = input.payout_method_data.has_value() ||
                                     (input.payout_token.has_value() && !input.payout_token->empty()) ||
                                     input.payout_method_id.has_value() ||
                                     input.source_bank_data.has_value();
        std::string status = has_method_data
                                 ? std::string{input.confirm.value_or(false) ? kRequiresCreation
                                                                             : kRequiresConfirmation}
                                 : std::string{kRequiresPayoutMethodData};
        std::string connector_name{};
        if (input.payout_token && !input.payout_token->empty()) {
            // External-vault proxy substitute: token payouts park method data in the local
            // key store and keep the upstream pre-confirmation status.
            connector_name = "external_vault";
            note = note.empty() ? std::string{"external_vault_token"} : note + ";external_vault_token";
            store_payout_method_data(payout_id, body);
        } else if (input.confirm.value_or(false)) {
            auto choice = make_connector_decision(
                connector_request, merchant_id, payout_id, amount, currency,
                input.payout_type ? input.payout_type->get_value() : std::string{},
                requested_connector);
            if (choice.error) {
                send_error(res, send, *choice.error);
                return;
            }
            if (choice.connector) {
                const auto outcome = invoke_connector(*choice.connector, "authorize", connector_request);
                if (outcome.error) {
                    send_error(res, send, *outcome.error);
                    return;
                }
                status = status_from_connector(outcome.status, input.auto_fulfill.value_or(false));
                connector_name = std::string{choice.connector->name()};
                if (outcome.shadow_killed) {
                    m_context->metrics().counter("payouts_shadow_killed").increment();
                }
                if (!outcome.reference_id.empty()) {
                    note = note.empty() ? std::string{"connector_ref="} + outcome.reference_id
                                        : note + ";connector_ref=" + outcome.reference_id;
                }
                m_context->process_trackers().schedule(
                    "payout_sync", std::format(R"({{"payout_id":"{}"}})", payout_id), "0");
            } else {
                note = note.empty() ? std::string{"connector_unavailable"}
                                    : note + ";connector_unavailable";
            }
        }

        if (!input.payout_token && (input.payout_method_data.has_value() ||
                                    input.payout_method_id.has_value() ||
                                    input.source_bank_data.has_value())) {
            store_payout_method_data(payout_id, body);
        }

        app::store::payouts::PayoutRecord record{};
        record.set_payout_id(payout_id);
        record.set_merchant_id(merchant_id);
        record.set_amount(amount);
        record.set_currency(currency);
        record.set_status(status);
        record.set_connector(connector_name);
        record.set_created_at(created_at_now());
        if (input.payout_type) {
            record.set_payout_type(input.payout_type->get_value());
        }
        if (input.customer_id) {
            record.set_customer_id(input.customer_id->get_value());
        }
        record.set_metadata(encode_meta(
            profile_id, browser_info, payout_link_id,
            payout_link_id.empty() ? std::string{} : std::string{"Submitted"}, note,
            input.merchant_order_reference_id.value_or(""),
            input.entity_type ? input.entity_type->get_value() : std::string{}));
        record = m_context->repository().save_payout(record);

        record_payout_attempt(
            record.get_payout_id(), record.get_merchant_id(), connector_name,
            record.get_status(), record.get_amount(), record.get_currency());

        m_context->cache().put("payouts.idempotency." + payout_id,
                               core::cache::to_cache_value(merchant_id));
        emit_payout_webhook("created", record);

        auto response = response_from_record(record);
        enrich_response(response, record, false);
        if (input.merchant_order_reference_id) {
            response.set_merchant_order_reference_id(*input.merchant_order_reference_id);
        }
        if (input.description) {
            response.set_description(*input.description);
        }

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payouts_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payouts_retrieve")) {
            send_error(res, send, *denied);
            return;
        }

        const auto path_id = payout_id_from_path(req);
        if (!path_id.has_value()) {
            send_error(res, send, error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing payout id in path"));
            return;
        }
        // Upstream uses `web::Query<PayoutRetrieveBody>` (`payouts.rs:90-97`); all
        // three fields come from the query string, never from a request body.
        const auto query = app::http::RequestQuery{}(req);
        auto retrieve = serde::Ser::deserialize<app::model::payouts::PayoutRetrieveBody>(
            "application/json",
            query_json(query, {"force_sync", "merchant_id", "expand_attempts"}));
        if (!retrieve) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "invalid PayoutRetrieveBody query"));
            return;
        }
        const bool force_sync = retrieve->get_force_sync().value_or(false);
        const bool expand_attempts = retrieve->get_expand_attempts().value_or(false);

        m_context->metrics().record_flow("payout_retrieve");
        m_context->metrics().counter("payouts_retrieve_total").increment();

        auto found = find_payout_record(*path_id);
        if (!found.has_value()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "payout not found"));
            return;
        }
        auto record = *found;

        // Profile scope from the authenticated principal (`payouts_retrieve_core:928-940`).
        const auto scope = resolve_profile_id(req, std::nullopt);
        if (!scope.empty() && meta_value(record.get_metadata(), "profile_id") != scope) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "payout not found"));
            return;
        }

        if (force_sync && should_call_retrieve(record.get_status())) {
            auto sync_request = app::connector::ConnectorRequestBuilder{}
                                    .operation("payout_sync")
                                    .merchant_id(record.get_merchant_id())
                                    .amount(record.get_amount())
                                    .currency(record.get_currency())
                                    .resource_id(record.get_payout_id())();
            if (auto connector = m_context->connector_router().select(record.get_merchant_id())) {
                const auto outcome = invoke_connector(*connector, "sync", sync_request);
                if (outcome.error) {
                    send_error(res, send, *outcome.error);
                    return;
                }
                if (!outcome.status.empty()) {
                    record.set_status(status_from_connector(outcome.status, false));
                }
                m_context->repository().update_payout(record);
                update_latest_attempt(record.get_payout_id(), record.get_status(), {}, {});
                emit_payout_webhook("synced", record);
            } else {
                m_context->metrics().counter("payouts_connector_unavailable").increment();
            }
        }

        auto response = response_from_record(record);
        enrich_response(response, record, expand_attempts);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payouts_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payouts_update")) {
            send_error(res, send, *denied);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payouts::PayoutCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const auto& input = *parsed;

        const auto path_id = payout_id_from_path(req);
        if (!path_id.has_value()) {
            send_error(res, send, error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing payout id in path"));
            return;
        }

        PayoutLockGuard lock{m_context->locks(), std::format("payout:{}", *path_id), "payouts_update"};
        if (!lock.held()) {
            send_error(res, send, error(
                core::error::AppErrorKind::PRECONDITION_FAILED, "payout is locked by another request"));
            return;
        }

        m_context->metrics().record_flow("payout_update");
        m_context->metrics().counter("payouts_update_total").increment();

        auto found = find_payout_record(*path_id);
        if (!found.has_value()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "payout not found"));
            return;
        }
        auto record = *found;

        // Upstream `payouts_update_core:864-872`: terminal or already-initiated payouts
        // cannot be updated (`InvalidRequestData`).
        if (is_payout_terminal_state(record.get_status()) ||
            is_payout_initiated(record.get_status())) {
            send_error(res, send, error(
                core::error::AppErrorKind::INVALID_REQUEST,
                std::format("Payout {} cannot be updated for status {}",
                            record.get_payout_id(), normalize_status(record.get_status()))));
            return;
        }

        const auto header_merchant_id = app::http::merchant_id_from_header{}(req);
        if (!header_merchant_id.empty()) {
            record.set_merchant_id(header_merchant_id);
        } else if (input.merchant_id) {
            record.set_merchant_id(input.merchant_id->get_value());
        }

        std::int64_t amount{record.get_amount()};
        if (input.amount) {
            const auto& amount_text = input.amount->get_value();
            std::int64_t decoded{0};
            const auto result = std::from_chars(amount_text.data(), amount_text.data() + amount_text.size(), decoded);
            if (result.ec == std::errc{}) {
                amount = decoded;
            }
        }
        if (amount <= 0) {
            send_error(res, send, error(
                core::error::AppErrorKind::PRECONDITION_FAILED, "payout amount must be positive"));
            return;
        }
        record.set_amount(amount);

        if (input.currency) {
            record.set_currency(input.currency->get_value());
        }
        if (input.payout_type) {
            record.set_payout_type(input.payout_type->get_value());
        }
        if (input.customer_id) {
            record.set_customer_id(input.customer_id->get_value());
        }

        auto connector_request = app::connector::ConnectorRequestBuilder{}
                                     .operation("payout")
                                     .merchant_id(record.get_merchant_id())
                                     .amount(record.get_amount())
                                     .currency(record.get_currency())
                                     .resource_id(record.get_payout_id())
                                     .raw_payload(body)();
        if (input.confirm.value_or(false)) {
            auto choice = make_connector_decision(
                connector_request, record.get_merchant_id(), record.get_payout_id(),
                record.get_amount(), record.get_currency(), record.get_payout_type(),
                first_requested_connector(input.connector));
            if (choice.error) {
                send_error(res, send, *choice.error);
                return;
            }
            if (choice.connector) {
                const auto outcome = invoke_connector(*choice.connector, "authorize", connector_request);
                if (outcome.error) {
                    send_error(res, send, *outcome.error);
                    return;
                }
                if (!outcome.status.empty()) {
                    record.set_status(status_from_connector(outcome.status, input.auto_fulfill.value_or(false)));
                }
                record.set_connector(std::string{choice.connector->name()});
                update_latest_attempt(record.get_payout_id(), record.get_status(), {}, {});
            }
        }

        const auto previous_meta = record.get_metadata();
        record.set_metadata(merge_meta_browser_info(
            merge_meta_profile(previous_meta, resolve_profile_id(req, input.profile_id)),
            browser_info_from_headers(req, input.browser_info)));

        m_context->repository().update_payout(record);
        m_context->cache().put("payouts.updated." + record.get_payout_id(),
                               core::cache::to_cache_value(record.get_status()));
        emit_payout_webhook("updated", record);

        auto response = response_from_record(record);
        enrich_response(response, record, false);
        if (input.merchant_order_reference_id) {
            response.set_merchant_order_reference_id(*input.merchant_order_reference_id);
        }
        if (input.description) {
            response.set_description(*input.description);
        }

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payouts_confirm(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payouts_confirm")) {
            send_error(res, send, *denied);
            return;
        }

        using Input = app::model::payouts::PayoutCreateRequest;
        std::optional<Input> parsed{};
        const auto body = req.body_as_string();
        if (!body.empty()) {
            auto decoded = serde::Ser::deserialize<Input>("application/json", body);
            if (!decoded) {
                send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            parsed.emplace(std::move(*decoded));
        }

        const auto path_id = payout_id_from_path(req);
        if (!path_id.has_value()) {
            send_error(res, send, error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing payout id in path"));
            return;
        }

        PayoutLockGuard lock{m_context->locks(), std::format("payout:{}", *path_id), "payouts_confirm"};
        if (!lock.held()) {
            send_error(res, send, error(
                core::error::AppErrorKind::PRECONDITION_FAILED, "payout is locked by another request"));
            return;
        }

        m_context->metrics().record_flow("payout_confirm");
        m_context->metrics().counter("payouts_confirm_total").increment();

        auto found = find_payout_record(*path_id);
        if (!found.has_value()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "payout not found"));
            return;
        }
        auto record = *found;

        // `validate_payout_status_against_not_allowed_statuses` (`core/payouts.rs:791-804`).
        if (is_confirm_disallowed(record.get_status())) {
            send_error(res, send, error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                std::format("You cannot confirm this payout because it has status {}",
                            normalize_status(record.get_status()))));
            return;
        }

        auto connector_request = app::connector::ConnectorRequestBuilder{}
                                     .operation("payout")
                                     .merchant_id(record.get_merchant_id())
                                     .amount(record.get_amount())
                                     .currency(record.get_currency())
                                     .resource_id(record.get_payout_id())
                                     .raw_payload(body)();
        auto choice = make_connector_decision(
            connector_request, record.get_merchant_id(), record.get_payout_id(),
            record.get_amount(), record.get_currency(), record.get_payout_type(),
            parsed.has_value() ? first_requested_connector(parsed->connector) : std::string{});
        if (choice.error) {
            send_error(res, send, *choice.error);
            return;
        }
        if (choice.connector) {
            const auto outcome = invoke_connector(*choice.connector, "authorize", connector_request);
            if (outcome.error) {
                // Connector failure keeps the payout in a failed state with mapped error metadata.
                record.set_status(std::string{kFailed});
                m_context->repository().update_payout(record);
                update_latest_attempt(record.get_payout_id(), kFailed, "CONNECTOR_ERROR", outcome.error->get_message());
                emit_payout_webhook("failed", record);
                send_error(res, send, *outcome.error);
                return;
            }
            record.set_status(status_from_connector(
                outcome.status, parsed.has_value() && parsed->auto_fulfill.value_or(false)));
            record.set_connector(std::string{choice.connector->name()});
        } else {
            record.set_status(std::string{kRequiresConfirmation});
        }
        record.set_metadata(merge_meta_payout_link(record.get_metadata(), "Submitted"));
        m_context->repository().update_payout(record);
        update_latest_attempt(record.get_payout_id(), record.get_status(), {}, {});
        emit_payout_webhook("confirmed", record);

        auto response = response_from_record(record);
        enrich_response(response, record, false);
        if (parsed.has_value()) {
            if (parsed->merchant_order_reference_id) {
                response.set_merchant_order_reference_id(*parsed->merchant_order_reference_id);
            }
            if (parsed->description) {
                response.set_description(*parsed->description);
            }
        }
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payouts_cancel(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payouts_cancel")) {
            send_error(res, send, *denied);
            return;
        }

        const auto path_id = payout_id_from_path(req);
        if (!path_id.has_value()) {
            send_error(res, send, error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing payout id in path"));
            return;
        }

        PayoutLockGuard lock{m_context->locks(), std::format("payout:{}", *path_id), "payouts_cancel"};
        if (!lock.held()) {
            send_error(res, send, error(
                core::error::AppErrorKind::PRECONDITION_FAILED, "payout is locked by another request"));
            return;
        }

        m_context->metrics().record_flow("payout_cancel");
        m_context->metrics().counter("payouts_cancel_total").increment();

        auto found = find_payout_record(*path_id);
        if (!found.has_value()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "payout not found"));
            return;
        }
        auto record = *found;
        if (is_cancel_terminal(record.get_status())) {
            send_error(res, send, error(
                core::error::AppErrorKind::INVALID_REQUEST,
                std::format("Payout {:?} cannot be cancelled for status {}",
                            record.get_payout_id(), normalize_status(record.get_status()))));
            return;
        }

        // Upstream local cancellation requires both an eligible status and a connector
        // that supports instant payout (`core/payouts.rs:1008-1012`).
        if (is_eligible_for_local_payout_cancellation(record.get_status()) &&
            supports_instant_payout(record.get_connector(), record.get_payout_type())) {
            record.set_status(std::string{kCancelled});
            record.set_metadata(merge_meta_note(record.get_metadata(), "Cancelled by user"));
            m_context->repository().update_payout(record);
            // Upstream sets `error_message: Some("Cancelled by user")`, `error_code: None`
            // (`core/payouts.rs:1019-1020`).
            update_latest_attempt(record.get_payout_id(), kCancelled, {}, "Cancelled by user");
        } else {
            auto cancel_request = app::connector::ConnectorRequestBuilder{}
                                      .operation("payout_cancel")
                                      .merchant_id(record.get_merchant_id())
                                      .amount(record.get_amount())
                                      .currency(record.get_currency())
                                      .resource_id(record.get_payout_id())();
            auto connector = m_context->connector_router().select(record.get_merchant_id());
            if (!connector) {
                // No connector backend registered locally: apply the deterministic local
                // cancellation path rather than failing the request.
                record.set_status(std::string{kCancelled});
            } else {
                const auto outcome = invoke_connector(*connector, "cancel", cancel_request);
                if (outcome.error) {
                    send_error(res, send, *outcome.error);
                    return;
                }
                record.set_status(outcome.status.empty()
                                      ? std::string{kCancelled}
                                      : status_from_connector(outcome.status, false));
            }
            m_context->repository().update_payout(record);
            update_latest_attempt(record.get_payout_id(), record.get_status(),
                                  {}, "Cancelled by user");
        }
        emit_payout_webhook("cancelled", record);

        auto response = response_from_record(record);
        enrich_response(response, record, false);
        response.set_error_message(std::string{"Cancelled by user"});
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payouts_fulfill(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payouts_fulfill")) {
            send_error(res, send, *denied);
            return;
        }

        const auto path_id = payout_id_from_path(req);
        if (!path_id.has_value()) {
            send_error(res, send, error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing payout id in path"));
            return;
        }

        PayoutLockGuard lock{m_context->locks(), std::format("payout:{}", *path_id), "payouts_fulfill"};
        if (!lock.held()) {
            send_error(res, send, error(
                core::error::AppErrorKind::PRECONDITION_FAILED, "payout is locked by another request"));
            return;
        }

        m_context->metrics().record_flow("payout_fulfill");
        m_context->metrics().counter("payouts_fulfill_total").increment();

        auto found = find_payout_record(*path_id);
        if (!found.has_value()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "payout not found"));
            return;
        }
        auto record = *found;
        if (!is_fulfill_allowed(record.get_status())) {
            send_error(res, send, error(
                core::error::AppErrorKind::INVALID_REQUEST,
                std::format("Payout {:?} cannot be fulfilled for status {}",
                            record.get_payout_id(), normalize_status(record.get_status()))));
            return;
        }

        auto fulfill_request = app::connector::ConnectorRequestBuilder{}
                                   .operation("payout_fulfill")
                                   .merchant_id(record.get_merchant_id())
                                   .amount(record.get_amount())
                                   .currency(record.get_currency())
                                   .resource_id(record.get_payout_id())();
        auto connector = m_context->connector_router().select(record.get_merchant_id());
        if (connector) {
            const auto outcome = invoke_connector(*connector, "capture", fulfill_request);
            if (outcome.error) {
                record.set_status(std::string{kFailed});
                m_context->repository().update_payout(record);
                update_latest_attempt(record.get_payout_id(), kFailed, "CONNECTOR_ERROR", outcome.error->get_message());
                emit_payout_webhook("failed", record);
                send_error(res, send, *outcome.error);
                return;
            }
            record.set_connector(std::string{connector->name()});
        } else {
            m_context->metrics().counter("payouts_connector_unavailable").increment();
        }
        record.set_status(std::string{kSuccess});
        m_context->repository().update_payout(record);
        update_latest_attempt(record.get_payout_id(), kSuccess, {}, {});
        emit_payout_webhook("fulfilled", record);

        auto response = response_from_record(record);
        enrich_response(response, record, false);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payouts_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payouts_list")) {
            send_error(res, send, *denied);
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        auto constraints = list_constraints_from_query(query);
        if (!constraints) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "invalid PayoutListConstraints query"));
            return;
        }
        m_context->metrics().record_flow("payout_list");
        m_context->metrics().counter("payouts_list_total").increment();

        auto response = list_response(
            list_filters_from_constraints(*constraints),
            "", // merchant scope passes None profile_id_list (payouts.rs:354)
            time_range_from_query(query));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payouts_list_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payouts_list_profile")) {
            send_error(res, send, *denied);
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        auto constraints = list_constraints_from_query(query);
        if (!constraints) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "invalid PayoutListConstraints query"));
            return;
        }
        m_context->metrics().record_flow("payout_list_profile");
        m_context->metrics().counter("payouts_list_profile_total").increment();

        auto response = list_response(
            list_filters_from_constraints(*constraints),
            resolve_profile_id(req, std::nullopt),
            time_range_from_query(query));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payouts_list_by_filter(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payouts_list_by_filter")) {
            send_error(res, send, *denied);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payouts::PayoutListFilterConstraints;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        const auto filters = list_filters_from_dto(*parsed, query);
        // `validate_payout_list_request_for_joins` (validator.rs:609-620, max limit 20).
        if (filters.limit > 20) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "limit should be in between 1 and 20"));
            return;
        }
        m_context->metrics().record_flow("payout_list_by_filter");
        m_context->metrics().counter("payouts_list_by_filter_total").increment();

        auto response = list_response(filters, "", time_range_from_body(body));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payouts_list_by_filter_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payouts_list_by_filter_profile")) {
            send_error(res, send, *denied);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payouts::PayoutListFilterConstraints;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        const auto filters = list_filters_from_dto(*parsed, query);
        if (filters.limit > 20) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST,
                                        "limit should be in between 1 and 20"));
            return;
        }
        m_context->metrics().record_flow("payout_list_by_filter_profile");
        m_context->metrics().counter("payouts_list_by_filter_profile_total").increment();

        auto response = list_response(
            filters, resolve_profile_id(req, parsed->profile_id), time_range_from_body(body));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payouts_list_available_filters_for_merchant(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payouts_list_available_filters_for_merchant")) {
            send_error(res, send, *denied);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        // Upstream input is `web::Json<TimeRange>`, not a filters object (`payouts.rs:497`).
        const auto range = time_range_from_body(body);
        if (const auto invalid = require_time_range(range)) {
            send_error(res, send, *invalid);
            return;
        }
        m_context->metrics().record_flow("payout_available_filters_merchant");
        m_context->metrics().counter("payouts_available_filters_merchant_total").increment();

        const auto filters = available_filters(scoped_records("", range));
        auto text = serde::Ser::encode_json(filters);
        send_json(res, send, text, true);
    }

    void handle_payouts_list_available_filters_for_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payouts_list_available_filters_for_profile")) {
            send_error(res, send, *denied);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        // Upstream input is `web::Json<TimeRange>` (`payouts.rs:533`).
        const auto range = time_range_from_body(body);
        if (const auto invalid = require_time_range(range)) {
            send_error(res, send, *invalid);
            return;
        }
        m_context->metrics().record_flow("payout_available_filters_profile");
        m_context->metrics().counter("payouts_available_filters_profile_total").increment();

        const auto filters = available_filters(
            scoped_records(resolve_profile_id(req, std::nullopt), range));
        auto text = serde::Ser::encode_json(filters);
        send_json(res, send, text, true);
    }

    void handle_get_payout_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "get_payout_filters")) {
            send_error(res, send, *denied);
            return;
        }

        m_context->metrics().record_flow("get_payout_filters");
        m_context->metrics().counter("payouts_get_filters_total").increment();
        const auto auth = app::auth::AuthContextBuilder{}(req);
        auto merchant_id = app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) {
            merchant_id = auth.merchant_id();
        }

        auto text = serde::Ser::encode_json(filters_v2_from_mcas("", merchant_id));
        send_json(res, send, text, true);
    }

    void handle_get_payout_filters_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "get_payout_filters_profile")) {
            send_error(res, send, *denied);
            return;
        }

        m_context->metrics().record_flow("get_payout_filters_profile");
        m_context->metrics().counter("payouts_get_filters_profile_total").increment();
        const auto auth = app::auth::AuthContextBuilder{}(req);
        auto merchant_id = app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) {
            merchant_id = auth.merchant_id();
        }

        auto text = serde::Ser::encode_json(
            filters_v2_from_mcas(resolve_profile_id(req, std::nullopt), merchant_id));
        send_json(res, send, text, true);
    }

    void handle_get_payouts_aggregates(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "get_payouts_aggregates")) {
            send_error(res, send, *denied);
            return;
        }

        // Upstream input is `web::Query<TimeRange>` (`payouts.rs:708`); no body is read.
        m_context->metrics().record_flow("get_payouts_aggregates");
        m_context->metrics().counter("payouts_aggregates_total").increment();
        const auto query = app::http::RequestQuery{}(req);
        const auto range = time_range_from_query(query);
        if (const auto invalid = require_time_range(range)) {
            send_error(res, send, *invalid);
            return;
        }

        auto response = aggregate_response(scoped_records("", range));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_get_payouts_aggregates_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "get_payouts_aggregates_profile")) {
            send_error(res, send, *denied);
            return;
        }

        m_context->metrics().record_flow("get_payouts_aggregates_profile");
        m_context->metrics().counter("payouts_aggregates_profile_total").increment();
        const auto query = app::http::RequestQuery{}(req);
        const auto range = time_range_from_query(query);
        if (const auto invalid = require_time_range(range)) {
            send_error(res, send, *invalid);
            return;
        }

        auto response = aggregate_response(scoped_records(resolve_profile_id(req, std::nullopt), range));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payouts_manual_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto denied = authorize_operation(req, "payouts_manual_update")) {
            send_error(res, send, *denied);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payouts::PayoutsManualUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const auto& input = *parsed;
        // `PayoutsManualUpdateRequest::is_update_parameter_present` (`api_models/payouts.rs:1931-1938`).
        if (!input.get_status().has_value() && !input.get_error_code().has_value() &&
            !input.get_error_message().has_value() && !input.get_connector_payout_id().has_value()) {
            send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED,
                                        "Request must contain atleast one parameter to update"));
            return;
        }

        const auto path_id = payout_id_from_path(req);
        if (!path_id.has_value()) {
            send_error(res, send, error(
                core::error::AppErrorKind::INVALID_REQUEST, "missing payout id in path"));
            return;
        }

        PayoutLockGuard lock{m_context->locks(), std::format("payout:{}", *path_id), "payouts_manual_update"};
        if (!lock.held()) {
            send_error(res, send, error(
                core::error::AppErrorKind::PRECONDITION_FAILED, "payout is locked by another request"));
            return;
        }

        m_context->metrics().record_flow("payout_manual_update");
        m_context->metrics().counter("payouts_manual_update_total").increment();

        auto found = find_payout_record(*path_id);
        if (!found.has_value()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "payout not found"));
            return;
        }
        auto record = *found;
        const auto attempt_status = input.get_status() ? normalize_status(input.get_status()->get_value())
                                                       : std::string{};
        const std::string attempt_id = input.get_payout_attempt_id();
        std::string final_attempt_status = attempt_status;

        if (!attempt_id.empty()) {
            const auto existing = m_context->repository().find_payout_attempt_by_string(attempt_id);
            if (existing.has_value()) {
                auto attempt = *existing;
                if (!attempt_status.empty()) {
                    attempt.set_status(attempt_status);
                }
                if (input.get_error_code()) {
                    attempt.set_error_code(*input.get_error_code());
                }
                if (input.get_error_message()) {
                    attempt.set_error_message(*input.get_error_message());
                }
                m_context->repository().update_payout_attempt(attempt);
                if (final_attempt_status.empty()) {
                    final_attempt_status = attempt.get_status();
                }
            } else {
                record_payout_attempt(
                    record.get_payout_id(), record.get_merchant_id(),
                    input.get_connector_payout_id().value_or(""),
                    attempt_status, record.get_amount(), record.get_currency(),
                    input.get_error_code().value_or(""),
                    input.get_error_message().value_or(""));
            }
        }

        // Upstream updates the payout status only when the manual-updated attempt is the
        // active attempt (`payouts_manual_update_core:4633-4651`).
        if (!attempt_status.empty() && attempt_id == payout_attempt_id(record.get_payout_id())) {
            record.set_status(attempt_status);
            m_context->repository().update_payout(record);
        }
        emit_payout_webhook("manual_updated", record);

        app::model::payouts::PayoutsManualUpdateResponse response{};
        app::model::payouts::PayoutIdValue payout_value{};
        payout_value.set_value(record.get_payout_id());
        response.set_payout_id(payout_value);
        response.set_payout_attempt_id(attempt_id);
        app::model::payouts::MerchantIdValue merchant_value{};
        merchant_value.set_value(record.get_merchant_id());
        response.set_merchant_id(merchant_value);
        if (!final_attempt_status.empty()) {
            app::model::payouts::PayoutStatusValue status_value{};
            status_value.set_value(final_attempt_status);
            response.set_attempt_status(status_value);
        }
        response.set_error_code(input.get_error_code());
        response.set_error_message(input.get_error_message());
        response.set_connector_payout_id(input.get_connector_payout_id());

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/payouts"}
            .add_route(
                Route{"/create"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payouts_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payouts_create")
            )
            .add_route(
                Route{"/:payout_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payouts_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payouts_retrieve")
            )
            .add_route(
                Route{"/:payout_id"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payouts_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payouts_update")
            )
            .add_route(
                Route{"/:payout_id/confirm"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payouts_confirm(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payouts_confirm")
            )
            .add_route(
                Route{"/:payout_id/cancel"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payouts_cancel(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payouts_cancel")
            )
            .add_route(
                Route{"/:payout_id/fulfill"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payouts_fulfill(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payouts_fulfill")
            )
            .add_route(
                Route{"/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payouts_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payouts_list")
            )
            .add_route(
                Route{"/profile/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payouts_list_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payouts_list_profile")
            )
            .add_route(
                Route{"/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payouts_list_by_filter(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payouts_list_by_filter")
            )
            .add_route(
                Route{"/profile/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payouts_list_by_filter_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payouts_list_by_filter_profile")
            )
            .add_route(
                Route{"/filter"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payouts_list_available_filters_for_merchant(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payouts_list_available_filters_for_merchant")
            )
            .add_route(
                Route{"/profile/filter"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payouts_list_available_filters_for_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payouts_list_available_filters_for_profile")
            )
            .add_route(
                Route{"/v2/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_payout_filters(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payout_filters")
            )
            .add_route(
                Route{"/v2/profile/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_payout_filters_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payout_filters_profile")
            )
            .add_route(
                Route{"/aggregate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_payouts_aggregates(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payouts_aggregates")
            )
            .add_route(
                Route{"/profile/aggregate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_payouts_aggregates_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payouts_aggregates_profile")
            )
            .add_route(
                Route{"/:payout_id/manual-update"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payouts_manual_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payouts_manual_update")
            );
    }

private:
    /** @brief Per-payout Hold lock, released on every exit path via RAII. */
    class PayoutLockGuard
    {
    public:
        PayoutLockGuard(
            app::locking::LockRegistry& locks,
            std::string_view key,
            std::string owner
        ) :
            m_locks{locks},
            m_key{key}
        {
            m_lock = m_locks.acquire(m_key, std::int64_t{30}, std::move(owner));
        }

        ~PayoutLockGuard()
        {
            if (m_lock.has_value()) {
                m_locks.release(m_key);
            }
        }

        PayoutLockGuard(const PayoutLockGuard&) = delete;
        PayoutLockGuard& operator=(const PayoutLockGuard&) = delete;

        [[nodiscard]] bool held() const noexcept
        {
            return m_lock.has_value();
        }

    private:
        app::locking::LockRegistry& m_locks;
        std::string m_key{};
        std::optional<app::locking::ApiLock> m_lock{};
    };

    struct ConnectorChoice
    {
        std::shared_ptr<app::connector::IConnector> connector{};
        std::string decision_action{};
        std::string decision_connector{};
        std::optional<core::error::AppError> error{};
    };

    struct ConnectorOutcome
    {
        std::string status{};
        std::string reference_id{};
        std::string message{};
        bool shadow_killed{};
        std::optional<core::error::AppError> error{};
    };

    /** @brief Filter fields of upstream `PayoutListFilterConstraints` (`api_models/payouts.rs:1325-1368`). */
    struct ListFilters
    {
        std::string payout_id{};
        std::string merchant_order_reference_id{};
        std::string customer_id{};
        std::vector<std::string> connector{};
        std::vector<std::string> currency{};
        std::vector<std::string> status{};
        std::vector<std::string> payout_method{};
        std::string entity_type{};
        std::size_t offset{};
        std::size_t limit{};
    };

    /**
     * @brief Wire values of `common_enums::PayoutStatus` (`enums.rs:9354-9377`,
     * `#[serde(rename_all = "snake_case")]`). Exact upstream values replace the earlier
     * local "pending"/"authorized"/"fulfilled" strings.
     */
    static constexpr std::string_view kRequiresCreation{"requires_creation"};
    static constexpr std::string_view kRequiresConfirmation{"requires_confirmation"};
    static constexpr std::string_view kRequiresPayoutMethodData{"requires_payout_method_data"};
    static constexpr std::string_view kRequiresFulfillment{"requires_fulfillment"};
    static constexpr std::string_view kRequiresVendorAccountCreation{"requires_vendor_account_creation"};
    static constexpr std::string_view kInitiated{"initiated"};
    static constexpr std::string_view kPending{"pending"};
    static constexpr std::string_view kSuccess{"success"};
    static constexpr std::string_view kFailed{"failed"};
    static constexpr std::string_view kCancelled{"cancelled"};
    static constexpr std::string_view kExpired{"expired"};
    static constexpr std::string_view kReversed{"reversed"};
    static constexpr std::string_view kIneligible{"ineligible"};
    static constexpr std::string_view kNotPermitted{"not_permitted"};

    /** @brief All upstream `PayoutStatus::iter()` values in declaration order (`enums.rs:9354-9377`). */
    [[nodiscard]] std::vector<std::string_view> all_payout_statuses() const
    {
        return {
            kSuccess, kFailed, kCancelled, kInitiated, kExpired, kReversed, kPending,
            kIneligible, kNotPermitted, kRequiresCreation, kRequiresConfirmation,
            kRequiresPayoutMethodData, kRequiresFulfillment, kRequiresVendorAccountCreation,
        };
    }

    /**
     * @brief `common_utils::types::TimeRange` (`common_utils/src/types.rs:809-822`).
     * `start_time` mandatory with alias `startTime`; `end_time` optional with alias `endTime`.
     */
    struct TimeRange
    {
        std::string start_time{};
        std::optional<std::string> end_time{};
        bool present{};
    };

    [[nodiscard]] core::error::AppError error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    /**
     * @brief Upstream JWT permission names generated by `generate_permissions!`
     * {Payout: Read, entities: [Profile, Merchant]} — `services/authorization/permissions.rs:4,30-33`.
     */
    enum class PayoutPermission : std::uint8_t
    {
        PROFILE_PAYOUT_READ,
        MERCHANT_PAYOUT_READ,
    };

    /** @brief Credential combination each upstream route extractor accepts. */
    enum class PayoutAuthMode : std::uint8_t
    {
        API_KEY,                       // HeaderAuth(ApiKeyAuth)
        API_KEY_OR_JWT_PROFILE,        // auth_type(HeaderAuth(...), JWTAuth{ProfilePayoutRead})
        API_KEY_OR_JWT_MERCHANT,       // auth_type(HeaderAuth(...), JWTAuth{MerchantPayoutRead})
        JWT_PROFILE,                   // JWTAuth{ProfilePayoutRead, allow_connected:true}
        ADMIN_API,                     // AdminApiAuth
    };

    [[nodiscard]] PayoutAuthMode auth_mode_for(std::string_view operation) const
    {
        if (operation == "payouts_manual_update") {
            return PayoutAuthMode::ADMIN_API;                       // payouts.rs:803
        }
        if (operation == "get_payouts_aggregates" ||
            operation == "payouts_list" || operation == "payouts_list_by_filter" ||
            operation == "payouts_list_available_filters_for_merchant") {
            return PayoutAuthMode::API_KEY_OR_JWT_MERCHANT;         // payouts.rs:356-367,433-444,510-521,720-724
        }
        if (operation == "payouts_retrieve" || operation == "payouts_list_profile" ||
            operation == "payouts_list_by_filter_profile" ||
            operation == "payouts_list_available_filters_for_profile" ||
            operation == "get_payout_filters" || operation == "get_payout_filters_profile") {
            return PayoutAuthMode::API_KEY_OR_JWT_PROFILE;          // payouts.rs:120-131,397-408,474-485,551-562,593-604,631-642
        }
        if (operation == "get_payouts_aggregates_profile") {
            return PayoutAuthMode::JWT_PROFILE;                     // payouts.rs:752-756
        }
        return PayoutAuthMode::API_KEY;                             // payouts.rs:75-78,175-178,222-225,263-266,304-307
    }

    [[nodiscard]] std::string_view permission_name(PayoutPermission permission) const noexcept
    {
        return permission == PayoutPermission::PROFILE_PAYOUT_READ
                   ? "ProfilePayoutRead"
                   : "MerchantPayoutRead";
    }

    /** @brief Mirrors `authorization::check_permission` (`services/authorization.rs:115-128`). */
    [[nodiscard]] std::optional<core::error::AppError> check_upstream_permission(
        const app::auth::AuthContext& auth,
        PayoutPermission permission
    ) const
    {
        const auto single = auth.claim("permission");
        const auto list = auth.claim("permissions");
        if (!single.has_value() && !list.has_value()) {
            return std::nullopt;
        }
        const std::string_view required = permission_name(permission);
        const auto grants = [required](std::string_view granted)
        {
            std::string_view rest = granted;
            while (!rest.empty()) {
                const std::size_t comma = rest.find(',');
                const std::string_view token = rest.substr(0, comma);
                if (token == required || token == "ADMIN") {
                    return true;
                }
                if (comma == std::string_view::npos) {
                    break;
                }
                rest = rest.substr(comma + 1);
            }
            return false;
        };
        if ((single.has_value() && grants(*single)) || (list.has_value() && grants(*list))) {
            return std::nullopt;
        }
        return error(core::error::AppErrorKind::FORBIDDEN,
                     std::format("Access forbidden: {}", required));
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto mode = auth_mode_for(operation);
        const auto auth = app::auth::AuthContextBuilder{}(req);
        const bool has_token = !auth.token().is_empty();
        const auto api_key = auth.claim(app::auth::kClaimApiKey);
        const auto internal_key = auth.claim(app::auth::kClaimInternalApiKey);
        const bool has_api_key = api_key.has_value() && !api_key->empty();
        const bool has_internal_key = internal_key.has_value() && !internal_key->empty();

        switch (mode) {
        case PayoutAuthMode::API_KEY:
            // `HeaderAuth(ApiKeyAuth)` uses the `api-key` header; the bearer token is the
            // local equivalent (routes/payouts.rs:75-78).
            if (!has_token && !has_api_key) {
                return error(core::error::AppErrorKind::UNAUTHORIZED, "missing API key");
            }
            return std::nullopt;
        case PayoutAuthMode::ADMIN_API:
            // `AdminApiAuth` uses `admin-api-key` (routes/payouts.rs:803); the internal
            // API key is its local equivalent.
            if (!has_token && !has_internal_key) {
                return error(core::error::AppErrorKind::UNAUTHORIZED, "missing admin API key");
            }
            return std::nullopt;
        case PayoutAuthMode::JWT_PROFILE:
            if (!has_token) {
                return error(core::error::AppErrorKind::UNAUTHORIZED, "missing bearer token");
            }
            return check_upstream_permission(auth, PayoutPermission::PROFILE_PAYOUT_READ);
        case PayoutAuthMode::API_KEY_OR_JWT_MERCHANT:
            if (has_api_key) {
                return std::nullopt;
            }
            if (!has_token) {
                return error(core::error::AppErrorKind::UNAUTHORIZED, "missing bearer token");
            }
            return check_upstream_permission(auth, PayoutPermission::MERCHANT_PAYOUT_READ);
        case PayoutAuthMode::API_KEY_OR_JWT_PROFILE:
        default:
            if (has_api_key) {
                return std::nullopt;
            }
            if (!has_token) {
                return error(core::error::AppErrorKind::UNAUTHORIZED, "missing bearer token");
            }
            return check_upstream_permission(auth, PayoutPermission::PROFILE_PAYOUT_READ);
        }
    }

    [[nodiscard]] std::string to_lower(std::string_view value) const
    {
        std::string out{value};
        for (char& character : out) {
            if (character >= 'A' && character <= 'Z') {
                character = static_cast<char>(character - 'A' + 'a');
            }
        }
        return out;
    }

    [[nodiscard]] std::optional<std::int64_t> to_number(std::string_view value) const
    {
        std::int64_t parsed{0};
        const char* begin = value.data();
        const char* end = value.data() + value.size();
        const auto result = std::from_chars(begin, end, parsed);
        if (result.ec != std::errc{} || result.ptr != end) {
            return std::nullopt;
        }
        return parsed;
    }

    [[nodiscard]] std::string json_escape(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char character : value) {
            if (character == '"' || character == '\\') {
                out.push_back('\\');
            }
            out.push_back(character == '\n' ? ' ' : character);
        }
        return out;
    }

    [[nodiscard]] std::string meta_value(std::string_view json, std::string_view key) const
    {
        const std::string needle = std::format("\"{}\":\"", key);
        const auto at = json.find(needle);
        if (at == std::string_view::npos) {
            return {};
        }
        const auto start = at + needle.size();
        const auto end = json.find('"', start);
        if (end == std::string_view::npos) {
            return {};
        }
        return std::string{json.substr(start, end - start)};
    }

    [[nodiscard]] std::string encode_meta(
        std::string_view profile_id,
        std::string_view browser_info,
        std::string_view payout_link_id,
        std::string_view payout_link_status,
        std::string_view note,
        std::string_view merchant_order_reference_id = {},
        std::string_view entity_type = {}
    ) const
    {
        return std::format(
            R"({{"profile_id":"{}","browser_info":"{}","payout_link_id":"{}","payout_link_status":"{}","note":"{}","merchant_order_reference_id":"{}","entity_type":"{}"}})",
            json_escape(profile_id),
            json_escape(browser_info),
            json_escape(payout_link_id),
            json_escape(payout_link_status),
            json_escape(note),
            json_escape(merchant_order_reference_id),
            json_escape(entity_type));
    }

    [[nodiscard]] std::string merge_meta_profile(std::string_view meta, std::string_view profile_id) const
    {
        return encode_meta(
            profile_id,
            meta_value(meta, "browser_info"),
            meta_value(meta, "payout_link_id"),
            meta_value(meta, "payout_link_status"),
            meta_value(meta, "note"),
            meta_value(meta, "merchant_order_reference_id"),
            meta_value(meta, "entity_type"));
    }

    [[nodiscard]] std::string merge_meta_browser_info(std::string_view meta, std::string_view browser_info) const
    {
        return encode_meta(
            meta_value(meta, "profile_id"),
            browser_info,
            meta_value(meta, "payout_link_id"),
            meta_value(meta, "payout_link_status"),
            meta_value(meta, "note"),
            meta_value(meta, "merchant_order_reference_id"),
            meta_value(meta, "entity_type"));
    }

    [[nodiscard]] std::string merge_meta_payout_link(std::string_view meta, std::string_view link_status) const
    {
        return encode_meta(
            meta_value(meta, "profile_id"),
            meta_value(meta, "browser_info"),
            meta_value(meta, "payout_link_id"),
            link_status,
            meta_value(meta, "note"),
            meta_value(meta, "merchant_order_reference_id"),
            meta_value(meta, "entity_type"));
    }

    [[nodiscard]] std::string merge_meta_note(std::string_view meta, std::string_view note) const
    {
        return encode_meta(
            meta_value(meta, "profile_id"),
            meta_value(meta, "browser_info"),
            meta_value(meta, "payout_link_id"),
            meta_value(meta, "payout_link_status"),
            note,
            meta_value(meta, "merchant_order_reference_id"),
            meta_value(meta, "entity_type"));
    }

    /**
     * @brief Local `populate_browser_info_for_payouts` (`routes/payouts.rs:648-701`).
     * Upstream fills `ip_address` from the connection's real IP and `accept_language`
     * from `HeaderPayload::locale`, keeping any caller-supplied browser info. Locally the
     * `x-forwarded-for` / `accept-language` headers stand in for those two sources; the
     * full structured `BrowserInformation` field layout is emitted as JSON.
     */
    [[nodiscard]] std::string browser_info_from_headers(
        interfaces::io::IRequest& req,
        const std::optional<app::model::payouts::BrowserInformationValue>& provided
    ) const
    {
        std::string fields{};
        if (provided.has_value() && !provided->get_value().empty()) {
            auto value = provided->get_value();
            if (value.front() == '{') {
                value.erase(value.begin());
            }
            if (!value.empty() && value.back() == '}') {
                value.pop_back();
            }
            fields = std::move(value);
        }
        const auto append = [&fields](std::string_view key, std::string_view value)
        {
            if (value.empty() || fields.find(std::format("\"{}\"", key)) != std::string::npos) {
                return;
            }
            if (!fields.empty()) {
                fields.push_back(',');
            }
            fields += std::format("\"{}\":\"{}\"", key, value);
        };
        append("ip_address", json_escape(app::http::HeaderReader{}.get(req, "x-forwarded-for")));
        append("accept_language", json_escape(app::http::HeaderReader{}.get(req, "accept-language")));
        append("user_agent", json_escape(app::http::HeaderReader{}.get(req, "user-agent")));
        append("accept_header", json_escape(app::http::HeaderReader{}.get(req, "accept")));
        return std::format("{{{}}}", fields);
    }

    [[nodiscard]] std::string first_requested_connector(
        const std::optional<std::vector<app::model::payouts::PayoutConnectorsValue>>& connectors
    ) const
    {
        if (connectors.has_value() && !connectors->empty()) {
            return connectors->front().get_value();
        }
        return {};
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_create_input(
        const app::model::payouts::PayoutCreateRequest& input,
        std::string_view header_merchant_id
    ) const
    {
        if (!input.amount.has_value()) {
            return error(core::error::AppErrorKind::VALIDATION_FAILED, "payout amount is required");
        }
        if (!input.currency.has_value() || input.currency->get_value().empty()) {
            return error(core::error::AppErrorKind::VALIDATION_FAILED, "payout currency is required");
        }
        const bool has_method = input.payout_method_data.has_value() ||
                                input.payout_method_id.has_value() ||
                                input.payout_type.has_value() ||
                                (input.payout_token.has_value() && !input.payout_token->empty()) ||
                                input.payout_link.value_or(false) ||
                                input.payout_link_config.has_value();
        if (!has_method) {
            return error(core::error::AppErrorKind::VALIDATION_FAILED, "a payout method is required");
        }
        if (input.merchant_id.has_value() && !input.merchant_id->get_value().empty() &&
            !header_merchant_id.empty() && input.merchant_id->get_value() != header_merchant_id) {
            return error(core::error::AppErrorKind::VALIDATION_FAILED,
                         "payout merchant does not match the request merchant");
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string resolve_profile_id(
        interfaces::io::IRequest& req,
        const std::optional<app::model::payouts::ProfileIdValue>& input_profile
    ) const
    {
        if (input_profile.has_value() && !input_profile->get_value().empty()) {
            return input_profile->get_value();
        }
        const auto query = app::http::RequestQuery{}(req);
        if (const auto from_query = query.get("profile_id");
            from_query.has_value() && !from_query->empty()) {
            return *from_query;
        }
        const auto auth = app::auth::AuthContextBuilder{}(req);
        return auth.profile_id();
    }

    void store_payout_method_data(std::string_view payout_id, std::string_view raw_payload) const
    {
        if (raw_payload.empty()) {
            return;
        }
        m_context->key_store().put(
            "payout_method_data", payout_id, "raw",
            app::crypto::SecretString{std::string{raw_payload}});
        m_context->metrics().counter("payouts_method_data_stored").increment();
    }

    [[nodiscard]] ConnectorChoice make_connector_decision(
        app::connector::ConnectorRequest& request,
        std::string_view merchant_id,
        std::string_view payout_id,
        std::int64_t amount,
        std::string_view currency,
        std::string_view payout_type,
        std::string_view requested_connector
    ) const
    {
        // Local implementation of `get_connector_choice` + `make_connector_decision`
        // (`core/payouts.rs:167-418`, `:586-623`). UCS/routing tiers are external infra;
        // the local `ConnectorRouter` substitutes for them. No config-derived decision
        // program is consulted — the eligible connector comes from the request's `connector`
        // field (`payouts.rs:81` -> `core/payouts.rs:758`) and router ranking.
        ConnectorChoice choice{};
        (void)merchant_id;
        (void)payout_id;
        (void)amount;
        (void)currency;
        request.set_payment_method_type(std::string{payout_type});
        auto connector = m_context->connector_router().select_for(request);
        if (!connector) {
            const auto last = m_context->connector_router().last_error();
            if (last.has_value() && last->get_message() != "no connector registered") {
                choice.error = *last;
            } else {
                m_context->metrics().counter("payouts_connector_unavailable").increment();
            }
            return choice;
        }
        if (!requested_connector.empty() && connector->name() != requested_connector) {
            // Upstream restricts the decision to the merchant-supplied eligible connector
            // list (`PayoutCreateRequest.connector`, `core/payouts.rs:758`).
            m_context->metrics().counter("payouts_connector_unavailable").increment();
            return choice;
        }
        choice.decision_action = "allow";
        choice.decision_connector = std::string{connector->name()};
        choice.connector = std::move(connector);
        m_context->metrics().record_flow("payout.connector.decision");
        m_context->metrics().counter("payouts_connector_decision").increment();
        return choice;
    }

    [[nodiscard]] std::optional<core::error::AppError> map_connector_error(
        const app::connector::ConnectorResponse& response
    ) const
    {
        const auto combined = to_lower(std::format("{} {}", response.get_status(), response.get_message()));
        if (combined.find("timeout") != std::string::npos ||
            combined.find("timed_out") != std::string::npos) {
            return error(core::error::AppErrorKind::TIMEOUT, "connector timed out");
        }
        if (combined.find("5xx") != std::string::npos ||
            combined.find("server_error") != std::string::npos) {
            return error(core::error::AppErrorKind::CONNECTOR_ERROR, "connector server error");
        }
        if (response.get_status().empty() ||
            combined.find("failed") != std::string::npos ||
            combined.find("declined") != std::string::npos ||
            combined.find("error") != std::string::npos) {
            return error(core::error::AppErrorKind::CONNECTOR_ERROR,
                         response.get_message().empty()
                             ? std::string{"connector payout failed"}
                             : response.get_message());
        }
        return std::nullopt;
    }

    [[nodiscard]] ConnectorOutcome invoke_connector(
        app::connector::IConnector& connector,
        std::string_view verb,
        const app::connector::ConnectorRequest& request
    ) const
    {
        ConnectorOutcome outcome{};
        const auto response = [&connector, verb, &request]() {
            if (verb == "cancel") {
                return connector.cancel(request);
            }
            if (verb == "capture") {
                return connector.capture(request);
            }
            if (verb == "sync") {
                return connector.sync(request);
            }
            return connector.authorize(request);
        }();
        outcome.status = response.get_status();
        outcome.reference_id = response.get_reference_id();
        outcome.message = response.get_message();
        outcome.shadow_killed = response.get_shadow_killed();
        outcome.error = map_connector_error(response);
        return outcome;
    }

    /** @brief Maps a local connector outcome to an upstream `PayoutStatus` wire value. */
    [[nodiscard]] std::string status_from_connector(std::string_view status, bool auto_fulfill) const
    {
        const auto lower = to_lower(status);
        if (lower == "authorized" || lower == "requires_fulfillment") {
            return auto_fulfill ? std::string{kSuccess} : std::string{kRequiresFulfillment};
        }
        if (lower.empty()) {
            return std::string{kRequiresFulfillment};
        }
        return normalize_status(status);
    }

    /** @brief Normalizes legacy local status strings to upstream snake_case values. */
    [[nodiscard]] std::string normalize_status(std::string_view status) const
    {
        const auto lower = to_lower(status);
        if (lower == "succeeded") {
            return std::string{kSuccess};
        }
        if (lower == "authorized") {
            return std::string{kRequiresFulfillment};
        }
        if (lower == "fulfilled") {
            return std::string{kSuccess};
        }
        if (lower == "notpermitted") {
            return std::string{kNotPermitted};
        }
        if (lower == "requiresfulfillment") {
            return std::string{kRequiresFulfillment};
        }
        if (lower == "requirescreation") {
            return std::string{kRequiresCreation};
        }
        if (lower == "requiresconfirmation") {
            return std::string{kRequiresConfirmation};
        }
        if (lower == "requirespayoutmethoddata") {
            return std::string{kRequiresPayoutMethodData};
        }
        if (lower == "requiresvendoraccountcreation") {
            return std::string{kRequiresVendorAccountCreation};
        }
        return std::string{lower};
    }

    [[nodiscard]] bool status_is(std::string_view status, std::string_view expected) const
    {
        return normalize_status(status) == expected;
    }

    /**
     * @brief Exact port of `Connector::supports_instant_payout` (`connector_enums.rs:239-258`).
     * A missing connector name maps to `true` (`core/payouts.rs:1009-1012` `unwrap_or(true)`).
     */
    [[nodiscard]] bool connector_supports_instant_payout(
        std::string_view connector_name,
        std::string_view payout_type
    ) const
    {
        const auto connector = to_lower(connector_name);
        const auto method = to_lower(payout_type);
        if (connector == "paypal") {
            return method == "wallet";
        }
        if (connector == "checkout" || connector == "envoy" || connector == "adyenplatform" ||
            connector == "nomupay" || connector == "loonio" || connector == "truelayer" ||
            connector == "trustly" || connector == "gotymesanlam") {
            return true;
        }
        if (method == "card") {
            return true;
        }
        if (connector == "worldpay" || connector == "worldpayxml") {
            return method == "wallet";
        }
        if (connector == "itaubank" || connector == "deutschebank") {
            return method == "bank";
        }
        if (connector == "mifinity") {
            return method == "wallet" || method == "bank";
        }
        return false;
    }

    [[nodiscard]] bool supports_instant_payout(
        std::string_view connector_name,
        std::string_view payout_type
    ) const
    {
        if (connector_name.empty()) {
            return true;
        }
        return connector_supports_instant_payout(connector_name, payout_type);
    }

    /** @brief `PayoutStatus::is_terminal_status` (`enums.rs:9391-9401`). */
    [[nodiscard]] bool is_payout_terminal_state(std::string_view status) const
    {
        const auto value = normalize_status(status);
        return value != kRequiresCreation && value != kRequiresConfirmation &&
               value != kRequiresPayoutMethodData && value != kRequiresVendorAccountCreation &&
               value != kPending && value != kInitiated && value != kRequiresFulfillment;
    }

    /** @brief `helpers::is_payout_initiated` (`core/payouts/helpers.rs:1350-1359`). */
    [[nodiscard]] bool is_payout_initiated(std::string_view status) const
    {
        const auto value = normalize_status(status);
        return value != kRequiresCreation && value != kRequiresConfirmation &&
               value != kRequiresPayoutMethodData && value != kRequiresVendorAccountCreation &&
               value != kInitiated;
    }

    /** @brief `helpers::is_eligible_for_local_payout_cancellation` (`helpers.rs:1406-1415`). */
    [[nodiscard]] bool is_eligible_for_local_payout_cancellation(std::string_view status) const
    {
        const auto value = normalize_status(status);
        return value == kRequiresCreation || value == kRequiresConfirmation ||
               value == kRequiresPayoutMethodData || value == kRequiresVendorAccountCreation ||
               value == kRequiresFulfillment;
    }

    /** @brief `helpers::should_call_retrieve` (`helpers.rs:1389-1394`). */
    [[nodiscard]] bool should_call_retrieve(std::string_view status) const
    {
        const auto value = normalize_status(status);
        return value == kPending || value == kInitiated;
    }

    /** @brief `payouts_confirm_core` disallowed statuses (`core/payouts.rs:791-804`). */
    [[nodiscard]] bool is_confirm_disallowed(std::string_view status) const
    {
        const auto value = normalize_status(status);
        const std::array<std::string_view, 8> disallowed{
            kCancelled, kSuccess, kFailed, kPending, kIneligible, kNotPermitted,
            kRequiresFulfillment, kRequiresVendorAccountCreation};
        return std::find(disallowed.begin(), disallowed.end(), value) != disallowed.end();
    }

    /** @brief `payouts_cancel_core` terminal guard (`core/payouts.rs:999-1006`). */
    [[nodiscard]] bool is_cancel_terminal(std::string_view status) const
    {
        return is_payout_terminal_state(status);
    }

    /** @brief `payouts_fulfill_core` guard (`core/payouts.rs:1111-1120`). */
    [[nodiscard]] bool is_fulfill_allowed(std::string_view status) const
    {
        return !is_payout_terminal_state(status) &&
               normalize_status(status) == kRequiresFulfillment;
    }

    [[nodiscard]] std::optional<std::string> json_string_any(
        std::string_view body,
        std::initializer_list<std::string_view> keys
    ) const
    {
        for (const auto key : keys) {
            if (auto value = meta_value(body, key); !value.empty()) {
                return value;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] TimeRange time_range_from_query(const app::http::QueryParams& query) const
    {
        TimeRange range{};
        const auto start = query.get("start_time").has_value()
                               ? query.get("start_time")
                               : query.get("startTime");
        const auto end = query.get("end_time").has_value()
                             ? query.get("end_time")
                             : query.get("endTime");
        if (!start.has_value() && !end.has_value()) {
            return range;
        }
        range.present = true;
        range.start_time = start.value_or("");
        range.end_time = end;
        return range;
    }

    [[nodiscard]] TimeRange time_range_from_body(std::string_view body) const
    {
        TimeRange range{};
        const auto start = json_string_any(body, {"start_time", "startTime"});
        const auto end = json_string_any(body, {"end_time", "endTime"});
        if (!start.has_value() && !end.has_value()) {
            return range;
        }
        range.present = true;
        range.start_time = start.value_or("");
        range.end_time = end;
        return range;
    }

    /** @brief ISO8601 UTC timestamp matching `common_utils::date_time::now()` wire format. */
    [[nodiscard]] std::string created_at_now() const
    {
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
                                 std::chrono::system_clock::now().time_since_epoch()
                             ).count();
        const std::chrono::sys_seconds point{std::chrono::seconds{seconds}};
        return std::format("{:%FT%TZ}", point);
    }

    /** @brief `utils::get_payout_attempt_id(payout_id, count)` (`utils.rs:175-178`). */
    [[nodiscard]] std::string payout_attempt_id(std::string_view payout_id, int count = 1) const
    {
        return std::format("{}_{}", payout_id, count);
    }

    [[nodiscard]] bool within_time_range(std::string_view created_at, const TimeRange& range) const
    {
        if (!range.present) {
            return true;
        }
        const auto compare = [](std::string_view lhs, std::string_view rhs) -> int
        {
            if (const auto l = to_number(lhs); l.has_value()) {
                if (const auto r = to_number(rhs); r.has_value()) {
                    return *l < *r ? -1 : (*l > *r ? 1 : 0);
                }
            }
            if (lhs < rhs) {
                return -1;
            }
            if (lhs > rhs) {
                return 1;
            }
            return 0;
        };
        if (!range.start_time.empty() && compare(created_at, range.start_time) < 0) {
            return false;
        }
        if (range.end_time.has_value() && !range.end_time->empty() &&
            compare(created_at, *range.end_time) > 0) {
            return false;
        }
        return true;
    }

    /**
     * @brief Local `filter_by_constraints` + `filter_objects_based_on_profile_id_list`
     * (`core/payouts.rs:1202-1212`, `:1456-1473`). Empty profile id means upstream
     * `None` (merchant scope); a non-empty id is the single authenticated profile.
     */
    [[nodiscard]] std::vector<app::store::payouts::PayoutRecord> scoped_records(
        std::string_view profile_id,
        const TimeRange& range
    ) const
    {
        std::vector<app::store::payouts::PayoutRecord> out{};
        for (const auto& record : m_context->repository().list_payouts()) {
            if (!profile_id.empty() &&
                meta_value(record.get_metadata(), "profile_id") != profile_id) {
                continue;
            }
            if (!within_time_range(record.get_created_at(), range)) {
                continue;
            }
            out.push_back(record);
        }
        return out;
    }

    app::store::payout_attempts::PayoutAttemptRecord record_payout_attempt(
        std::string_view payout_id,
        std::string_view merchant_id,
        std::string_view connector,
        std::string_view status,
        std::int64_t amount,
        std::string_view currency,
        std::string_view error_code = {},
        std::string_view error_message = {}
    ) const
    {
        app::store::payout_attempts::PayoutAttemptRecord attempt{};
        attempt.set_attempt_id(payout_attempt_id(payout_id));
        attempt.set_payout_id(std::string{payout_id});
        attempt.set_merchant_id(std::string{merchant_id});
        attempt.set_connector(std::string{connector});
        attempt.set_status(std::string{status});
        attempt.set_amount(amount);
        attempt.set_currency(std::string{currency});
        attempt.set_error_code(std::string{error_code});
        attempt.set_error_message(std::string{error_message});
        return m_context->repository().save_payout_attempt(attempt);
    }

    void update_latest_attempt(
        std::string_view payout_id,
        std::string_view status,
        std::string_view error_code,
        std::string_view error_message
    ) const
    {
        for (auto attempt : m_context->repository().list_payout_attempts()) {
            if (attempt.get_payout_id() != payout_id) {
                continue;
            }
            attempt.set_status(std::string{status});
            if (!error_code.empty()) {
                attempt.set_error_code(std::string{error_code});
            }
            if (!error_message.empty()) {
                attempt.set_error_message(std::string{error_message});
            }
            m_context->repository().update_payout_attempt(attempt);
            return;
        }
    }

    [[nodiscard]] std::vector<app::model::payouts::PayoutAttemptResponse> attempts_for(
        std::string_view payout_id
    ) const
    {
        std::vector<app::model::payouts::PayoutAttemptResponse> out{};
        for (const auto& attempt : m_context->repository().list_payout_attempts()) {
            if (attempt.get_payout_id() != payout_id) {
                continue;
            }
            app::model::payouts::PayoutAttemptResponse item{};
            item.set_attempt_id(attempt.get_attempt_id());
            app::model::payouts::PayoutStatusValue status{};
            status.set_value(attempt.get_status());
            item.set_status(status);
            app::model::payouts::MinorUnitValue amount{};
            amount.set_value(std::to_string(attempt.get_amount()));
            item.set_amount(amount);
            app::model::payouts::CurrencyValue currency{};
            currency.set_value(attempt.get_currency());
            item.set_currency(currency);
            if (!attempt.get_connector().empty()) {
                item.set_connector(attempt.get_connector());
            }
            if (!attempt.get_error_code().empty()) {
                item.set_error_code(attempt.get_error_code());
            }
            if (!attempt.get_error_message().empty()) {
                item.set_error_message(attempt.get_error_message());
            }
            out.push_back(std::move(item));
        }
        return out;
    }

    /**
     * @brief Upstream outgoing webhook trigger (`utils.rs:1470-1537`). The event type is
     * derived from the final PayoutStatus (`common_enums/transformers.rs:2194-2215`); statuses
     * without an event mapping emit nothing. The local EventBus + WebhookDeliveryService
     * substitute for `create_event_and_trigger_outgoing_webhook`.
     */
    void emit_payout_webhook(std::string_view, const app::store::payouts::PayoutRecord& record) const
    {
        const auto status = normalize_status(record.get_status());
        std::string_view event_type{};
        if (status == kSuccess) {
            event_type = "payout_success";
        } else if (status == kFailed) {
            event_type = "payout_failed";
        } else if (status == kCancelled) {
            event_type = "payout_cancelled";
        } else if (status == kInitiated) {
            event_type = "payout_initiated";
        } else if (status == kExpired) {
            event_type = "payout_expired";
        } else if (status == kReversed) {
            event_type = "payout_reversed";
        } else if (status == kNotPermitted) {
            event_type = "payout_not_permitted";
        }
        if (event_type.empty()) {
            return;
        }
        const auto payload = std::format(
            R"({{"payout_id":"{}","merchant_id":"{}","status":"{}","event_type":"{}","created":"{}","amount":"{}","currency":"{}"}})",
            json_escape(record.get_payout_id()), json_escape(record.get_merchant_id()),
            json_escape(status), event_type, json_escape(record.get_created_at()),
            record.get_amount(), json_escape(record.get_currency()));
        const auto event_id = m_context->events().publish(
            std::format("payout.{}", event_type), payload);
        m_context->webhook_delivery().enqueue(
            event_id, std::format("merchant://{}/webhooks", record.get_merchant_id()));
        m_context->metrics().record_flow("payout.webhook");
        m_context->metrics().counter("payouts_webhook_emitted").increment();
    }

    /** @brief Extract the :payout_id segment that follows "payouts" in v1/v2 paths. */
    [[nodiscard]] std::optional<std::string> payout_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i{0}; i + 1 < segments.count(); ++i) {
            if (segments.at(i).value_or(std::string_view{}) == "payouts") {
                const auto candidate = segments.at(i + 1);
                if (candidate.has_value() && !candidate->empty()) {
                    return std::string{*candidate};
                }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<app::store::payouts::PayoutRecord> find_payout_record(std::string_view payout_id) const
    {
        if (payout_id.empty()) {
            return std::nullopt;
        }
        return m_context->repository().find_payout_by_string(payout_id);
    }

    [[nodiscard]] app::model::payouts::PayoutCreateResponse response_from_record(
        const app::store::payouts::PayoutRecord& record
    ) const
    {
        using Response = app::model::payouts::PayoutCreateResponse;
        Response response{};
        app::model::payouts::PayoutIdValue payout_value{};
        payout_value.set_value(record.get_payout_id());
        response.set_payout_id(payout_value);
        app::model::payouts::MerchantIdValue merchant_value{};
        merchant_value.set_value(record.get_merchant_id());
        response.set_merchant_id(merchant_value);
        app::model::payouts::MinorUnitValue amount_value{};
        amount_value.set_value(std::to_string(record.get_amount()));
        response.set_amount(amount_value);
        app::model::payouts::CurrencyValue currency_value{};
        currency_value.set_value(record.get_currency());
        response.set_currency(currency_value);
        app::model::payouts::PayoutStatusValue status_value{};
        status_value.set_value(record.get_status());
        response.set_status(status_value);
        if (!record.get_connector().empty()) {
            response.set_connector(record.get_connector());
        }
        if (!record.get_payout_type().empty()) {
            app::model::payouts::PayoutTypeValue type_value{};
            type_value.set_value(record.get_payout_type());
            response.set_payout_type(type_value);
        }
        if (!record.get_customer_id().empty()) {
            app::model::payouts::CustomerIdValue customer_value{};
            customer_value.set_value(record.get_customer_id());
            response.set_customer_id(customer_value);
        }
        return response;
    }

    void enrich_response(
        app::model::payouts::PayoutCreateResponse& response,
        const app::store::payouts::PayoutRecord& record,
        bool expand_attempts
    ) const
    {
        const auto metadata = record.get_metadata();
        const auto profile_id = meta_value(metadata, "profile_id");
        if (!profile_id.empty()) {
            app::model::payouts::ProfileIdValue profile{};
            profile.set_value(profile_id);
            response.set_profile_id(profile);
        }
        if (!record.get_created_at().empty()) {
            app::model::payouts::PrimitiveDateTimeValue created{};
            created.set_value(record.get_created_at());
            response.set_created(created);
        }
        const auto payout_link_id = meta_value(metadata, "payout_link_id");
        if (!payout_link_id.empty()) {
            app::model::payouts::PayoutLinkResponse link{};
            link.set_payout_link_id(payout_link_id);
            app::model::payouts::SecretValue secret{};
            secret.set_value(std::format("/payout-links/{}", payout_link_id));
            link.set_link(secret);
            response.set_payout_link(link);
        }
        if (expand_attempts) {
            response.set_attempts(attempts_for(record.get_payout_id()));
        }
    }

    [[nodiscard]] std::vector<std::string> distinct_field(
        const std::vector<app::store::payouts::PayoutRecord>& records,
        const std::function<std::string(const app::store::payouts::PayoutRecord&)>& getter
    ) const
    {
        std::vector<std::string> values{};
        for (const auto& record : records) {
            const auto value = getter(record);
            if (value.empty()) {
                continue;
            }
            if (std::find(values.begin(), values.end(), value) == values.end()) {
                values.emplace_back(value);
            }
        }
        return values;
    }

    /** @brief `get_aggregates_for_payouts` (`core/payouts.rs:4502-4529`): all statuses, zero-filled. */
    [[nodiscard]] app::model::payouts::PayoutsAggregateResponse aggregate_response(
        const std::vector<app::store::payouts::PayoutRecord>& records
    ) const
    {
        std::unordered_map<std::string, std::int64_t> counts{};
        for (const auto status : all_payout_statuses()) {
            counts.emplace(std::string{status}, 0);
        }
        for (const auto& record : records) {
            ++counts[normalize_status(record.get_status())];
        }
        app::model::payouts::PayoutsAggregateResponse response{};
        response.set_status_with_count(std::move(counts));
        return response;
    }

    /**
     * @brief Local `get_filters_for_payouts` (`core/payouts.rs:1475-1491`).
     */
    [[nodiscard]] app::model::payouts::PayoutListFilters available_filters(
        const std::vector<app::store::payouts::PayoutRecord>& records
    ) const
    {
        app::model::payouts::PayoutListFilters filters{};
        std::vector<app::model::payouts::PayoutConnectorsValue> connectors{};
        for (const auto& connector : distinct_field(records, [](const auto& r) { return r.get_connector(); })) {
            app::model::payouts::PayoutConnectorsValue value{};
            value.set_value(connector);
            connectors.push_back(std::move(value));
        }
        filters.set_connector(std::move(connectors));
        std::vector<app::model::payouts::CurrencyValue> currencies{};
        for (const auto& currency : distinct_field(records, [](const auto& r) { return r.get_currency(); })) {
            app::model::payouts::CurrencyValue value{};
            value.set_value(currency);
            currencies.push_back(std::move(value));
        }
        filters.set_currency(std::move(currencies));
        std::vector<app::model::payouts::PayoutStatusValue> statuses{};
        for (const auto& status : distinct_field(records, [](const auto& r) { return r.get_status(); })) {
            app::model::payouts::PayoutStatusValue value{};
            value.set_value(normalize_status(status));
            statuses.push_back(std::move(value));
        }
        filters.set_status(std::move(statuses));
        std::vector<app::model::payouts::PayoutTypeValue> methods{};
        for (const auto& method : distinct_field(records, [](const auto& r) { return r.get_payout_type(); })) {
            app::model::payouts::PayoutTypeValue value{};
            value.set_value(method);
            methods.push_back(std::move(value));
        }
        filters.set_payout_method(std::move(methods));
        return filters;
    }

    /** @brief `TimeRange.start_time` is mandatory for filter/aggregate routes (`types.rs:814-821`). */
    [[nodiscard]] std::optional<core::error::AppError> require_time_range(const TimeRange& range) const
    {
        if (!range.present || range.start_time.empty()) {
            return error(core::error::AppErrorKind::INVALID_REQUEST, "start_time is required");
        }
        return std::nullopt;
    }

    /** @brief `api_enums::Currency::iter()` (`common_enums/src/enums.rs:1093-1252`). */
    [[nodiscard]] std::vector<std::string_view> all_currencies() const
    {
        static const std::vector<std::string_view> currencies{
            "AED", "AFN", "ALL", "AMD", "ANG", "AOA", "ARS", "AUD", "AWG", "AZN", "BAM", "BBD",
            "BDT", "BGN", "BHD", "BIF", "BMD", "BND", "BOB", "BRL", "BSD", "BTN", "BWP", "BYN",
            "BZD", "CAD", "CDF", "CHF", "CLF", "CLP", "CNY", "COP", "CRC", "CUC", "CUP", "CVE",
            "CZK", "DJF", "DKK", "DOP", "DZD", "EGP", "ERN", "ETB", "EUR", "FJD", "FKP", "GBP",
            "GEL", "GHS", "GIP", "GMD", "GNF", "GTQ", "GYD", "HKD", "HNL", "HRK", "HTG", "HUF",
            "IDR", "ILS", "INR", "IQD", "IRR", "ISK", "JMD", "JOD", "JPY", "KES", "KGS", "KHR",
            "KMF", "KPW", "KRW", "KWD", "KYD", "KZT", "LAK", "LBP", "LKR", "LRD", "LSL", "LYD",
            "MAD", "MDL", "MGA", "MKD", "MMK", "MNT", "MOP", "MRU", "MUR", "MVR", "MWK", "MXN",
            "MYR", "MZN", "NAD", "NGN", "NIO", "NOK", "NPR", "NZD", "OMR", "PAB", "PEN", "PGK",
            "PHP", "PKR", "PLN", "PYG", "QAR", "RON", "RSD", "RUB", "RWF", "SAR", "SBD", "SCR",
            "SDG", "SEK", "SGD", "SHP", "SLE", "SLL", "SOS", "SRD", "SSP", "STD", "STN", "SVC",
            "SYP", "SZL", "THB", "TJS", "TMT", "TND", "TOP", "TRY", "TTD", "TWD", "TZS", "UAH",
            "UGX", "USD", "UYU", "UZS", "VES", "VND", "VUV", "WST", "XAF", "XCD", "XOF", "XPF",
            "YER", "ZAR", "ZMW", "ZWL",
        };
        return currencies;
    }

    [[nodiscard]] std::vector<std::string> all_json_string_values(
        std::string_view json,
        std::string_view key
    ) const
    {
        const std::string needle = std::format("\"{}\":\"", key);
        std::vector<std::string> out{};
        std::size_t cursor{0};
        while ((cursor = json.find(needle, cursor)) != std::string_view::npos) {
            const auto start = cursor + needle.size();
            const auto end = json.find('"', start);
            if (end == std::string_view::npos) {
                break;
            }
            out.emplace_back(json.substr(start, end - start));
            cursor = end + 1;
        }
        return out;
    }

    /**
     * @brief Exact `get_payout_filters_core` (`core/payouts.rs:1495-1559`): payout
     * merchant connector accounts feed the connector map and payout-method set; currency
     * and status are the full upstream enums. Connector map values are structured
     * `MerchantConnectorInfo` objects matching `api_models/src/admin.rs:1626-1633`.
     */
    [[nodiscard]] app::model::payouts::PayoutListFiltersV2 filters_v2_from_mcas(
        std::string_view profile_id,
        std::string_view merchant_id
    ) const
    {
        app::model::payouts::PayoutListFiltersV2 filters{};
        std::vector<app::model::payouts::CurrencyValue> currencies{};
        for (const auto currency : all_currencies()) {
            app::model::payouts::CurrencyValue value{};
            value.set_value(std::string{currency});
            currencies.push_back(std::move(value));
        }
        filters.set_currency(std::move(currencies));
        std::vector<app::model::payouts::PayoutStatusValue> statuses{};
        for (const auto status : all_payout_statuses()) {
            app::model::payouts::PayoutStatusValue value{};
            value.set_value(std::string{status});
            statuses.push_back(std::move(value));
        }
        filters.set_status(std::move(statuses));

        std::unordered_map<std::string, std::vector<app::model::payouts::MerchantConnectorInfo>> connector_map{};
        std::vector<app::model::payouts::PayoutTypeValue> payout_methods{};
        for (const auto& mca : m_context->repository().list_merchant_connectors()) {
            if (!merchant_id.empty() && mca.get_merchant_id() != merchant_id) {
                continue;
            }
            if (!profile_id.empty() && mca.get_profile_id() != profile_id) {
                continue;
            }
            if (mca.get_connector_type() != "payout_processor") {
                continue;
            }
            if (!mca.get_connector_label().empty()) {
                // Upstream `MerchantConnectorResponse::to_merchant_connector_info`
                // (`api_models/src/admin.rs:1726-1732`): connector_label from the label,
                // merchant_connector_id from the MCA id, connector_type from the record.
                app::model::payouts::MerchantConnectorInfo info{};
                info.set_connector_label(mca.get_connector_label());
                info.set_merchant_connector_id(mca.get_merchant_connector_id());
                info.set_connector_type(mca.get_connector_type());
                connector_map[mca.get_connector_name()].push_back(std::move(info));
            }
            for (const auto& method : all_json_string_values(mca.get_payment_methods_enabled(), "payment_method")) {
                if (std::find_if(payout_methods.begin(), payout_methods.end(),
                                 [&method](const auto& value) { return value.get_value() == method; }) ==
                    payout_methods.end()) {
                    app::model::payouts::PayoutTypeValue value{};
                    value.set_value(method);
                    payout_methods.push_back(std::move(value));
                }
            }
        }
        filters.set_connector(std::move(connector_map));
        filters.set_payout_method(std::move(payout_methods));
        return filters;
    }

    /**
     * @brief Local `payouts_filtered_list_core` row filtering (`core/payouts.rs:1338-1453`).
     * Every field of the typed `PayoutListFilterConstraints` is applied to persisted rows;
     * `total_count` matches the un-paginated filtered set.
     */
    [[nodiscard]] app::model::payouts::PayoutListResponse list_response(
        const ListFilters& filters,
        std::string_view profile_id,
        const TimeRange& range
    ) const
    {
        const auto records = scoped_records(profile_id, range);
        std::vector<app::store::payouts::PayoutRecord> matched{};
        for (const auto& record : records) {
            if (!filters.payout_id.empty() && record.get_payout_id() != filters.payout_id) {
                continue;
            }
            if (!filters.merchant_order_reference_id.empty() &&
                meta_value(record.get_metadata(), "merchant_order_reference_id") !=
                    filters.merchant_order_reference_id) {
                continue;
            }
            if (!filters.customer_id.empty() && record.get_customer_id() != filters.customer_id) {
                continue;
            }
            if (!filters.connector.empty() &&
                std::find(filters.connector.begin(), filters.connector.end(), record.get_connector()) ==
                    filters.connector.end()) {
                continue;
            }
            if (!filters.currency.empty() &&
                std::find(filters.currency.begin(), filters.currency.end(), record.get_currency()) ==
                    filters.currency.end()) {
                continue;
            }
            if (!filters.status.empty()) {
                const auto status = normalize_status(record.get_status());
                const bool any = std::any_of(filters.status.begin(), filters.status.end(),
                                             [&status](const auto& c) { return status == c; });
                if (!any) {
                    continue;
                }
            }
            if (!filters.payout_method.empty() &&
                std::find(filters.payout_method.begin(), filters.payout_method.end(),
                          record.get_payout_type()) == filters.payout_method.end()) {
                continue;
            }
            if (!filters.entity_type.empty() &&
                meta_value(record.get_metadata(), "entity_type") != filters.entity_type) {
                continue;
            }
            matched.push_back(record);
        }
        std::vector<app::model::payouts::PayoutCreateResponse> data{};
        for (std::size_t i = filters.offset; i < matched.size(); ++i) {
            if (filters.limit > 0 && data.size() >= filters.limit) {
                break;
            }
            auto item = response_from_record(matched[i]);
            enrich_response(item, matched[i], false);
            data.push_back(std::move(item));
        }
        app::model::payouts::PayoutListResponse response{};
        response.set_data(std::move(data));
        response.set_size(response.get_data().size());
        response.set_total_count(static_cast<std::int64_t>(matched.size()));
        return response;
    }

    [[nodiscard]] std::size_t parse_count(const std::optional<std::string>& text) const
    {
        if (!text.has_value()) {
            return 0;
        }
        return to_count(*text, std::nullopt);
    }

    [[nodiscard]] std::size_t to_count(std::string_view value, const std::optional<std::string>& fallback) const
    {
        std::size_t parsed{0};
        if (!value.empty()) {
            const auto result = std::from_chars(value.data(), value.data() + value.size(), parsed);
            if (result.ec == std::errc{}) {
                return parsed;
            }
        }
        if (fallback.has_value()) {
            const auto& text = *fallback;
            const auto result = std::from_chars(text.data(), text.data() + text.size(), parsed);
            if (result.ec == std::errc{}) {
                return parsed;
            }
        }
        return 0;
    }

    /** @brief Builds a JSON object from selected query keys for typed `web::Query` DTO parsing. */
    [[nodiscard]] std::string query_json(
        const app::http::QueryParams& query,
        std::initializer_list<std::string_view> keys
    ) const
    {
        std::string json{"{"};
        bool first{true};
        for (const auto key : keys) {
            if (const auto value = query.get(key); value.has_value()) {
                if (!first) {
                    json.push_back(',');
                }
                first = false;
                json += std::format("\"{}\":\"{}\"", key, json_escape(*value));
            }
        }
        json.push_back('}');
        return json;
    }

    /** @brief Typed `PayoutListConstraints` (`api_models/payouts.rs:1295-1321`) from query. */
    [[nodiscard]] std::optional<app::model::payouts::PayoutListConstraints> list_constraints_from_query(
        const app::http::QueryParams& query
    ) const
    {
        return serde::Ser::deserialize<app::model::payouts::PayoutListConstraints>(
            "application/json",
            query_json(query, {"customer_id", "starting_after", "ending_before", "limit", "created"}));
    }

    [[nodiscard]] std::vector<std::string> status_values(
        const std::optional<std::vector<app::model::payouts::PayoutStatusValue>>& values
    ) const
    {
        std::vector<std::string> out{};
        if (values.has_value()) {
            for (const auto& value : *values) {
                out.push_back(normalize_status(value.get_value()));
            }
        }
        return out;
    }

    template<typename Value>
    [[nodiscard]] std::vector<std::string> wrapper_values(
        const std::optional<std::vector<Value>>& values
    ) const
    {
        std::vector<std::string> out{};
        if (values.has_value()) {
            for (const auto& value : *values) {
                out.push_back(value.get_value());
            }
        }
        return out;
    }

    /** @brief Maps the typed filter DTO (`api_models/payouts.rs:1325-1368`) to row filters. */
    [[nodiscard]] ListFilters list_filters_from_dto(
        const app::model::payouts::PayoutListFilterConstraints& input,
        const app::http::QueryParams& query
    ) const
    {
        ListFilters filters{};
        filters.payout_id = input.payout_id ? input.payout_id->get_value()
                                            : query.get("payout_id").value_or("");
        filters.merchant_order_reference_id = input.merchant_order_reference_id.value_or("");
        filters.customer_id = input.customer_id ? input.customer_id->get_value()
                                                : query.get("customer_id").value_or("");
        filters.connector = wrapper_values(input.connector);
        filters.currency = wrapper_values(input.currency);
        filters.status = status_values(input.status);
        filters.payout_method = wrapper_values(input.payout_method);
        filters.entity_type = input.entity_type ? input.entity_type->get_value() : std::string{};
        filters.offset = to_count(input.offset.get_value(), query.get("offset"));
        filters.limit = to_count(input.limit.get_value(), query.get("limit"));
        return filters;
    }

    /** @brief GET `/payouts/list` uses `PayoutListConstraints`, not the filter DTO. */
    [[nodiscard]] ListFilters list_filters_from_constraints(
        const app::model::payouts::PayoutListConstraints& constraints
    ) const
    {
        ListFilters filters{};
        if (constraints.customer_id.has_value()) {
            filters.customer_id = constraints.customer_id->get_value();
        }
        filters.limit = to_count(constraints.limit.get_value(), std::nullopt);
        return filters;
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

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::payouts
