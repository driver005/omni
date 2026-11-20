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
        const auto browser_info = browser_info_from_headers(req);
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

        std::string status{"pending"};
        std::string connector_name{};
        if (input.payout_token && !input.payout_token->empty()) {
            // Deterministic substitute for proxy::ExternalVaultPayout: a token-based payout
            // parks the method data in the local key store and never calls a PSP.
            status = "pending";
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
                status = outcome.status.empty() ? std::string{"pending"} : outcome.status;
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
        record.set_created_at(std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()));
        if (input.payout_type) {
            record.set_payout_type(input.payout_type->get_value());
        }
        if (input.customer_id) {
            record.set_customer_id(input.customer_id->get_value());
        }
        record.set_metadata(encode_meta(
            profile_id, browser_info, payout_link_id,
            payout_link_id.empty() ? std::string{} : std::string{"Submitted"}, note));
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
        const auto query = app::http::RequestQuery{}(req);

        app::model::payouts::PayoutRetrieveBody retrieve{};
        const auto body = req.body_as_string();
        if (!body.empty()) {
            auto parsed = serde::Ser::deserialize<app::model::payouts::PayoutRetrieveBody>(
                "application/json", body);
            if (!parsed) {
                send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            retrieve = std::move(*parsed);
        }
        const bool force_sync = retrieve.get_force_sync().value_or(
            query.get("force_sync").value_or("false") == "true");
        const bool expand_attempts = retrieve.get_expand_attempts().value_or(
            query.get("expand_attempts").value_or("false") == "true");
        (void)query.get("return_raw_connector_response");

        m_context->metrics().record_flow("payout_retrieve");
        m_context->metrics().counter("payouts_retrieve_total").increment();

        auto found = find_payout_record(*path_id);
        if (!found.has_value()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "payout not found"));
            return;
        }
        auto record = *found;

        if (force_sync) {
            const auto scope = resolve_profile_id(req, std::nullopt);
            (void)scope;
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
                    record.set_status(outcome.status);
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
                    record.set_status(outcome.status);
                }
                record.set_connector(std::string{choice.connector->name()});
                update_latest_attempt(record.get_payout_id(), record.get_status(), {}, {});
            }
        }

        const auto previous_meta = record.get_metadata();
        record.set_metadata(merge_meta_profile(
            previous_meta, resolve_profile_id(req, input.profile_id)));

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

        // In the local model "pending" represents upstream RequiresConfirmation, so it is
        // the only confirmable state. Every later/terminal state is rejected.
        if (is_confirm_disallowed(record.get_status())) {
            send_error(res, send, error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                std::format("payout in status '{}' cannot be confirmed", record.get_status())));
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
                record.set_status("failed");
                m_context->repository().update_payout(record);
                update_latest_attempt(record.get_payout_id(), "failed", "CONNECTOR_ERROR", outcome.error->get_message());
                emit_payout_webhook("failed", record);
                send_error(res, send, *outcome.error);
                return;
            }
            record.set_status(outcome.status.empty() ? std::string{"authorized"} : outcome.status);
            record.set_connector(std::string{choice.connector->name()});
        } else {
            record.set_status("pending");
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
                core::error::AppErrorKind::PRECONDITION_FAILED,
                std::format("payout in status '{}' is terminal and cannot be cancelled",
                            record.get_status())));
            return;
        }

        if (supports_instant_payout(record.get_connector())) {
            record.set_status("cancelled");
            record.set_metadata(merge_meta_note(record.get_metadata(), "Cancelled by user"));
            m_context->repository().update_payout(record);
            update_latest_attempt(record.get_payout_id(), "cancelled",
                                  "CANCELLED_BY_USER", "Cancelled by user");
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
                record.set_status("cancelled");
            } else {
                const auto outcome = invoke_connector(*connector, "cancel", cancel_request);
                if (outcome.error) {
                    send_error(res, send, *outcome.error);
                    return;
                }
                record.set_status(outcome.status.empty() ? std::string{"cancelled"} : outcome.status);
            }
            m_context->repository().update_payout(record);
            update_latest_attempt(record.get_payout_id(), record.get_status(),
                                  "CANCELLED_BY_USER", "Cancelled by user");
        }
        emit_payout_webhook("cancelled", record);

        auto response = response_from_record(record);
        enrich_response(response, record, false);
        response.set_error_message(std::string{"Cancelled by user"});
        response.set_error_code(std::string{"CANCELLED_BY_USER"});
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
                core::error::AppErrorKind::PRECONDITION_FAILED,
                std::format("payout in status '{}' cannot be fulfilled", record.get_status())));
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
                record.set_status("failed");
                m_context->repository().update_payout(record);
                update_latest_attempt(record.get_payout_id(), "failed", "CONNECTOR_ERROR", outcome.error->get_message());
                emit_payout_webhook("failed", record);
                send_error(res, send, *outcome.error);
                return;
            }
            record.set_connector(std::string{connector->name()});
        } else {
            m_context->metrics().counter("payouts_connector_unavailable").increment();
        }
        record.set_status("fulfilled");
        m_context->repository().update_payout(record);
        update_latest_attempt(record.get_payout_id(), "fulfilled", {}, {});
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
        m_context->metrics().record_flow("payout_list");
        m_context->metrics().counter("payouts_list_total").increment();

        auto response = list_response(
            query.get("payout_id").value_or(""),
            query.get("customer_id").value_or(""),
            query.get("status").value_or(""),
            query.get("currency").value_or(""),
            query.get("profile_id").value_or(""),
            time_range_from_query(query),
            parse_count(query.get("offset")),
            parse_count(query.get("limit"))
        );
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
        m_context->metrics().record_flow("payout_list_profile");
        m_context->metrics().counter("payouts_list_profile_total").increment();

        auto response = list_response(
            query.get("payout_id").value_or(""),
            query.get("customer_id").value_or(""),
            query.get("status").value_or(""),
            query.get("currency").value_or(""),
            resolve_profile_id(req, std::nullopt),
            time_range_from_query(query),
            parse_count(query.get("offset")),
            parse_count(query.get("limit"))
        );
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

        m_context->metrics().record_flow("payout_list_by_filter");
        m_context->metrics().counter("payouts_list_by_filter_total").increment();

        const auto query = app::http::RequestQuery{}(req);
        auto response = list_response(
            parsed->payout_id ? parsed->payout_id->get_value() : query.get("payout_id").value_or(""),
            parsed->customer_id ? parsed->customer_id->get_value() : query.get("customer_id").value_or(""),
            status_filter(*parsed, query.get("status")),
            currency_filter(*parsed, query.get("currency")),
            parsed->profile_id ? parsed->profile_id->get_value() : resolve_profile_id(req, std::nullopt),
            parsed->time_range ? parsed->time_range->get_value() : time_range_from_query(query),
            to_count(parsed->offset.get_value(), query.get("offset")),
            to_count(parsed->limit.get_value(), query.get("limit"))
        );
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

        m_context->metrics().record_flow("payout_list_by_filter_profile");
        m_context->metrics().counter("payouts_list_by_filter_profile_total").increment();

        const auto query = app::http::RequestQuery{}(req);
        auto response = list_response(
            parsed->payout_id ? parsed->payout_id->get_value() : query.get("payout_id").value_or(""),
            parsed->customer_id ? parsed->customer_id->get_value() : query.get("customer_id").value_or(""),
            status_filter(*parsed, query.get("status")),
            currency_filter(*parsed, query.get("currency")),
            resolve_profile_id(req, parsed->profile_id),
            parsed->time_range ? parsed->time_range->get_value() : time_range_from_query(query),
            to_count(parsed->offset.get_value(), query.get("offset")),
            to_count(parsed->limit.get_value(), query.get("limit"))
        );
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
        using Input = app::model::payouts::PayoutListFilters;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        m_context->metrics().record_flow("payout_available_filters_merchant");
        m_context->metrics().counter("payouts_available_filters_merchant_total").increment();

        const auto query = app::http::RequestQuery{}(req);
        const auto records = scoped_records(
            query.get("profile_id").value_or(""), time_range_from_query(query));
        auto filters = *parsed;
        const auto statuses = distinct_field(records, [](const auto& record) { return record.get_status(); });
        if (!statuses.empty()) {
            std::vector<app::model::payouts::PayoutStatusValue> values{};
            for (const auto& value : statuses) {
                app::model::payouts::PayoutStatusValue item{};
                item.set_value(value);
                values.push_back(std::move(item));
            }
            filters.set_status(std::move(values));
        }

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
        using Input = app::model::payouts::PayoutListFiltersV2;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        m_context->metrics().record_flow("payout_available_filters_profile");
        m_context->metrics().counter("payouts_available_filters_profile_total").increment();

        const auto query = app::http::RequestQuery{}(req);
        const auto records = scoped_records(
            resolve_profile_id(req, std::nullopt), time_range_from_query(query));
        auto filters = *parsed;
        const auto statuses = distinct_field(records, [](const auto& record) { return record.get_status(); });
        if (!statuses.empty()) {
            std::vector<app::model::payouts::PayoutStatusValue> values{};
            for (const auto& value : statuses) {
                app::model::payouts::PayoutStatusValue item{};
                item.set_value(value);
                values.push_back(std::move(item));
            }
            filters.set_status(std::move(values));
        }

        auto text = serde::Ser::encode_json(filters);
        send_json(res, send, text, true);
    }

    void handle_payouts_accounts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream payouts_accounts is an unauthenticated stub whose route is commented out
        // of app.rs. It is not present in RouteAuthRegistry, so this extra C++-only route
        // deliberately keeps its coarse token check instead of failing closed with FORBIDDEN.
        const auto auth_error = m_context->authorizer().authorize(
            app::auth::TokenParser{}(req),
            app::auth::AppPermission::PAYMENT_READ
        );
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        m_context->metrics().record_flow("payout_accounts");
        m_context->metrics().counter("payouts_accounts_total").increment();
        (void)app::http::RequestQuery{}(req).get("merchant_id");

        const auto encoded = serde::Ser::encode_json(std::string{"payouts.payouts_accounts"});
        send_json(res, send, std::format(R"({{"operation":{}}})", encoded), true);
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
        const auto query = app::http::RequestQuery{}(req);

        auto text = serde::Ser::encode_json(filters_from_records(
            query.get("profile_id").value_or(""), time_range_from_query(query)));
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
        const auto query = app::http::RequestQuery{}(req);

        auto text = serde::Ser::encode_json(filters_from_records(
            resolve_profile_id(req, std::nullopt), time_range_from_query(query)));
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

        const auto body = req.body_as_string();
        if (!body.empty()) {
            auto parsed = serde::Ser::decode_generic("application/json", body);
            if (!parsed) {
                send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
        }

        m_context->metrics().record_flow("get_payouts_aggregates");
        m_context->metrics().counter("payouts_aggregates_total").increment();
        const auto query = app::http::RequestQuery{}(req);

        app::model::payouts::PayoutsAggregateResponse response{};
        std::unordered_map<std::string, std::int64_t> counts{};
        for (const auto& record : scoped_records(
                 query.get("profile_id").value_or(""), time_range_from_query(query))) {
            ++counts[record.get_status()];
        }
        response.set_status_with_count(std::move(counts));

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

        const auto body = req.body_as_string();
        if (!body.empty()) {
            auto parsed = serde::Ser::decode_generic("application/json", body);
            if (!parsed) {
                send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
        }

        m_context->metrics().record_flow("get_payouts_aggregates_profile");
        m_context->metrics().counter("payouts_aggregates_profile_total").increment();
        const auto query = app::http::RequestQuery{}(req);

        app::model::payouts::PayoutsAggregateResponse response{};
        std::unordered_map<std::string, std::int64_t> counts{};
        for (const auto& record : scoped_records(
                 resolve_profile_id(req, std::nullopt), time_range_from_query(query))) {
            ++counts[record.get_status()];
        }
        response.set_status_with_count(std::move(counts));

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
        const auto attempt_status = input.get_status() ? input.get_status()->get_value() : std::string{};
        if (!attempt_status.empty()) {
            record.set_status(attempt_status);
        }
        m_context->repository().update_payout(record);

        const std::string attempt_id = input.get_payout_attempt_id();
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
            } else {
                record_payout_attempt(
                    record.get_payout_id(), record.get_merchant_id(),
                    input.get_connector_payout_id().value_or(""),
                    attempt_status, record.get_amount(), record.get_currency(),
                    input.get_error_code().value_or(""),
                    input.get_error_message().value_or(""));
            }
        }
        emit_payout_webhook("manual_updated", record);

        app::model::payouts::PayoutsManualUpdateResponse response{};
        app::model::payouts::PayoutIdValue payout_value{};
        payout_value.set_value(record.get_payout_id());
        response.set_payout_id(payout_value);
        response.set_payout_attempt_id(input.get_payout_attempt_id());
        response.set_merchant_id(input.get_merchant_id());
        if (input.get_status()) {
            response.set_attempt_status(*input.get_status());
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
                Route{"/payouts_accounts"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payouts_accounts(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payouts_accounts")
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

    [[nodiscard]] core::error::AppError error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
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
        std::string_view note
    ) const
    {
        return std::format(
            R"({{"profile_id":"{}","browser_info":"{}","payout_link_id":"{}","payout_link_status":"{}","note":"{}"}})",
            json_escape(profile_id),
            json_escape(browser_info),
            json_escape(payout_link_id),
            json_escape(payout_link_status),
            json_escape(note));
    }

    [[nodiscard]] std::string merge_meta_profile(std::string_view meta, std::string_view profile_id) const
    {
        return encode_meta(
            profile_id,
            meta_value(meta, "browser_info"),
            meta_value(meta, "payout_link_id"),
            meta_value(meta, "payout_link_status"),
            meta_value(meta, "note"));
    }

    [[nodiscard]] std::string merge_meta_payout_link(std::string_view meta, std::string_view link_status) const
    {
        return encode_meta(
            meta_value(meta, "profile_id"),
            meta_value(meta, "browser_info"),
            meta_value(meta, "payout_link_id"),
            link_status,
            meta_value(meta, "note"));
    }

    [[nodiscard]] std::string merge_meta_note(std::string_view meta, std::string_view note) const
    {
        return encode_meta(
            meta_value(meta, "profile_id"),
            meta_value(meta, "browser_info"),
            meta_value(meta, "payout_link_id"),
            meta_value(meta, "payout_link_status"),
            note);
    }

    [[nodiscard]] std::string browser_info_from_headers(interfaces::io::IRequest& req) const
    {
        const auto user_agent = app::http::HeaderReader{}.get(req, "user-agent");
        const auto locale = app::http::HeaderReader{}.get(req, "accept-language");
        const auto ip = app::http::HeaderReader{}.get(req, "x-forwarded-for");
        return std::format("ua={}|locale={}|ip={}", user_agent, locale, ip);
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
        ConnectorChoice choice{};
        (void)payout_id;
        const auto context_json = std::format(
            R"({{"amount":"{}","currency":"{}","payment_method":"{}","country":"","connector":"{}"}})",
            amount, currency, payout_type, requested_connector);

        auto program = m_context->superposition().resolve(
            "payouts", "connector_decision", merchant_id);
        const std::string program_json = (program.has_value() && !program->get_value().empty())
                                             ? program->get_value()
                                             : std::string{
                                                 R"({"rules":[],"default_action":"allow","default_connector":""})"};
        const auto decision = m_context->decision_engine().evaluate(program_json, context_json);
        choice.decision_action = decision.get_action();
        choice.decision_connector = decision.get_connector_name();

        if (!decision.get_error_kind().empty()) {
            choice.error = error(core::error::AppErrorKind::VALIDATION_FAILED,
                                 decision.get_reason().empty()
                                     ? std::string{"invalid payout connector decision program"}
                                     : decision.get_reason());
            return choice;
        }
        if (decision.get_action() == "deny" || decision.get_action() == "error") {
            choice.error = error(core::error::AppErrorKind::PRECONDITION_FAILED,
                                 decision.get_reason().empty()
                                     ? std::string{"connector decision denied the payout"}
                                     : decision.get_reason());
            return choice;
        }

        request.set_payment_method_type(std::string{payout_type});
        auto connector = m_context->connector_router().select_for(request);
        if (!connector) {
            const auto last = m_context->connector_router().last_error();
            // An empty router is the normal local configuration: keep the payout pending
            // instead of failing. Real policy failures (kill switch, rollout) stay typed.
            if (last.has_value() && last->get_message() != "no connector registered") {
                choice.error = *last;
            } else {
                m_context->metrics().counter("payouts_connector_unavailable").increment();
            }
            return choice;
        }
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

    [[nodiscard]] bool supports_instant_payout(std::string_view connector_name) const
    {
        if (connector_name.empty()) {
            return true;
        }
        const auto configured = m_context->config_store().get_value(
            "payout_supports_instant", connector_name);
        if (configured.has_value()) {
            return *configured == "true" || *configured == "1";
        }
        return connector_name == "local";
    }

    [[nodiscard]] bool is_confirm_disallowed(std::string_view status) const
    {
        const auto lower = to_lower(status);
        const std::array<std::string_view, 10> disallowed{
            "cancelled", "success", "succeeded", "failed",
            "ineligible", "notpermitted", "requiresfulfillment", "requiresvendoraccountcreation",
            "authorized", "fulfilled"};
        return std::find(disallowed.begin(), disallowed.end(), std::string_view{lower}) != disallowed.end();
    }

    [[nodiscard]] bool is_cancel_terminal(std::string_view status) const
    {
        const auto lower = to_lower(status);
        const std::array<std::string_view, 4> terminal{
            "cancelled", "success", "succeeded", "failed"};
        return std::find(terminal.begin(), terminal.end(), std::string_view{lower}) != terminal.end();
    }

    [[nodiscard]] bool is_fulfill_allowed(std::string_view status) const
    {
        const auto lower = to_lower(status);
        return lower == "pending" || lower == "authorized" ||
               lower == "requiresfulfillment" || lower.empty();
    }

    [[nodiscard]] std::string time_range_from_query(const app::http::QueryParams& query) const
    {
        const auto start = query.get("start_time");
        const auto end = query.get("end_time");
        if (!start.has_value() && !end.has_value()) {
            return {};
        }
        return std::format("{},{}", start.value_or(""), end.value_or(""));
    }

    [[nodiscard]] bool within_time_range(std::string_view created_at, std::string_view time_range) const
    {
        if (time_range.empty()) {
            return true;
        }
        const auto created = to_number(created_at);
        if (!created.has_value()) {
            return true;
        }
        const auto comma = time_range.find(',');
        const auto start_text = comma == std::string_view::npos
                                    ? time_range
                                    : time_range.substr(0, comma);
        const auto end_text = comma == std::string_view::npos
                                  ? std::string_view{}
                                  : time_range.substr(comma + 1);
        if (!start_text.empty()) {
            const auto start = to_number(start_text);
            if (start.has_value() && *created < *start) {
                return false;
            }
        }
        if (!end_text.empty()) {
            const auto end = to_number(end_text);
            if (end.has_value() && *created > *end) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::vector<app::store::payouts::PayoutRecord> scoped_records(
        std::string_view profile_id,
        std::string_view time_range
    ) const
    {
        std::vector<app::store::payouts::PayoutRecord> out{};
        for (const auto& record : m_context->repository().list_payouts()) {
            if (!profile_id.empty() &&
                meta_value(record.get_metadata(), "profile_id") != profile_id) {
                continue;
            }
            if (!within_time_range(record.get_created_at(), time_range)) {
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
        attempt.set_attempt_id(m_context->ids().payment_id("pa_"));
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

    void emit_payout_webhook(std::string_view event_type, const app::store::payouts::PayoutRecord& record) const
    {
        const auto topic = std::format("payout.{}", event_type);
        const auto payload = std::format(
            R"({{"payout_id":"{}","merchant_id":"{}","status":"{}","event_type":"{}"}})",
            record.get_payout_id(), record.get_merchant_id(), record.get_status(), event_type);
        const auto event_id = m_context->events().publish(topic, payload);
        const auto endpoint = m_context->config_store().get_value(
            "payout_webhook_endpoint", record.get_merchant_id());
        if (endpoint.has_value() && !endpoint->empty()) {
            m_context->webhook_delivery().enqueue(event_id, *endpoint);
        }
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

    [[nodiscard]] app::model::payouts::PayoutListFiltersV2 filters_from_records(
        std::string_view profile_id,
        std::string_view time_range
    ) const
    {
        app::model::payouts::PayoutListFiltersV2 filters{};
        const auto records = scoped_records(profile_id, time_range);
        std::vector<app::model::payouts::PayoutStatusValue> statuses{};
        for (const auto& status : distinct_field(records, [](const auto& record) { return record.get_status(); })) {
            app::model::payouts::PayoutStatusValue value{};
            value.set_value(status);
            statuses.push_back(std::move(value));
        }
        filters.set_status(std::move(statuses));
        std::vector<app::model::payouts::CurrencyValue> currencies{};
        for (const auto& currency : distinct_field(records, [](const auto& record) { return record.get_currency(); })) {
            app::model::payouts::CurrencyValue value{};
            value.set_value(currency);
            currencies.push_back(std::move(value));
        }
        filters.set_currency(std::move(currencies));
        std::vector<app::model::payouts::PayoutTypeValue> methods{};
        for (const auto& payout_type : distinct_field(records, [](const auto& record) { return record.get_payout_type(); })) {
            app::model::payouts::PayoutTypeValue value{};
            value.set_value(payout_type);
            methods.push_back(std::move(value));
        }
        filters.set_payout_method(std::move(methods));
        return filters;
    }

    [[nodiscard]] app::model::payouts::PayoutListResponse list_response(
        std::string_view payout_id,
        std::string_view customer_id,
        std::string_view status,
        std::string_view currency,
        std::string_view profile_id,
        std::string_view time_range,
        std::size_t offset,
        std::size_t limit
    ) const
    {
        const auto records = scoped_records(profile_id, time_range);
        std::vector<app::model::payouts::PayoutCreateResponse> data{};
        std::size_t skipped{0};
        for (const auto& record : records) {
            if (!payout_id.empty() && record.get_payout_id() != payout_id) {
                continue;
            }
            if (!customer_id.empty() && record.get_customer_id() != customer_id) {
                continue;
            }
            if (!status.empty() && record.get_status() != status) {
                continue;
            }
            if (!currency.empty() && record.get_currency() != currency) {
                continue;
            }
            if (skipped++ < offset) {
                continue;
            }
            if (limit > 0 && data.size() >= limit) {
                break;
            }
            auto item = response_from_record(record);
            enrich_response(item, record, false);
            data.push_back(std::move(item));
        }
        app::model::payouts::PayoutListResponse response{};
        response.set_data(std::move(data));
        response.set_size(response.get_data().size());
        response.set_total_count(static_cast<std::int64_t>(records.size()));
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

    [[nodiscard]] std::string status_filter(
        const app::model::payouts::PayoutListFilterConstraints& input,
        const std::optional<std::string>& fallback
    )
    {
        if (input.get_status().has_value() && !input.get_status()->empty()) {
            return input.get_status()->front().get_value();
        }
        return fallback.value_or("");
    }

    [[nodiscard]] std::string currency_filter(
        const app::model::payouts::PayoutListFilterConstraints& input,
        const std::optional<std::string>& fallback
    )
    {
        if (input.get_currency().has_value() && !input.get_currency()->empty()) {
            return input.get_currency()->front().get_value();
        }
        return fallback.value_or("");
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
