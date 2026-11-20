module;
export module app_handlers:refunds;

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
import app_store;
import app_connector;
import app_http;
import app_crypto;
import app_metrics;
import app_locking;
import app_events;
import app_workflow;
import app_services;

export namespace app::handlers::refunds {

class RefundsRouteHandlers
{
    using Error = core::error::AppError;
    using Kind = core::error::AppErrorKind;
    using Record = app::store::refunds::RefundRecord;
    using Response = app::model::refunds::RefundResponse;
    using Status = app::model::refunds::RefundStatus;

public:
    void handle_refunds_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("refunds_create");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::refunds::RefundsCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }

        m_context->metrics().record_flow("refund_create");
        m_context->metrics().counter("refunds_create_total").increment();

        const auto path = app::http::RequestPath{}(req);
        const auto path_id = refund_id_from_path(req);
        const bool is_v2 = path.count() > 0 && path.at(0).value_or(std::string_view{}) == "v2";
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto header_merchant_id = app::http::merchant_id_from_header{}(req);

        const std::string payment_id{parsed->get_payment_id().get_value()};
        std::optional<std::int64_t> requested_amount{};
        if (parsed->get_amount()) {
            requested_amount = parse_amount(parsed->get_amount()->get_value());
        }
        std::string merchant_id = header_merchant_id;
        if (merchant_id.empty() && parsed->get_merchant_id()) {
            merchant_id = parsed->get_merchant_id()->get_value();
        }
        if (merchant_id.empty()) {
            merchant_id = auth_context.merchant_id();
        }
        const std::string profile_id = auth_context.profile_id();

        if (const auto invalid = validate_refund_against_payment(payment_id, requested_amount, merchant_id, profile_id)) {
            send_error(res, send, *invalid);
            return;
        }

        const auto lock = m_context->locks().acquire(
            std::string{"refund:"} + payment_id, std::int64_t{30}, "refund_create");
        if (!lock) {
            m_context->metrics().counter("refunds_lock_contended").increment();
            send_error(res, send, core::error::AppErrorBuilder{}(
                Kind::DUPLICATE_REQUEST, "a refund is already being processed for this payment"));
            return;
        }

        const std::int64_t captured_amount = captured_amount_for(payment_id).value_or(requested_amount.value_or(0));
        const std::int64_t amount = requested_amount.value_or(captured_amount);
        if (amount <= 0) {
            m_context->locks().release(std::string{"refund:"} + payment_id);
            send_error(res, send, core::error::AppErrorBuilder{}(
                Kind::INVALID_REQUEST, "refund amount must be positive"));
            return;
        }

        const std::string currency = currency_for(payment_id);
        Record record{};
        record.set_refund_id(path_id.value_or(m_context->ids().payment_id("rf_")));
        record.set_payment_id(payment_id);
        record.set_merchant_id(merchant_id);
        record.set_amount(amount);
        record.set_currency(currency);
        record.set_status("Pending");
        if (parsed->get_reason()) { record.set_reason(*parsed->get_reason()); }
        if (!profile_id.empty()) {
            m_context->config_store().put("refund_profile_scope", profile_id, merchant_id);
        }

        std::string creds_identifier{};
        if (parsed->get_merchant_connector_details()) {
            const auto value = parsed->get_merchant_connector_details()->get_value();
            m_context->config_store().put("merchant_connector_details", record.get_refund_id(), value);
            creds_identifier = extract_creds_identifier(value);
            if (!creds_identifier.empty()) {
                m_context->config_store().put("refund_creds_identifier", record.get_refund_id(), creds_identifier);
            }
        }

        if (const auto attempt = find_last_successful_attempt(payment_id)) {
            record.set_connector(attempt->get_connector());
        }
        m_context->cache().put(
            std::string{"refunds.idempotency."} + record.get_refund_id(),
            core::cache::to_cache_value(record.get_payment_id()));
        record = m_context->repository().save_refund(record);

        execute_refund(record, creds_identifier, parsed->get_return_raw_connector_response().value_or(false),
                       is_v2 ? "refund_v2" : "refund");
        m_context->repository().update_refund(record);
        m_context->locks().release(std::string{"refund:"} + payment_id);

        auto text = serde::Ser::encode_json(to_refund_response(record));
        send_json(res, send, text, true);
    }

    void handle_refunds_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("refunds_retrieve");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto path_id = refund_id_from_path(req);
        if (!path_id.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                Kind::INVALID_REQUEST, "missing refund id in path"));
            return;
        }
        const auto query = app::http::RequestQuery{}(req);
        const bool include_raw = query.get("return_raw_connector_response").value_or("false") == "true";

        m_context->metrics().record_flow("refund_retrieve");
        m_context->metrics().counter("refunds_retrieve_total").increment();

        const auto found = find_refund_record(*path_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::NOT_FOUND, "refund not found"));
            return;
        }
        auto text = serde::Ser::encode_json(to_refund_response(*found, include_raw));
        send_json(res, send, text, true);
    }

    void handle_refunds_retrieve_with_gateway_creds(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("refunds_retrieve_with_gateway_creds");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::refunds::RefundsRetrieveRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto path_id = refund_id_from_path(req);
        if (!path_id.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                Kind::INVALID_REQUEST, "missing refund id in path"));
            return;
        }
        const auto query = app::http::RequestQuery{}(req);
        const bool force_sync = parsed->get_force_sync().value_or(
            query.get("force_sync").value_or("false") == "true");

        m_context->metrics().record_flow("refund_retrieve_gateway");
        m_context->metrics().counter("refunds_gateway_sync_total").increment();

        const auto found = find_refund_record(*path_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::NOT_FOUND, "refund not found"));
            return;
        }
        Record record = *found;

        std::string creds_identifier{};
        if (parsed->get_merchant_connector_details()) {
            const auto value = parsed->get_merchant_connector_details()->get_value();
            m_context->config_store().put("merchant_connector_details", record.get_refund_id(), value);
            creds_identifier = extract_creds_identifier(value);
        }
        if (creds_identifier.empty()) {
            creds_identifier = m_context->config_store()
                .get_value("refund_creds_identifier", record.get_refund_id()).value_or(std::string{});
        }
        if (force_sync) {
            execute_refund(record, creds_identifier, parsed->get_force_sync().value_or(false), "sync");
            m_context->repository().update_refund(record);
        }
        auto text = serde::Ser::encode_json(to_refund_response(record, true));
        send_json(res, send, text, true);
    }

    void handle_refunds_retrieve_with_body(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("refunds_retrieve_with_body");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::refunds::RefundsRetrieveBody;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        const auto refund_id = query.get("refund_id");
        const bool force_sync = parsed->get_force_sync().value_or(
            query.get("force_sync").value_or("false") == "true");
        const bool all_keys_required = parsed->get_all_keys_required().value_or(
            query.get("all_keys_required").value_or("false") == "true");

        m_context->metrics().record_flow("refund_sync");
        m_context->metrics().counter("refunds_sync_total").increment();

        std::optional<Record> found{};
        if (refund_id.has_value()) { found = find_refund_record(*refund_id); }
        Record record{};
        if (found.has_value()) {
            record = *found;
        } else {
            const auto records = m_context->repository().list_refunds();
            if (!records.empty()) { record = records.front(); }
        }
        if (record.get_refund_id().empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::NOT_FOUND, "refund not found"));
            return;
        }

        if (should_call_refund(force_sync, all_keys_required)) {
            derive_split_refunds(record.get_payment_id());
            const auto creds_identifier = m_context->config_store()
                .get_value("refund_creds_identifier", record.get_refund_id()).value_or(std::string{});
            execute_refund(record, creds_identifier, all_keys_required, "sync");
            m_context->repository().update_refund(record);
        } else {
            m_context->metrics().counter("refunds_sync_skipped").increment();
        }
        auto text = serde::Ser::encode_json(to_refund_response(record, true));
        send_json(res, send, text, true);
    }

    void handle_refunds_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("refunds_update");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::refunds::RefundUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto path_id = refund_id_from_path(req);
        const std::string refund_id = path_id.value_or(parsed->get_refund_id());
        m_context->metrics().record_flow("refund_update");
        m_context->metrics().counter("refunds_update_total").increment();

        const auto found = find_refund_record(refund_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::NOT_FOUND, "refund not found"));
            return;
        }
        Record record = *found;
        m_context->cache().put(
            std::string{"refunds.updated."} + record.get_refund_id(),
            core::cache::to_cache_value(record.get_status()), core::cache::Ttl{}.seconds(300));
        if (parsed->get_reason()) { record.set_reason(*parsed->get_reason()); }
        if (parsed->get_metadata()) {
            m_context->config_store().put("refund_metadata", record.get_refund_id(),
                                          parsed->get_metadata()->get_value());
        }
        m_context->repository().update_refund(record);

        auto text = serde::Ser::encode_json(to_refund_response(record));
        send_json(res, send, text, true);
    }

    void handle_refunds_reverse(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("refunds_reverse");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::refunds::RefundReverseRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto path_id = refund_id_from_path(req);
        const std::string refund_id = path_id.value_or(parsed->get_refund_id());
        m_context->metrics().record_flow("refund_reverse");
        m_context->metrics().counter("refunds_reverse_total").increment();

        const auto found = find_refund_record(refund_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::NOT_FOUND, "refund not found"));
            return;
        }
        Record record = *found;
        if (status_from_text(record.get_status()) != Status::Succeeded) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                Kind::PRECONDITION_FAILED, "only a succeeded refund can be reversed"));
            return;
        }

        m_context->cache().put(
            std::string{"refunds.reversing."} + record.get_refund_id(),
            core::cache::to_cache_value(record.get_payment_id()), core::cache::Ttl{}.seconds(300));

        const auto intent = m_context->repository().find_payment_by_string(record.get_payment_id());
        const auto attempt = find_last_successful_attempt(record.get_payment_id());
        std::string void_metadata = std::format(
            "{{\"void_post_refund\":\"true\",\"refund_id\":\"{}\",\"payment_id\":\"{}\"",
            record.get_refund_id(), record.get_payment_id());
        if (attempt.has_value()) { void_metadata += std::format(",\"attempt_id\":\"{}\"", attempt->get_attempt_id()); }
        if (intent.has_value()) { void_metadata += std::format(",\"merchant_id\":\"{}\"", intent->get_merchant_id()); }
        void_metadata.push_back('}');
        m_context->config_store().put("refund_void_post_metadata", record.get_refund_id(), void_metadata);

        const auto creds_identifier = m_context->config_store()
            .get_value("refund_creds_identifier", record.get_refund_id()).value_or(std::string{});
        execute_refund(record, creds_identifier, false, "cancel");
        m_context->repository().update_refund(record);

        auto text = serde::Ser::encode_json(to_refund_response(record));
        send_json(res, send, text, true);
    }

    void handle_refunds_metadata_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("refunds_metadata_update");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::refunds::RefundMetadataUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto path_id = refund_id_from_path(req);
        if (!path_id.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                Kind::INVALID_REQUEST, "missing refund id in path"));
            return;
        }
        m_context->metrics().record_flow("refund_metadata_update");
        m_context->metrics().counter("refunds_metadata_update_total").increment();

        const auto found = find_refund_record(*path_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::NOT_FOUND, "refund not found"));
            return;
        }
        Record record = *found;
        if (parsed->get_reason()) { record.set_reason(*parsed->get_reason()); }
        if (parsed->get_metadata()) {
            m_context->config_store().put("refund_metadata", record.get_refund_id(),
                                          parsed->get_metadata()->get_value());
        }
        m_context->repository().update_refund(record);

        auto text = serde::Ser::encode_json(to_refund_response(record));
        send_json(res, send, text, true);
    }

    void handle_refunds_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("refunds_list");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::refunds::RefundListRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        auto filter = filter_from_query(query);
        apply_list_request(*parsed, filter);

        m_context->metrics().record_flow("refund_list");
        m_context->metrics().counter("refunds_list_total").increment();

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto records = scoped_records(
            apply_refund_filter(m_context->repository().list_refunds(), filter),
            auth_context, req, false);
        auto text = serde::Ser::encode_json(to_list_response(records));
        send_json(res, send, text, true);
    }

    void handle_refunds_list_for_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("refunds_list_for_platform");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto query = app::http::RequestQuery{}(req);
        auto filter = filter_from_query(query);
        if (filter.merchant_id.empty()) {
            filter.merchant_id = app::http::merchant_id_from_header{}(req);
        }

        m_context->metrics().record_flow("refund_platform_list");
        m_context->metrics().counter("refunds_platform_list_total").increment();

        const auto records = apply_refund_filter(m_context->repository().list_refunds(), filter);
        std::vector<app::model::refunds::PlatformRefundListItem> data{};
        data.reserve(records.size());
        for (const auto& record : records) { data.push_back(to_platform_item(record)); }
        using Response = app::model::refunds::PlatformRefundListResponse;
        Response response{};
        response.set_count(data.size());
        response.set_total_count(static_cast<std::int64_t>(data.size()));
        response.set_data(std::move(data));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_refunds_filter_list_for_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("refunds_filter_list_for_platform");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto query = app::http::RequestQuery{}(req);
        auto filter = filter_from_query(query);
        if (filter.merchant_id.empty()) {
            filter.merchant_id = app::http::merchant_id_from_header{}(req);
        }

        m_context->metrics().record_flow("refund_platform_filter");
        m_context->metrics().counter("refunds_platform_filter_total").increment();

        const auto records = apply_refund_filter(m_context->repository().list_refunds(), filter);
        const auto response = platform_filter_metadata(records);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_refunds_list_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("refunds_list_profile");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::refunds::RefundListRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        auto filter = filter_from_query(query);
        apply_list_request(*parsed, filter);

        m_context->metrics().record_flow("refund_profile_list");
        m_context->metrics().counter("refunds_profile_list_total").increment();

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto records = scoped_records(
            apply_refund_filter(m_context->repository().list_refunds(), filter),
            auth_context, req, true);
        auto text = serde::Ser::encode_json(to_list_response(records));
        send_json(res, send, text, true);
    }

    void handle_refunds_filter_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("refunds_filter_list");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::refunds::RefundListFilters;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        const auto filter = filter_from_query(query);
        m_context->metrics().record_flow("refund_filter_list");
        m_context->metrics().counter("refunds_filter_list_total").increment();

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto records = scoped_records(
            apply_refund_filter(m_context->repository().list_refunds(), filter),
            auth_context, req, false);
        const auto response = filter_metadata(records);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_get_refunds_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_refunds_filters");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto query = app::http::RequestQuery{}(req);
        m_context->metrics().record_flow("refund_filters");
        m_context->metrics().counter("refunds_filters_total").increment();

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto records = scoped_records(
            apply_refund_filter(m_context->repository().list_refunds(), filter_from_query(query)),
            auth_context, req, false);
        const auto response = filter_list_metadata(records);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_get_refunds_filters_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_refunds_filters_profile");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto query = app::http::RequestQuery{}(req);
        m_context->metrics().record_flow("refund_profile_filters");
        m_context->metrics().counter("refunds_profile_filters_total").increment();

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto records = scoped_records(
            apply_refund_filter(m_context->repository().list_refunds(), filter_from_query(query)),
            auth_context, req, true);
        const auto response = filter_metadata(records);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_get_refunds_aggregates(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_refunds_aggregates");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto query = app::http::RequestQuery{}(req);
        m_context->metrics().record_flow("refund_aggregates");
        m_context->metrics().counter("refunds_aggregates_total").increment();

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto records = scoped_records(
            apply_refund_filter(m_context->repository().list_refunds(), filter_from_query(query)),
            auth_context, req, false);
        auto text = serde::Ser::encode_json(aggregate_response(records, query));
        send_json(res, send, text, true);
    }

    void handle_refunds_manual_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("refunds_manual_update");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::refunds::RefundManualUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto path_id = refund_id_from_path(req);
        const std::string refund_id = path_id.value_or(parsed->get_refund_id());
        m_context->metrics().record_flow("refund_manual_update");
        m_context->metrics().counter("refunds_manual_update_total").increment();

        const auto found = find_refund_record(refund_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}(Kind::NOT_FOUND, "refund not found"));
            return;
        }
        Record record = *found;
        if (parsed->get_connector_refund_id()) {
            m_context->cache().put(
                std::string{"refund.connector_refund_id."} + record.get_refund_id(),
                core::cache::to_cache_value(*parsed->get_connector_refund_id()),
                core::cache::Ttl{}.seconds(600));
        }
        if (parsed->get_status()) { record.set_status(std::string{status_to_text(*parsed->get_status())}); }
        std::string error_meta{};
        if (parsed->get_error_code()) { error_meta = *parsed->get_error_code(); }
        error_meta.push_back('|');
        if (parsed->get_error_message()) { error_meta += *parsed->get_error_message(); }
        if (!error_meta.empty()) {
            m_context->cache().put(std::string{"refunds.error."} + record.get_refund_id(),
                core::cache::to_cache_value(error_meta), core::cache::Ttl{}.seconds(300));
        }
        m_context->repository().update_refund(record);

        auto text = serde::Ser::encode_json(to_refund_response(record));
        send_json(res, send, text, true);
    }

    void handle_get_refunds_aggregate_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_refunds_aggregate_profile");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto query = app::http::RequestQuery{}(req);
        m_context->metrics().record_flow("refund_profile_aggregates");
        m_context->metrics().counter("refunds_profile_aggregates_total").increment();

        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto records = scoped_records(
            apply_refund_filter(m_context->repository().list_refunds(), filter_from_query(query)),
            auth_context, req, true);
        auto text = serde::Ser::encode_json(aggregate_response(records, query));
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        Router{router, "/refunds"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_create")
            )
            .add_route(
                Route{"/sync"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_retrieve_with_body(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_retrieve_with_body")
            )
            .add_route(
                Route{"/:id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_retrieve")
            )
            .add_route(
                Route{"/:id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_update")
            )
            .add_route(
                Route{"/:id/reverse"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_reverse(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_reverse")
            )
            .add_route(
                Route{"/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_list")
            )
            .add_route(
                Route{"/platform/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_list_for_platform(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_list_for_platform")
            )
            .add_route(
                Route{"/platform/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_filter_list_for_platform(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_filter_list_for_platform")
            )
            .add_route(
                Route{"/profile/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_list_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_list_profile")
            )
            .add_route(
                Route{"/filter"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_filter_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_filter_list")
            )
            .add_route(
                Route{"/v2/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_refunds_filters(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_refunds_filters")
            )
            .add_route(
                Route{"/aggregate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_refunds_aggregates(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_refunds_aggregates")
            )
            .add_route(
                Route{"/profile/aggregate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_refunds_aggregate_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_refunds_aggregate_profile")
            )
            .add_route(
                Route{"/v2/profile/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_refunds_filters_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_refunds_filters_profile")
            )
            .add_route(
                Route{"/:id/manual-update"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_manual_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_manual_update")
            );

        Router{router, "/v2/refunds"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_create")
            )
            .add_route(
                Route{"/:id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_retrieve")
            )
            .add_route(
                Route{"/:id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_retrieve_with_gateway_creds(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_retrieve_with_gateway_creds")
            )
            .add_route(
                Route{"/:id/update-metadata"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_metadata_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_metadata_update")
            )
            .add_route(
                Route{"/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_refunds_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch refunds_list")
            );
    }

    explicit RefundsRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    /** @brief Extract the :id segment that follows "refunds" in v1/v2 paths. */
    [[nodiscard]] std::optional<std::string> refund_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i{0}; i + 1 < segments.count(); ++i) {
            const auto current = segments.at(i).value_or(std::string_view{});
            if (current == "refunds") {
                const auto candidate = segments.at(i + 1);
                if (candidate.has_value() && !candidate->empty() && *candidate != "v2" &&
                    *candidate != "list" && *candidate != "sync" && *candidate != "filter" &&
                    *candidate != "aggregate" && *candidate != "platform" && *candidate != "profile") {
                    return std::string{*candidate};
                }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string lowercase(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char ch : value) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return out;
    }

    [[nodiscard]] std::optional<std::int64_t> parse_amount(std::string_view raw) const
    {
        if (raw.empty()) { return std::nullopt; }
        std::int64_t amount{};
        const auto [end, ec] = std::from_chars(raw.data(), raw.data() + raw.size(), amount);
        (void)end;
        if (ec != std::errc{}) { return std::nullopt; }
        return amount;
    }

    [[nodiscard]] std::int64_t parse_amount_from_metadata(std::string_view metadata, std::string_view key) const
    {
        const std::string pattern = std::format("\"{}\"", key);
        const auto pos = metadata.find(pattern);
        if (pos == std::string_view::npos) { return 0; }
        auto cursor = pos + pattern.size();
        while (cursor < metadata.size() && (metadata[cursor] == ':' || std::isspace(static_cast<unsigned char>(metadata[cursor])))) {
            ++cursor;
        }
        return parse_amount(metadata.substr(cursor)).value_or(0);
    }

    [[nodiscard]] std::string extract_creds_identifier(std::string_view value) const
    {
        constexpr std::string_view key{"\"creds_identifier\""};
        const auto pos = value.find(key);
        if (pos == std::string_view::npos) { return {}; }
        auto cursor = pos + key.size();
        while (cursor < value.size() && (value[cursor] == ':' || std::isspace(static_cast<unsigned char>(value[cursor])))) {
            ++cursor;
        }
        if (cursor >= value.size() || value[cursor] != '"') { return {}; }
        ++cursor;
        const auto end = value.find('"', cursor);
        if (end == std::string_view::npos) { return {}; }
        return std::string{value.substr(cursor, end - cursor)};
    }

    [[nodiscard]] std::optional<app::store::payments::PaymentIntentRecord>
    find_payment_intent(std::string_view payment_id) const
    {
        if (payment_id.empty()) { return std::nullopt; }
        return m_context->repository().find_payment_by_string(payment_id);
    }

    [[nodiscard]] std::optional<std::int64_t> captured_amount_for(std::string_view payment_id) const
    {
        const auto intent = find_payment_intent(payment_id);
        if (!intent) { return std::nullopt; }
        return intent->get_amount();
    }

    [[nodiscard]] std::string currency_for(std::string_view payment_id) const
    {
        const auto intent = find_payment_intent(payment_id);
        return intent ? intent->get_currency() : std::string{};
    }

    [[nodiscard]] std::optional<app::store::payment_attempts::PaymentAttemptRecord>
    find_last_successful_attempt(std::string_view payment_id) const
    {
        std::optional<app::store::payment_attempts::PaymentAttemptRecord> best{};
        for (const auto& attempt : m_context->repository().list_payment_attempts()) {
            if (std::string_view{attempt.get_payment_id()} != payment_id) { continue; }
            const auto& status = attempt.get_status();
            const bool success = status == "Succeeded" || status == "succeeded" || status == "success" ||
                                 status == "captured" || status == "partially_captured" || status == "charged";
            if (!success) { continue; }
            if (!best.has_value() || attempt.get_created_at() > best->get_created_at()) { best = attempt; }
        }
        return best;
    }

    [[nodiscard]] std::optional<Error> validate_refund_against_payment(
        std::string_view payment_id,
        std::optional<std::int64_t> requested_amount,
        std::string_view merchant_id,
        std::string_view profile_id
    ) const
    {
        const auto intent = find_payment_intent(payment_id);
        if (!intent) {
            return core::error::AppErrorBuilder{}(Kind::NOT_FOUND, "payment intent not found");
        }
        const auto& status = intent->get_status();
        const bool refundable = status == "Succeeded" || status == "succeeded" || status == "success" ||
                                status == "PartiallyCaptured" || status == "partially_captured";
        if (!refundable) {
            return core::error::AppErrorBuilder{}(
                Kind::PRECONDITION_FAILED, "payment intent is not in a refundable state");
        }
        if (!profile_id.empty()) {
            const auto scoped = m_context->config_store().get_value("refund_profile_scope", profile_id);
            if (scoped.has_value() && *scoped != intent->get_merchant_id()) {
                return core::error::AppErrorBuilder{}(
                    Kind::FORBIDDEN, "profile is not authorized for this payment");
            }
        }
        if (!merchant_id.empty() && !intent->get_merchant_id().empty() && merchant_id != intent->get_merchant_id()) {
            return core::error::AppErrorBuilder{}(
                Kind::FORBIDDEN, "merchant is not authorized for this payment");
        }
        const auto& metadata = intent->get_metadata();
        if (metadata.find("post_capture_void") != std::string_view::npos) {
            return core::error::AppErrorBuilder{}(
                Kind::PRECONDITION_FAILED,
                "refund void cannot be performed after a post-capture void has been issued");
        }
        const std::int64_t captured = intent->get_amount();
        const std::int64_t blocked = parse_amount_from_metadata(metadata, "blocked_amount");
        const std::int64_t requested = requested_amount.value_or(captured);
        if (requested <= 0) {
            return core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "refund amount must be positive");
        }
        if (blocked + requested > captured) {
            return core::error::AppErrorBuilder{}(
                Kind::PRECONDITION_FAILED, "refund amount exceeds the captured amount");
        }
        return std::nullopt;
    }

    [[nodiscard]] bool should_call_refund(bool force_sync, bool all_keys_required) const
    {
        return force_sync || all_keys_required;
    }

    void derive_split_refunds(std::string_view payment_id) const
    {
        const auto siblings = m_context->repository().find_refunds_by_payment_id(payment_id);
        if (siblings.size() < 2) { return; }
        const std::string payload = std::format(
            "{{\"payment_id\":\"{}\",\"split_refund_count\":{}}}", payment_id, siblings.size());
        m_context->events().publish("refund.split_request_derived", payload);
        m_context->metrics().counter("refunds_split_requests_derived").increment();
    }

    void notify_refund(const Record& record, std::string_view flow) const
    {
        const std::string payload = std::format(
            "{{\"refund_id\":\"{}\",\"payment_id\":\"{}\",\"merchant_id\":\"{}\",\"status\":\"{}\",\"flow\":\"{}\"}}",
            record.get_refund_id(), record.get_payment_id(), record.get_merchant_id(),
            record.get_status(), flow);
        const std::string topic = std::format("refund.{}", lowercase(record.get_status()));
        const std::string event_id = m_context->events().publish(topic, payload);
        const std::string endpoint = m_context->config_store()
            .get_value("refund_webhook_endpoint", record.get_merchant_id())
            .value_or(std::string{"local://refund-webhook/"} + record.get_merchant_id());
        m_context->webhook_delivery().enqueue(event_id, endpoint);
        m_context->metrics().counter("refunds_webhook_notified").increment();
    }

    void apply_connector_response(
        Record& record,
        const app::connector::ConnectorResponse& response,
        std::string_view flow
    ) const
    {
        const auto& raw_status = response.get_status();
        if (raw_status.empty()) {
            m_context->metrics().record_flow("refund.status.no_change");
            m_context->metrics().counter("refunds_status_no_change").increment();
            return;
        }
        const Status mapped = status_from_text(raw_status);

        if (mapped == Status::Succeeded && response.get_reference_id().empty()) {
            record.set_status("Review");
            record.set_reason("IE:Integrity Check Failed");
            m_context->cache().put(
                std::string{"refunds.error."} + record.get_refund_id(),
                core::cache::to_cache_value(std::string{"IE|Integrity Check Failed|"}),
                core::cache::Ttl{}.seconds(300));
            m_context->metrics().counter("refunds_integrity_check_failed").increment();
            notify_refund(record, std::string{flow} + ".integrity_failed");
            return;
        }

        record.set_status(std::string{status_to_text(mapped)});
        if (!response.get_reference_id().empty()) {
            m_context->cache().put(
                std::string{"refund.connector_refund_id."} + record.get_refund_id(),
                core::cache::to_cache_value(response.get_reference_id()),
                core::cache::Ttl{}.seconds(600));
        }
        m_context->cache().put(
            std::string{"refund.raw_connector_response."} + record.get_refund_id(),
            core::cache::to_cache_value(response.get_raw_json()),
            core::cache::Ttl{}.seconds(600));

        std::string error_meta{};
        if (mapped == Status::Failed) {
            const bool timeout = lowercase(response.get_message()).find("timeout") != std::string::npos;
            record.set_reason(timeout ? std::string{} : response.get_message());
            if (!timeout) {
                error_meta = std::string{"AE|Connector declined|"} + response.get_message();
            }
            m_context->metrics().counter("refunds_connector_failure").increment();
        }
        m_context->cache().put(
            std::string{"refunds.error."} + record.get_refund_id(),
            core::cache::to_cache_value(error_meta), core::cache::Ttl{}.seconds(300));

        notify_refund(record, flow);
        if (mapped == Status::Succeeded) {
            const std::string payload = std::format(
                "{{\"refund_id\":\"{}\",\"payment_id\":\"{}\"}}",
                record.get_refund_id(), record.get_payment_id());
            m_context->events().publish("offer_engine.refund_notification", payload);
            m_context->metrics().counter("refunds_offer_engine_notified").increment();
        }
    }

    void execute_refund(
        Record& record,
        std::string_view creds_identifier,
        bool all_keys_required,
        std::string_view operation
    ) const
    {
        app::connector::ConnectorRequest request{};
        request.set_operation(std::string{operation});
        request.set_merchant_id(record.get_merchant_id());
        request.set_amount(record.get_amount());
        request.set_currency(record.get_currency());
        request.set_resource_id(record.get_payment_id());
        std::string raw = std::format(
            "{{\"flow\":\"{}\",\"refund_id\":\"{}\",\"all_keys_required\":{}}}",
            operation, record.get_refund_id(), all_keys_required ? "true" : "false");
        if (!creds_identifier.empty()) {
            raw += std::format(",\"creds_identifier\":\"{}\"", creds_identifier);
        }
        request.set_raw_payload(std::move(raw));

        auto connector = m_context->connector_router().select_for(request);
        if (!connector) {
            connector = m_context->connector_router().select(record.get_merchant_id());
        }
        if (!connector) {
            m_context->metrics().record_flow("refund.execute.no_connector");
            m_context->metrics().counter("refunds_execute_no_connector").increment();
            return;
        }
        const std::string name{connector->name()};
        record.set_connector(name);
        if (m_context->config_store().is_killed(name)) {
            m_context->metrics().counter("refunds_connector_killed").increment();
        }
        if (m_context->config_store().is_shadow_killed(name)) {
            m_context->metrics().counter("refunds_connector_shadow_killed").increment();
        }

        const auto tracker_id = m_context->process_trackers().schedule(
            std::string{"refund_"} + std::string{operation},
            std::format("{{\"refund_id\":\"{}\",\"connector\":\"{}\"}}", record.get_refund_id(), name),
            "0");
        app::connector::ConnectorResponse response{};
        if (operation == "cancel") {
            response = connector->cancel(request);
        } else if (operation == "sync") {
            response = connector->sync(request);
        } else {
            response = connector->refund(request);
        }
        apply_connector_response(record, response, operation);
        m_context->process_trackers().mark_terminal(tracker_id, record.get_status());
        m_context->metrics().record_flow(std::string{"refund.execute."} + std::string{operation});
    }

    [[nodiscard]] std::optional<Record> find_refund_record(std::string_view refund_id) const
    {
        if (refund_id.empty()) { return std::nullopt; }
        for (const auto& record : m_context->repository().list_refunds()) {
            if (std::string_view{record.get_refund_id()} == refund_id) { return record; }
        }
        return std::nullopt;
    }

    [[nodiscard]] Status status_from_text(std::string_view text) const
    {
        if (text == "Succeeded" || text == "succeeded" || text == "success" ||
            text == "successful" || text == "refunded") {
            return Status::Succeeded;
        }
        if (text == "Failed" || text == "failed" || text == "failure" || text == "error") {
            return Status::Failed;
        }
        if (text == "Review" || text == "review" || text == "ManualReview" || text == "manual_review") {
            return Status::Review;
        }
        return Status::Pending;
    }

    [[nodiscard]] std::string_view status_to_text(Status status) const
    {
        switch (status) {
        case Status::Succeeded: return "Succeeded";
        case Status::Failed: return "Failed";
        case Status::Review: return "Review";
        case Status::Pending:
        default: return "Pending";
        }
    }

    [[nodiscard]] Response to_refund_response(const Record& record, bool include_raw = false) const
    {
        Response response{};
        response.set_refund_id(record.get_refund_id());
        app::model::refunds::PaymentIdValue payment_value{};
        payment_value.set_value(record.get_payment_id());
        response.set_payment_id(payment_value);
        app::model::refunds::MinorUnitValue amount_value{};
        amount_value.set_value(std::to_string(record.get_amount()));
        response.set_amount(amount_value);
        response.set_currency(record.get_currency());
        response.set_status(status_from_text(record.get_status()));
        if (!record.get_reason().empty()) { response.set_reason(record.get_reason()); }
        response.set_connector(record.get_connector());

        const auto connector_refund = m_context->cache().get(
            std::string{"refund.connector_refund_id."} + record.get_refund_id()).value_or(std::string{});
        if (!connector_refund.empty()) { response.set_connector_refund_id(connector_refund); }

        const auto meta = m_context->cache().get(
            std::string{"refunds.error."} + record.get_refund_id()).value_or(std::string{});
        if (!meta.empty()) {
            const auto first = meta.find('|');
            const auto second = first == std::string::npos ? std::string::npos : meta.find('|', first + 1);
            const std::string code = first == std::string::npos ? meta : meta.substr(0, first);
            const std::string unified = first == std::string::npos ? std::string{}
                : (second == std::string::npos ? meta.substr(first + 1)
                                               : meta.substr(first + 1, second - first - 1));
            const std::string message = second == std::string::npos ? std::string{} : meta.substr(second + 1);
            if (!code.empty()) { response.set_error_code(code); }
            if (!unified.empty()) { response.set_unified_code(unified); }
            if (!message.empty()) { response.set_error_message(message); }
        }
        if (include_raw) {
            const auto raw = m_context->cache().get(
                std::string{"refund.raw_connector_response."} + record.get_refund_id()).value_or(std::string{});
            if (!raw.empty()) { response.set_raw_connector_response(raw); }
        }
        return response;
    }

    [[nodiscard]] app::model::refunds::RefundListResponse
    to_list_response(const std::vector<Record>& records) const
    {
        using ListResponse = app::model::refunds::RefundListResponse;
        ListResponse response{};
        std::vector<Response> data{};
        data.reserve(records.size());
        for (const auto& record : records) { data.push_back(to_refund_response(record)); }
        response.set_count(data.size());
        response.set_total_count(static_cast<std::int64_t>(data.size()));
        response.set_data(std::move(data));
        return response;
    }

    struct RefundFilter
    {
        std::string refund_id{};
        std::string payment_id{};
        std::string merchant_id{};
        std::string connector{};
        std::string currency{};
        std::string status{};
        std::string limit{};
        std::string offset{};
    };

    [[nodiscard]] RefundFilter filter_from_query(const app::http::QueryParams& query) const
    {
        RefundFilter filter{};
        const auto take = [&query](std::string_view key) {
            return query.get(key).value_or(std::string{});
        };
        filter.refund_id = take("refund_id");
        filter.payment_id = take("payment_id");
        filter.merchant_id = take("merchant_id");
        filter.connector = take("connector");
        filter.currency = take("currency");
        filter.status = take("refund_status");
        filter.limit = take("limit");
        filter.offset = take("offset");
        return filter;
    }

    void apply_list_request(const app::model::refunds::RefundListRequest& parsed, RefundFilter& filter) const
    {
        if (parsed.get_refund_id().has_value()) { filter.refund_id = *parsed.get_refund_id(); }
        if (parsed.get_payment_id().has_value()) { filter.payment_id = parsed.get_payment_id()->get_value(); }
        if (parsed.get_limit().has_value()) { filter.limit = parsed.get_limit()->get_value(); }
        if (!parsed.get_offset().get_value().empty()) { filter.offset = parsed.get_offset().get_value(); }
        if (parsed.get_connector().has_value() && !parsed.get_connector()->empty()) {
            filter.connector = parsed.get_connector()->front();
        }
        if (parsed.get_currency().has_value() && !parsed.get_currency()->empty()) {
            filter.currency = parsed.get_currency()->front().get_value();
        }
        if (parsed.get_refund_status().has_value() && !parsed.get_refund_status()->empty()) {
            filter.status = parsed.get_refund_status()->front().get_value();
        }
        if (parsed.get_profile_id().has_value() && !parsed.get_profile_id()->get_value().empty()) {
            const auto scoped = m_context->config_store()
                .get_value("refund_profile_scope", parsed.get_profile_id()->get_value());
            if (scoped.has_value()) { filter.merchant_id = *scoped; }
        }
    }

    [[nodiscard]] std::vector<Record>
    apply_refund_filter(std::vector<Record> records, const RefundFilter& filter) const
    {
        const auto matches = [](std::string_view needle, std::string_view value) {
            return needle.empty() || needle == value;
        };
        std::vector<Record> selected{};
        selected.reserve(records.size());
        for (const auto& record : records) {
            if (!matches(filter.refund_id, record.get_refund_id()) ||
                !matches(filter.payment_id, record.get_payment_id()) ||
                !matches(filter.merchant_id, record.get_merchant_id()) ||
                !matches(filter.connector, record.get_connector()) ||
                !matches(filter.currency, record.get_currency()) ||
                !matches(filter.status, record.get_status())) {
                continue;
            }
            selected.push_back(record);
        }

        std::size_t offset{0};
        if (!filter.offset.empty()) {
            const auto [end, ec] = std::from_chars(
                filter.offset.data(), filter.offset.data() + filter.offset.size(), offset);
            (void)end;
            if (ec != std::errc{}) { offset = 0; }
        }
        if (offset >= selected.size()) { return {}; }
        std::size_t count{selected.size() - offset};
        if (!filter.limit.empty()) {
            std::size_t requested{};
            const auto [end, ec] = std::from_chars(
                filter.limit.data(), filter.limit.data() + filter.limit.size(), requested);
            (void)end;
            if (ec == std::errc{} && requested < count) { count = requested; }
        }
        const auto first = selected.begin() + static_cast<std::ptrdiff_t>(offset);
        return std::vector<Record>{first, first + static_cast<std::ptrdiff_t>(count)};
    }

    [[nodiscard]] std::vector<Record> scoped_records(
        std::vector<Record> records,
        const app::auth::AuthContext& auth,
        interfaces::io::IRequest& req,
        bool profile_scoped
    ) const
    {
        std::string merchant = app::http::merchant_id_from_header{}(req);
        if (merchant.empty()) { merchant = auth.merchant_id(); }
        if (profile_scoped && !auth.profile_id().empty()) {
            const auto scoped = m_context->config_store().get_value("refund_profile_scope", auth.profile_id());
            if (scoped.has_value()) { merchant = *scoped; }
        }
        if (merchant.empty()) { return records; }
        std::vector<Record> out{};
        out.reserve(records.size());
        for (const auto& record : records) {
            if (record.get_merchant_id().empty() || record.get_merchant_id() == merchant) {
                out.push_back(record);
            }
        }
        return out;
    }

    [[nodiscard]] bool in_time_range(const Record& record, const app::http::QueryParams& query) const
    {
        const auto start = query.get("start_time");
        const auto end = query.get("end_time");
        if (!start.has_value() && !end.has_value()) { return true; }
        const auto& created = record.get_created_at();
        if (created.empty()) { return true; }
        if (start.has_value() && created < *start) { return false; }
        if (end.has_value() && created > *end) { return false; }
        return true;
    }

    [[nodiscard]] app::model::refunds::RefundAggregateResponse
    aggregate_response(const std::vector<Record>& records, const app::http::QueryParams& query) const
    {
        std::unordered_map<std::string, std::int64_t> counts{};
        for (const auto& record : records) {
            if (in_time_range(record, query)) { ++counts[record.get_status()]; }
        }
        app::model::refunds::RefundAggregateResponse response{};
        response.set_status_with_count(std::move(counts));
        return response;
    }

    [[nodiscard]] app::model::refunds::RefundListMetaData
    filter_list_metadata(const std::vector<Record>& records) const
    {
        std::vector<std::string> connectors{};
        std::vector<app::model::refunds::CurrencyValue> currencies{};
        std::vector<app::model::refunds::RefundStatusValue> statuses{};
        std::unordered_set<std::string> seen_connector{};
        std::unordered_set<std::string> seen_currency{};
        std::unordered_set<std::string> seen_status{};
        for (const auto& record : records) {
            if (seen_connector.insert(record.get_connector()).second) {
                connectors.push_back(record.get_connector());
            }
            if (seen_currency.insert(record.get_currency()).second) {
                app::model::refunds::CurrencyValue currency{};
                currency.set_value(record.get_currency());
                currencies.push_back(std::move(currency));
            }
            if (seen_status.insert(record.get_status()).second) {
                app::model::refunds::RefundStatusValue status{};
                status.set_value(record.get_status());
                statuses.push_back(std::move(status));
            }
        }
        app::model::refunds::RefundListMetaData response{};
        response.set_connector(std::move(connectors));
        response.set_currency(std::move(currencies));
        response.set_refund_status(std::move(statuses));
        return response;
    }

    [[nodiscard]] app::model::refunds::RefundListFilters
    filter_metadata(const std::vector<Record>& records) const
    {
        std::unordered_map<std::string, std::vector<app::model::refunds::MerchantConnectorInfoValue>> connectors{};
        std::vector<app::model::refunds::CurrencyValue> currencies{};
        std::vector<app::model::refunds::RefundStatusValue> statuses{};
        std::unordered_set<std::string> seen_currency{};
        std::unordered_set<std::string> seen_status{};
        for (const auto& record : records) {
            app::model::refunds::MerchantConnectorInfoValue info{};
            info.set_value(record.get_connector());
            connectors[record.get_connector()].push_back(std::move(info));
            if (seen_currency.insert(record.get_currency()).second) {
                app::model::refunds::CurrencyValue currency{};
                currency.set_value(record.get_currency());
                currencies.push_back(std::move(currency));
            }
            if (seen_status.insert(record.get_status()).second) {
                app::model::refunds::RefundStatusValue status{};
                status.set_value(record.get_status());
                statuses.push_back(std::move(status));
            }
        }
        app::model::refunds::RefundListFilters response{};
        response.set_connector(std::move(connectors));
        response.set_currency(std::move(currencies));
        response.set_refund_status(std::move(statuses));
        return response;
    }

    [[nodiscard]] app::model::refunds::PlatformRefundListFilters
    platform_filter_metadata(const std::vector<Record>& records) const
    {
        std::unordered_map<std::string, std::vector<app::model::refunds::MerchantConnectorInfoValue>> connectors{};
        std::vector<app::model::refunds::CurrencyValue> currencies{};
        std::vector<app::model::refunds::RefundStatusValue> statuses{};
        std::unordered_set<std::string> seen_currency{};
        std::unordered_set<std::string> seen_status{};
        for (const auto& record : records) {
            app::model::refunds::MerchantConnectorInfoValue info{};
            info.set_value(record.get_connector());
            connectors[record.get_connector()].push_back(std::move(info));
            if (seen_currency.insert(record.get_currency()).second) {
                app::model::refunds::CurrencyValue currency{};
                currency.set_value(record.get_currency());
                currencies.push_back(std::move(currency));
            }
            if (seen_status.insert(record.get_status()).second) {
                app::model::refunds::RefundStatusValue status{};
                status.set_value(record.get_status());
                statuses.push_back(std::move(status));
            }
        }
        app::model::refunds::PlatformRefundListFilters response{};
        response.set_connector(std::move(connectors));
        response.set_currency(std::move(currencies));
        response.set_refund_status(std::move(statuses));
        return response;
    }

    [[nodiscard]] app::model::refunds::PlatformRefundListItem
    to_platform_item(const Record& record) const
    {
        app::model::refunds::PlatformRefundListItem item{};
        item.set_refund_id(record.get_refund_id());
        app::model::refunds::PaymentIdValue payment{};
        payment.set_value(record.get_payment_id());
        item.set_payment_id(payment);
        app::model::refunds::MerchantIdValue merchant{};
        merchant.set_value(record.get_merchant_id());
        item.set_merchant_id(merchant);
        item.set_connector(record.get_connector());
        app::model::refunds::MinorUnitValue amount{};
        amount.set_value(std::to_string(record.get_amount()));
        item.set_refund_amount(amount);
        app::model::refunds::MinorUnitValue total{};
        total.set_value(std::to_string(record.get_amount()));
        item.set_total_amount(total);
        app::model::refunds::CurrencyValue currency{};
        currency.set_value(record.get_currency());
        item.set_currency(currency);
        app::model::refunds::RefundStatusValue status{};
        status.set_value(record.get_status());
        item.set_refund_status(status);
        return item;
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        Error error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
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

} // namespace app::handlers::refunds
