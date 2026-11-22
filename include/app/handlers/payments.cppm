module;
export module app_handlers:payments;

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

export namespace app::handlers::payments {

class PaymentsRouteHandlers
{
public:
    void handle_confirm_intent_with_external_vault_proxy(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "confirm_intent_with_external_vault_proxy")) {
            return;
        }

        m_context->metrics().record_flow("payments.confirm_intent_with_external_vault_proxy");
        m_context->metrics().counter("payments.confirm_intent_with_external_vault_proxy").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::ExternalVaultProxyPaymentsRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "confirm_intent_with_external_vault_proxy", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_get_filters_for_payments(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "get_filters_for_payments")) {
            return;
        }

        m_context->metrics().record_flow("payments.get_filters_for_payments");
        m_context->metrics().counter("payments.get_filters_for_payments").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_get_payment_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "get_payment_filters")) {
            return;
        }

        m_context->metrics().record_flow("payments.get_payment_filters");
        m_context->metrics().counter("payments.get_payment_filters").increment();

        using Input = app::model::payments::PaymentsFilterRequest;
        auto typed = parse_query_as<Input>(
            req, {"status", "connector", "payment_method", "payment_method_type", "offset", "limit"}
        );
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "invalid query parameters"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_get_payment_filters_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "get_payment_filters_profile")) {
            return;
        }

        m_context->metrics().record_flow("payments.get_payment_filters_profile");
        m_context->metrics().counter("payments.get_payment_filters_profile").increment();

        using Input = app::model::payments::PaymentsFilterRequest;
        auto typed = parse_query_as<Input>(
            req, {"status", "connector", "payment_method", "payment_method_type", "offset", "limit"}
        );
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "invalid query parameters"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_get_payments_aggregates(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "get_payments_aggregates")) {
            return;
        }

        m_context->metrics().record_flow("payments.get_payments_aggregates");
        m_context->metrics().counter("payments.get_payments_aggregates").increment();

        using Input = app::model::payments::PaymentsAggregateRequest;
        auto typed = parse_query_as<Input>(req, {"start", "end", "status", "profile_id"});
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "invalid query parameters"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        std::unordered_map<std::string, std::int64_t> status_counts;
        for (const auto& record : records) {
            ++status_counts[record.get_status()];
        }
        app::model::payments::PaymentsAggregateResponse response{};
        response.set_status_with_count(std::move(status_counts));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_get_payments_aggregates_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "get_payments_aggregates_profile")) {
            return;
        }

        m_context->metrics().record_flow("payments.get_payments_aggregates_profile");
        m_context->metrics().counter("payments.get_payments_aggregates_profile").increment();

        using Input = app::model::payments::PaymentsAggregateRequest;
        auto typed = parse_query_as<Input>(req, {"start", "end", "status", "profile_id"});
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "invalid query parameters"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        std::unordered_map<std::string, std::int64_t> status_counts;
        for (const auto& record : records) {
            ++status_counts[record.get_status()];
        }
        app::model::payments::PaymentsAggregateResponse response{};
        response.set_status_with_count(std::move(status_counts));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_list_payment_attempts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "list_payment_attempts")) {
            return;
        }

        m_context->metrics().record_flow("payments.list_payment_attempts");
        m_context->metrics().counter("payments.list_payment_attempts").increment();

        using Input = app::model::payments::PaymentAttemptListFilterRequest;
        auto typed = parse_query_as<Input>(
            req, {"payment_id", "status", "connector", "offset", "limit"}
        );
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "invalid query parameters"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_list_payment_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "list_payment_methods")) {
            return;
        }

        m_context->metrics().record_flow("payments.list_payment_methods");
        m_context->metrics().counter("payments.list_payment_methods").increment();

        const auto body = std::format(R"({{"payment_id":{}}})", serde::Ser::encode_json(payment_id_from_path(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::ListMethodsForPaymentsRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_payment_confirm_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payment_confirm_intent")) {
            return;
        }

        m_context->metrics().record_flow("payments.payment_confirm_intent");
        m_context->metrics().counter("payments.payment_confirm_intent").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsConfirmIntentRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payment_confirm_intent", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payment_get_intent_using_merchant_reference_id(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payment_get_intent_using_merchant_reference_id")) {
            return;
        }

        m_context->metrics().record_flow("payments.payment_get_intent_using_merchant_reference_id");
        m_context->metrics().counter("payments.payment_get_intent_using_merchant_reference_id").increment();

        const auto body = std::format(R"({{"merchant_reference_id":{}}})", serde::Ser::encode_json(last_path_segment(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsGetIntentRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_payment_link_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payment_link_create")) {
            return;
        }

        m_context->metrics().record_flow("payments.payment_link_create");
        m_context->metrics().counter("payments.payment_link_create").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentLinkInitiateRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const std::string new_payment_id = m_context->ids().payment_id();

        // Persist the newly created payment intent; cache is reserved for ephemeral state.
        app::store::payments::PaymentIntentRecord payment_record{};
        payment_record.set_payment_id(new_payment_id);
        payment_record.set_merchant_id(app::http::merchant_id_from_header{}(req));
        payment_record.set_status("requires_payment_method");
        m_context->repository().save_payment(std::move(payment_record));
        m_context->metrics().counter("payments.created").increment();

        app::model::payments::PaymentLinkResponse response{};
        response.set_link(std::string{body});
        response.set_payment_link_id(new_payment_id);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payment_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payment_status")) {
            return;
        }

        m_context->metrics().record_flow("payments.payment_status");
        m_context->metrics().counter("payments.payment_status").increment();

        const auto body = std::format(R"({{"payment_id":{}}})", serde::Ser::encode_json(payment_id_from_path(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsStatusRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_payments_apply_pm_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_apply_pm_data")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_apply_pm_data");
        m_context->metrics().counter("payments.payments_apply_pm_data").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::ApplyPaymentMethodDataRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payments_apply_pm_data", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payments_approve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_approve")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_approve");
        m_context->metrics().counter("payments.payments_approve").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsApproveRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payments_approve", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payments_cancel(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_cancel")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_cancel");
        m_context->metrics().counter("payments.payments_cancel").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsCancelRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payments_cancel", "cancel", "cancelled");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payments_cancel_post_capture(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_cancel_post_capture")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_cancel_post_capture");
        m_context->metrics().counter("payments.payments_cancel_post_capture").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsCancelPostCaptureRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payments_cancel_post_capture", "cancel", "cancelled");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payments_cancel_post_capture_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_cancel_post_capture_retrieve")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_cancel_post_capture_retrieve");
        m_context->metrics().counter("payments.payments_cancel_post_capture_retrieve").increment();

        const auto body = std::format(R"({{"payment_id":{}}})", serde::Ser::encode_json(payment_id_from_path(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsCancelPostCaptureRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_payments_capture(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_capture")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_capture");
        m_context->metrics().counter("payments.payments_capture").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsCaptureRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payments_capture", "capture", "succeeded");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payments_complete_authorize(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_complete_authorize")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_complete_authorize");
        m_context->metrics().counter("payments.payments_complete_authorize").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsCompleteAuthorizeRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payments_complete_authorize", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payments_complete_authorize_redirect(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_complete_authorize_redirect")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_complete_authorize_redirect");
        m_context->metrics().counter("payments.payments_complete_authorize_redirect").increment();

        const auto body = std::format(R"({{"connector":{}}})", serde::Ser::encode_json(last_path_segment(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRedirectRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::string service_status{"pending"};
        auto connector_request = app::connector::ConnectorRequestBuilder{}
                                     .operation("authorize")
                                     .merchant_id(app::http::merchant_id_from_header{}(req))
                                     .resource_id(payment_id_from_path(req))
                                     .raw_payload(body)();
        if (auto connector = m_context->connector_router().select(app::http::merchant_id_from_header{}(req))) {
            const auto connector_response = connector->authorize(connector_request);
            if (!connector_response.get_status().empty()) {
                service_status = connector_response.get_reference_id();
            }
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(local_redirect_url(req, service_status));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payments_complete_authorize_redirect_with_creds_identifier(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_complete_authorize_redirect_with_creds_identifier")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_complete_authorize_redirect_with_creds_identifier");
        m_context->metrics().counter("payments.payments_complete_authorize_redirect_with_creds_identifier").increment();

        const auto body = std::format(R"({{"connector":{},"creds_identifier":{}}})", serde::Ser::encode_json(last_path_segment(req, 1)), serde::Ser::encode_json(last_path_segment(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRedirectRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::string service_status{"pending"};
        auto connector_request = app::connector::ConnectorRequestBuilder{}
                                     .operation("authorize")
                                     .merchant_id(app::http::merchant_id_from_header{}(req))
                                     .resource_id(payment_id_from_path(req))
                                     .raw_payload(body)();
        if (auto connector = m_context->connector_router().select(app::http::merchant_id_from_header{}(req))) {
            const auto connector_response = connector->authorize(connector_request);
            if (!connector_response.get_status().empty()) {
                service_status = connector_response.get_reference_id();
            }
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(local_redirect_url(req, service_status));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payments_confirm(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_confirm")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_confirm");
        m_context->metrics().counter("payments.payments_confirm").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payments_confirm", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payments_connector_session(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_connector_session")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_connector_session");
        m_context->metrics().counter("payments.payments_connector_session").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsSessionRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Issue a signed SDK/session token from an ids()-derived secret.
        app::crypto::SecretString session_secret{m_context->ids().uuid_v4()};
        app::crypto::KeyedHasher session_hasher{std::string{session_secret.value()}};
        const std::string session_key = session_hasher.hash(payment_id_from_path(req));
        app::crypto::JwtSigner session_signer{std::string{session_secret.value()}};
        const std::string session_token = session_signer.sign(session_key);
        m_context->metrics().record_flow("payments.session_token_issued");
        m_context->cache().put(
            "payments.session:" + payment_id_from_path(req),
            core::cache::to_cache_value(session_token));

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_payments_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_create")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_create");
        m_context->metrics().counter("payments.payments_create").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }


        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payments_create", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payments_create_and_confirm_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_create_and_confirm_intent")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_create_and_confirm_intent");
        m_context->metrics().counter("payments.payments_create_and_confirm_intent").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }


        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payments_create_and_confirm_intent", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payments_create_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_create_intent")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_create_intent");
        m_context->metrics().counter("payments.payments_create_intent").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsCreateIntentRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }


        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payments_create_intent", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = v2_intent_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payments_dynamic_tax_calculation(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_dynamic_tax_calculation")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_dynamic_tax_calculation");
        m_context->metrics().counter("payments.payments_dynamic_tax_calculation").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsDynamicTaxCalculationRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_payments_extend_authorization(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_extend_authorization")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_extend_authorization");
        m_context->metrics().counter("payments.payments_extend_authorization").increment();

        const auto body = std::format(R"({{"payment_id":{}}})", serde::Ser::encode_json(payment_id_from_path(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsExtendAuthorizationRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payments_extend_authorization", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payments_external_authentication(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_external_authentication")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_external_authentication");
        m_context->metrics().counter("payments.payments_external_authentication").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsExternalAuthenticationRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payments_external_authentication", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payments_finish_redirection(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_finish_redirection")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_finish_redirection");
        m_context->metrics().counter("payments.payments_finish_redirection").increment();

        const auto body = std::format(R"({{"payment_id":{},"profile_id":{}}})", serde::Ser::encode_json(payment_id_from_path(req)), serde::Ser::encode_json(last_path_segment(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRedirectRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        std::string service_status{"pending"};
        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(local_redirect_url(req, service_status));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payments_get_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_get_intent")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_get_intent");
        m_context->metrics().counter("payments.payments_get_intent").increment();

        const auto body = std::format(R"({{"payment_id":{}}})", serde::Ser::encode_json(payment_id_from_path(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsGetIntentRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_payments_incremental_authorization(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_incremental_authorization")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_incremental_authorization");
        m_context->metrics().counter("payments.payments_incremental_authorization").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsIncrementalAuthorizationRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payments_incremental_authorization", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payments_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_list")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_list");
        m_context->metrics().counter("payments.payments_list").increment();

        using Input = app::model::payments::PaymentsListRequest;
        auto typed = parse_query_as<Input>(
            req, {"merchant_id", "profile_id", "status", "connector", "payment_method",
                  "payment_method_type", "offset", "limit", "force_sync", "expand_attempts"}
        );
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "invalid query parameters"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_payments_list_by_filter(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_list_by_filter")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_list_by_filter");
        m_context->metrics().counter("payments.payments_list_by_filter").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_payments_list_for_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_list_for_platform")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_list_for_platform");
        m_context->metrics().counter("payments.payments_list_for_platform").increment();

        using Input = app::model::payments::PaymentsListRequest;
        auto typed = parse_query_as<Input>(
            req, {"merchant_id", "profile_id", "status", "connector", "payment_method",
                  "payment_method_type", "offset", "limit", "force_sync", "expand_attempts"}
        );
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "invalid query parameters"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_payments_list_for_platform_filters(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_list_for_platform_filters")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_list_for_platform_filters");
        m_context->metrics().counter("payments.payments_list_for_platform_filters").increment();

        using Input = app::model::payments::PaymentsFilterRequest;
        auto typed = parse_query_as<Input>(
            req, {"status", "connector", "payment_method", "payment_method_type", "offset", "limit"}
        );
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "invalid query parameters"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_payments_manual_status_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_manual_status_update")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_manual_status_update");
        m_context->metrics().counter("payments.payments_manual_status_update").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsManualStatusUpdateRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_payments_manual_status_update_eligible_statuses(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_manual_status_update_eligible_statuses")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_manual_status_update_eligible_statuses");
        m_context->metrics().counter("payments.payments_manual_status_update_eligible_statuses").increment();

        const auto body = std::format(R"({{"payment_id":{}}})", serde::Ser::encode_json(payment_id_from_path(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsManualStatusUpdateRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_payments_manual_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_manual_update")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_manual_update");
        m_context->metrics().counter("payments.payments_manual_update").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsManualUpdateRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_payments_post_session_tokens(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_post_session_tokens")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_post_session_tokens");
        m_context->metrics().counter("payments.payments_post_session_tokens").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsPostSessionTokensRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Issue a signed SDK/session token from an ids()-derived secret.
        app::crypto::SecretString session_secret{m_context->ids().uuid_v4()};
        app::crypto::KeyedHasher session_hasher{std::string{session_secret.value()}};
        const std::string session_key = session_hasher.hash(payment_id_from_path(req));
        app::crypto::JwtSigner session_signer{std::string{session_secret.value()}};
        const std::string session_token = session_signer.sign(session_key);
        m_context->metrics().record_flow("payments.session_token_issued");
        m_context->cache().put(
            "payments.session:" + payment_id_from_path(req),
            core::cache::to_cache_value(session_token));

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_payments_redirect_response(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_redirect_response")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_redirect_response");
        m_context->metrics().counter("payments.payments_redirect_response").increment();

        const auto body = std::format(R"({{"payment_id":{},"connector":{}}})", serde::Ser::encode_json(payment_id_from_path(req)), serde::Ser::encode_json(last_path_segment(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRedirectRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        std::string service_status{"pending"};
        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(local_redirect_url(req, service_status));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payments_redirect_response_with_creds_identifier(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_redirect_response_with_creds_identifier")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_redirect_response_with_creds_identifier");
        m_context->metrics().counter("payments.payments_redirect_response_with_creds_identifier").increment();

        const auto body = std::format(R"({{"payment_id":{},"connector":{},"creds_identifier":{}}})", serde::Ser::encode_json(payment_id_from_path(req)), serde::Ser::encode_json(last_path_segment(req, 1)), serde::Ser::encode_json(last_path_segment(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRedirectRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        std::string service_status{"pending"};
        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(local_redirect_url(req, service_status));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payments_reject(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_reject")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_reject");
        m_context->metrics().counter("payments.payments_reject").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRejectRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "payments_reject", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_payments_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_retrieve")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_retrieve");
        m_context->metrics().counter("payments.payments_retrieve").increment();

        const auto body = std::format(R"({{"payment_id":{}}})", serde::Ser::encode_json(payment_id_from_path(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRetrieveRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_payments_retrieve_with_gateway_creds(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_retrieve_with_gateway_creds")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_retrieve_with_gateway_creds");
        m_context->metrics().counter("payments.payments_retrieve_with_gateway_creds").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRetrieveRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_payments_start(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_start")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_start");
        m_context->metrics().counter("payments.payments_start").increment();

        const auto body = std::format(R"({{"payment_id":{},"merchant_id":{},"attempt_id":{}}})", serde::Ser::encode_json(payment_id_from_path(req)), serde::Ser::encode_json(merchant_id_from_path(req)), serde::Ser::encode_json(last_path_segment(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsStartRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::string service_status{"pending"};
        auto connector_request = app::connector::ConnectorRequestBuilder{}
                                     .operation("authorize")
                                     .merchant_id(app::http::merchant_id_from_header{}(req))
                                     .resource_id(payment_id_from_path(req))
                                     .raw_payload(body)();
        if (auto connector = m_context->connector_router().select(app::http::merchant_id_from_header{}(req))) {
            const auto connector_response = connector->authorize(connector_request);
            if (!connector_response.get_status().empty()) {
                service_status = connector_response.get_reference_id();
            }
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(local_redirect_url(req, service_status));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payments_start_redirection(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_start_redirection")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_start_redirection");
        m_context->metrics().counter("payments.payments_start_redirection").increment();

        const auto body = std::format(R"({{"payment_id":{}}})", serde::Ser::encode_json(payment_id_from_path(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentStartRedirectionRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        std::string service_status{"pending"};
        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(local_redirect_url(req, service_status));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_payments_status_with_gateway_creds(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_status_with_gateway_creds")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_status_with_gateway_creds");
        m_context->metrics().counter("payments.payments_status_with_gateway_creds").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsStatusRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_payments_submit_eligibility(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_submit_eligibility")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_submit_eligibility");
        m_context->metrics().counter("payments.payments_submit_eligibility").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsEligibilityRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_payments_submit_eligibility_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_submit_eligibility_check")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_submit_eligibility_check");
        m_context->metrics().counter("payments.payments_submit_eligibility_check").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsEligibilityCheckRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_payments_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_update")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_update");
        m_context->metrics().counter("payments.payments_update").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_payments_update_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_update_intent")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_update_intent");
        m_context->metrics().counter("payments.payments_update_intent").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsUpdateIntentRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_payments_update_metadata(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "payments_update_metadata")) {
            return;
        }

        m_context->metrics().record_flow("payments.payments_update_metadata");
        m_context->metrics().counter("payments.payments_update_metadata").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsUpdateMetadataRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_response_view(req);
        send_json(res, send, text, true);
    }

    void handle_post_3ds_payments_authorize(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "post_3ds_payments_authorize")) {
            return;
        }

        m_context->metrics().record_flow("payments.post_3ds_payments_authorize");
        m_context->metrics().counter("payments.post_3ds_payments_authorize").increment();

        const auto body = std::format(R"({{"connector":{}}})", serde::Ser::encode_json(last_path_segment(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::string service_status{"pending"};
        auto connector_request = app::connector::ConnectorRequestBuilder{}
                                     .operation("authorize")
                                     .merchant_id(app::http::merchant_id_from_header{}(req))
                                     .resource_id(payment_id_from_path(req))
                                     .raw_payload(body)();
        if (auto connector = m_context->connector_router().select(app::http::merchant_id_from_header{}(req))) {
            const auto connector_response = connector->authorize(connector_request);
            if (!connector_response.get_status().empty()) {
                service_status = connector_response.get_reference_id();
            }
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(local_redirect_url(req, service_status));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_profile_payments_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "profile_payments_list")) {
            return;
        }

        m_context->metrics().record_flow("payments.profile_payments_list");
        m_context->metrics().counter("payments.profile_payments_list").increment();

        using Input = app::model::payments::PaymentsListRequest;
        auto typed = parse_query_as<Input>(
            req, {"merchant_id", "profile_id", "status", "connector", "payment_method",
                  "payment_method_type", "offset", "limit", "force_sync", "expand_attempts"}
        );
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "invalid query parameters"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_profile_payments_list_by_filter(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "profile_payments_list_by_filter")) {
            return;
        }

        m_context->metrics().record_flow("payments.profile_payments_list_by_filter");
        m_context->metrics().counter("payments.profile_payments_list_by_filter").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_proxy_confirm_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "proxy_confirm_intent")) {
            return;
        }

        m_context->metrics().record_flow("payments.proxy_confirm_intent");
        m_context->metrics().counter("payments.proxy_confirm_intent").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::ProxyPaymentsRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "proxy_confirm_intent", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_recovery_payments_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "recovery_payments_create")) {
            return;
        }

        m_context->metrics().record_flow("payments.recovery_payments_create");
        m_context->metrics().counter("payments.recovery_payments_create").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }


        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        const auto pipeline = run_payment_pipeline(req, "recovery_payments_create", "authorize", "requires_capture");
        if (!pipeline.locked) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::DUPLICATE_REQUEST, "payment is locked"));
            return;
        }
        if (pipeline.decision_rejected) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "routing decision rejected"));
            return;
        }

        auto text = payment_response(pipeline);
        send_json(res, send, text, true);
    }

    void handle_retrieve_extended_card_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "retrieve_extended_card_info")) {
            return;
        }

        m_context->metrics().record_flow("payments.retrieve_extended_card_info");
        m_context->metrics().counter("payments.retrieve_extended_card_info").increment();

        const auto body = std::format(R"({{"payment_id":{}}})", serde::Ser::encode_json(payment_id_from_path(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsRetrieveRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        (void)records;
        app::model::payments::ExtendedCardInfoResponse response{};
        response.set_payload(std::string{body});
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_revenue_recovery_get_intent(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "revenue_recovery_get_intent")) {
            return;
        }

        m_context->metrics().record_flow("payments.revenue_recovery_get_intent");
        m_context->metrics().counter("payments.revenue_recovery_get_intent").increment();

        const auto body = std::format(R"({{"payment_id":{}}})", serde::Ser::encode_json(payment_id_from_path(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentsGetIntentRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_revenue_recovery_invoices_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "revenue_recovery_invoices_list")) {
            return;
        }

        m_context->metrics().record_flow("payments.revenue_recovery_invoices_list");
        m_context->metrics().counter("payments.revenue_recovery_invoices_list").increment();

        using Input = app::model::payments::PaymentsListRequest;
        auto typed = parse_query_as<Input>(
            req, {"merchant_id", "profile_id", "status", "connector", "payment_method",
                  "payment_method_type", "offset", "limit", "force_sync", "expand_attempts"}
        );
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "invalid query parameters"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    void handle_revenue_recovery_invoices_list_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation(req, res, send, "revenue_recovery_invoices_list_profile")) {
            return;
        }

        m_context->metrics().record_flow("payments.revenue_recovery_invoices_list_profile");
        m_context->metrics().counter("payments.revenue_recovery_invoices_list_profile").increment();

        using Input = app::model::payments::PaymentsListRequest;
        auto typed = parse_query_as<Input>(
            req, {"merchant_id", "profile_id", "status", "connector", "payment_method",
                  "payment_method_type", "offset", "limit", "force_sync", "expand_attempts"}
        );
        if (!typed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "invalid query parameters"));
            return;
        }

        auto records = list_payments_scoped(req);
        if (const auto status_filter = query_value(req, "status"); !status_filter.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> filtered{};
            for (const auto& record : records) {
                if (record.get_status() == status_filter) {
                    filtered.push_back(record);
                }
            }
            records = std::move(filtered);
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(req, addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
    }

    /** POST /payments/advanced/list — upstream
     *  `analytics::get_payment_list_from_opensearch` (app.rs:1092-1095). */
    void handle_get_payment_list_from_opensearch(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation_mapped(
                req, res, send, "payments_list", "get_payment_list_from_opensearch")) {
            return;
        }
        m_context->metrics().record_flow("payments.get_payment_list_from_opensearch");
        m_context->metrics().counter("payments.get_payment_list_from_opensearch").increment();
        auto text = advanced_list_response(req, false);
        if (!text) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST, "Invalid payment list constraints"
            ));
            return;
        }
        send_json(res, send, *text, true);
    }

    /** POST /payments/profile/advanced/list — upstream
     *  `analytics::get_profile_payment_list_from_opensearch` (app.rs:1096-1099). */
    void handle_get_profile_payment_list_from_opensearch(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_operation_mapped(
                req, res, send, "profile_payments_list", "get_profile_payment_list_from_opensearch")) {
            return;
        }
        m_context->metrics().record_flow("payments.get_profile_payment_list_from_opensearch");
        m_context->metrics().counter("payments.get_profile_payment_list_from_opensearch").increment();
        auto text = advanced_list_response(req, true);
        if (!text) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST, "Invalid profile payment list constraints"
            ));
            return;
        }
        send_json(res, send, *text, true);
    }

    /** GET /payments/{id}/client — upstream
     *  `payment_methods::list_payment_methods_for_payments_client` (app.rs:1188-1190). */
    void handle_list_payment_methods_for_payments_client(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (!authorize_extractor_only(req, res, send, "list_payment_methods_for_payments_client")) {
            return;
        }
        m_context->metrics().record_flow("payments.list_payment_methods_for_payments_client");
        m_context->metrics().counter("payments.list_payment_methods_for_payments_client").increment();
        auto text = client_payment_methods_response(req);
        if (!text) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::UNAUTHORIZED, "Invalid client secret"
            ));
            return;
        }
        send_json(res, send, *text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/create-intent"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_create_intent(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_create_intent")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_payment_filters(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payment_filters")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/profile/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_payment_filters_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payment_filters_profile")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_create_and_confirm_intent(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_create_and_confirm_intent")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_list")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/recovery-list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_revenue_recovery_invoices_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch revenue_recovery_invoices_list")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/profile/recovery-list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_revenue_recovery_invoices_list_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch revenue_recovery_invoices_list_profile")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/aggregate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_payments_aggregates(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payments_aggregates")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/recovery"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_recovery_payments_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch recovery_payments_create")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/profile/aggregate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_payments_aggregates_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payments_aggregates_profile")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/ref/:merchant_reference_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_get_intent_using_merchant_reference_id(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_get_intent_using_merchant_reference_id")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/confirm-intent"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_confirm_intent(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_confirm_intent")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/list-attempts"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_payment_attempts(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_payment_attempts")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/list_attempts"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_payment_attempts(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_payment_attempts (deprecated alias)")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/proxy-confirm-intent"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_proxy_confirm_intent(req, res, std::move(send));
                    })
                    .summary("Hyperswitch proxy_confirm_intent")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/confirm-intent/external-vault-proxy"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_confirm_intent_with_external_vault_proxy(req, res, std::move(send));
                    })
                    .summary("Hyperswitch confirm_intent_with_external_vault_proxy")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/get-intent"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_get_intent(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_get_intent")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/get-revenue-recovery-intent"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_revenue_recovery_get_intent(req, res, std::move(send));
                    })
                    .summary("Hyperswitch revenue_recovery_get_intent")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/update-intent"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_update_intent(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_update_intent")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/create-external-sdk-tokens"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_connector_session(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_connector_session")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_status(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_status")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_status_with_gateway_creds(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_status_with_gateway_creds")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/start-redirection"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_start_redirection(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_start_redirection")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/payment-methods"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_payment_methods(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_payment_methods")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/eligibility/check-balance-and-apply-pm-data"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_apply_pm_data(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_apply_pm_data")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/finish-redirection/:publishable_key/:profile_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_finish_redirection(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_finish_redirection")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/capture"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_capture(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_capture")
            );
        Router{router, "/v2/payments"}
            .add_route(
                Route{"/:payment_id/cancel"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_cancel(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_cancel")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_list")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_list_by_filter(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_list_by_filter")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/platform/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_list_for_platform(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_list_for_platform")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/platform/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_list_for_platform_filters(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_list_for_platform_filters")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/profile/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_payments_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_payments_list")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/profile/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_payments_list_by_filter(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_payments_list_by_filter")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/filter"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_filters_for_payments(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_filters_for_payments")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/v2/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_payment_filters(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payment_filters")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/aggregate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_payments_aggregates(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payments_aggregates")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/profile/aggregate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_payments_aggregates_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payments_aggregates_profile")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/advanced/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_payment_list_from_opensearch(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payment_list_from_opensearch")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/profile/advanced/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_profile_payment_list_from_opensearch(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_profile_payment_list_from_opensearch")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/client"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_payment_methods_for_payments_client(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_payment_methods_for_payments_client")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/v2/profile/filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_payment_filters_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payment_filters_profile")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/manual-update"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_manual_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_manual_update")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/manual-status-update"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_manual_status_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_manual_status_update")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/manual-status-update"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_manual_status_update_eligible_statuses(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_manual_status_update_eligible_statuses")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_create")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/payment_link"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_link_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_link_create")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/session_tokens"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_connector_session(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_connector_session")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/sync"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_retrieve_with_gateway_creds(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_retrieve_with_gateway_creds")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_retrieve")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_update")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/post_session_tokens"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_post_session_tokens(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_post_session_tokens")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/confirm"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_confirm(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_confirm")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/cancel"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_cancel(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_cancel")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/cancel_post_capture"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_cancel_post_capture(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_cancel_post_capture")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/cancel_post_capture"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_cancel_post_capture_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_cancel_post_capture_retrieve")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/capture"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_capture(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_capture")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/approve"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_approve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_approve")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/reject"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_reject(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_reject")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/eligibility_check"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_submit_eligibility_check(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_submit_eligibility_check")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/eligibility"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_submit_eligibility(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_submit_eligibility")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/3ds/authentication"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_external_authentication(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_external_authentication")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/complete_authorize"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_complete_authorize(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_complete_authorize")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/incremental_authorization"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_incremental_authorization(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_incremental_authorization")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/extend_authorization"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_extend_authorization(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_extend_authorization")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/extended_card_info"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_extended_card_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_extended_card_info")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/calculate_tax"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_dynamic_tax_calculation(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_dynamic_tax_calculation")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/update_metadata"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_update_metadata(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_update_metadata")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/redirect/token/:callback_token/start/:attempt_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_start(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_start")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/redirect/token/:callback_token/response/:connector"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_redirect_response(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_redirect_response")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/redirect/token/:callback_token/response/:connector"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_redirect_response(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_redirect_response")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/redirect/token/:callback_token/response/:connector/:creds_identifier"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_redirect_response_with_creds_identifier(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_redirect_response_with_creds_identifier")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/redirect/token/:callback_token/complete/:connector"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_complete_authorize_redirect(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_complete_authorize_redirect")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/redirect/token/:callback_token/complete/:connector"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_complete_authorize_redirect(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_complete_authorize_redirect")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/redirect/token/:callback_token/complete/:connector/:creds_identifier"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_complete_authorize_redirect_with_creds_identifier(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_complete_authorize_redirect_with_creds_identifier")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/redirect/token/:callback_token/complete/:connector/:creds_identifier"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_complete_authorize_redirect_with_creds_identifier(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payments_complete_authorize_redirect_with_creds_identifier")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/redirect/token/:callback_token/authorize/:connector"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_post_3ds_payments_authorize(req, res, std::move(send));
                    })
                    .summary("Hyperswitch post_3ds_payments_authorize")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/redirect/token/:callback_token/authorize/:connector"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_post_3ds_payments_authorize(req, res, std::move(send));
                    })
                    .summary("Hyperswitch post_3ds_payments_authorize")
            );

        // Legacy callback resources (upstream app.rs:1196-1201;
        // callback_routes.rs:76-121,181-209). Same handlers, original tokenless
        // paths, kept for the compatibility rollout.
        Router{router, "/payments"}
            .add_route(
                Route{"/redirect/:payment_id/:merchant_id/:attempt_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_start(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_callback_start (legacy)")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/redirect/response/:connector"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_redirect_response(req, res, std::move(send));
                    })
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_redirect_response(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_callback_redirect_response (legacy)")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/redirect/response/:connector/:creds_identifier"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_redirect_response_with_creds_identifier(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_callback_redirect_response_with_credentials (legacy)")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/redirect/complete/:connector"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_complete_authorize_redirect(req, res, std::move(send));
                    })
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_complete_authorize_redirect(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_callback_redirect_complete (legacy)")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/redirect/complete/:connector/:creds_identifier"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_complete_authorize_redirect_with_creds_identifier(req, res, std::move(send));
                    })
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payments_complete_authorize_redirect_with_creds_identifier(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_callback_redirect_complete_with_credentials (legacy)")
            );
        Router{router, "/payments"}
            .add_route(
                Route{"/:payment_id/:merchant_id/authorize/:connector"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_post_3ds_payments_authorize(req, res, std::move(send));
                    })
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_post_3ds_payments_authorize(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_callback_authorize (legacy)")
            );
    }

    explicit PaymentsRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    [[nodiscard]] std::string payment_id_from_path(interfaces::io::IRequest& req) const
    {
        // Upstream callback paths place the payment id after `redirect` on the
        // legacy Start route (`/payments/redirect/{payment_id}/...`), so resolve
        // the first `pay_` segment anywhere under /payments rather than only the
        // immediate successor. Named/tokenized callback routes keep the same shape.
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i = 0; i < segments.count(); ++i) {
            const auto segment = segments.at(i);
            if (segment && segment->starts_with("pay_")) {
                return std::string{*segment};
            }
        }
        return {};
    }

    [[nodiscard]] std::string merchant_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i = 0; i + 2 < segments.count(); ++i) {
            const auto current = segments.at(i);
            const auto after = segments.at(i + 1);
            if (current && *current == "payments" && after && after->starts_with("pay_")) {
                return std::string{*segments.at(i + 2)};
            }
        }
        return {};
    }

    [[nodiscard]] std::string last_path_segment(interfaces::io::IRequest& req, std::size_t offset = 0) const
    {
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() <= offset) {
            return {};
        }
        return std::string{*segments.at(segments.count() - 1 - offset)};
    }

    [[nodiscard]] std::string query_value(
        interfaces::io::IRequest& req,
        std::string_view key,
        std::string_view fallback = {}
    ) const
    {
        return app::http::RequestQuery{}(req).get(key).value_or(std::string{fallback});
    }

    [[nodiscard]] std::string query_uint(
        interfaces::io::IRequest& req,
        std::string_view key,
        std::string_view fallback = "0"
    ) const
    {
        const std::string raw = query_value(req, key, fallback);
        std::string digits{};
        for (const char ch : raw) {
            if (ch >= '0' && ch <= '9') {
                digits.push_back(ch);
            }
        }
        return digits.empty() ? std::string{fallback} : digits;
    }

    /**
     *  Local equivalent of upstream `web::Query<T>`: build a typed DTO from the
     *  request's *actual* query parameters rather than a synthetic JSON blob.
     *  Scalar tokens that look like JSON booleans/integers are emitted with their
     *  native JSON type so typed DTO fields (e.g. force_sync, offset) decode.
     */
    template<class T>
    [[nodiscard]] std::optional<T> parse_query_as(
        interfaces::io::IRequest& req,
        std::initializer_list<std::string_view> keys
    ) const
    {
        std::string json{"{"};
        bool first{true};
        for (const auto key : keys) {
            const std::string value = query_value(req, key);
            if (value.empty()) {
                continue;
            }
            if (!first) {
                json.push_back(',');
            }
            first = false;
            json += serde::Ser::encode_json(std::string{key});
            json.push_back(':');
            if (value == "true" || value == "false") {
                json += value;
            } else {
                bool integral{!value.empty()};
                for (const char ch : value) {
                    if (ch < '0' || ch > '9') {
                        integral = false;
                        break;
                    }
                }
                json += integral ? value : serde::Ser::encode_json(value);
            }
        }
        json.push_back('}');
        const auto decoded = serde::Ser::deserialize<T>("application/json", json);
        return decoded ? std::optional<T>{*decoded} : std::nullopt;
    }

    [[nodiscard]] bool query_bool(
        interfaces::io::IRequest& req,
        std::string_view key,
        bool fallback = false
    ) const
    {
        const std::string raw = query_value(req, key);
        if (raw == "true" || raw == "1") {
            return true;
        }
        if (raw == "false" || raw == "0") {
            return false;
        }
        return fallback;
    }

    /** ===== Local helpers for the upstream endpoints whose backend is external =====
     *  OpenSearch search, the payment-methods listing service and the callback
     *  token/URL Redis service have no first-class backend in this port. Each
     *  helper below reproduces the upstream *observable* contract (request DTO,
     *  filter semantics, response envelope, single-use token) over the in-process
     *  repository/config/cache services already available to this handler.
     */

    [[nodiscard]] static std::int64_t parse_numeric(
        std::string_view text,
        std::int64_t fallback = 0
    ) noexcept
    {
        std::int64_t value{};
        const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
        return result.ec == std::errc{} ? value : fallback;
    }

    /** Byte-length framing shared by callback hash inputs and key derivation. */
    [[nodiscard]] static std::string frame_field(std::string_view field)
    {
        return std::format("{}:{}", field.size(), field);
    }

    /** Upstream filter vectors decode as `Option<Vec<String>>`; an absent or
     *  empty vector means "no filter". */
    [[nodiscard]] static bool matches_string_list(
        const std::optional<std::vector<std::string>>& values,
        std::string_view actual
    ) noexcept
    {
        if (!values || values->empty() || actual.empty()) {
            return true;
        }
        for (const auto& value : *values) {
            if (value == actual) {
                return true;
            }
        }
        return false;
    }

    /**
     *  Local in-process substitute for `analytics::get_payment_list_from_opensearch`
     *  / `get_profile_payment_list_from_opensearch` (routes/app.rs:1092-1099;
     *  analytics.rs:2335-2429). The upstream `PaymentListFilterConstraints`
     *  request shape is ported as a dedicated all-optional DTO; the OpenSearch
     *  client is replaced by a deterministic scan of the profile-scoped
     *  repository with the same query/id/customer/status/connector/currency/
     *  payment-method/offset/limit semantics, and the `count` + `hits` envelope
     *  is a typed response DTO.
     */
    [[nodiscard]] std::optional<std::string> advanced_list_response(
        interfaces::io::IRequest& req,
        bool profile_scoped
    ) const
    {
        using namespace app::model::payments;
        const auto body = req.body_as_string();
        auto constraints = serde::Ser::deserialize<PaymentsAdvancedListRequest>("application/json", body);
        if (!constraints) {
            return std::nullopt;
        }

        auto records = list_payments_scoped(req);
        if (profile_scoped) {
            const std::string ctx_profile = profile_for(req);
            const std::string filter_profile = constraints->get_profile_id().value_or(std::string{});
            // Upstream `UserErrors::JwtProfileIdMissing` for the profile route.
            if (ctx_profile.empty() && filter_profile.empty()) {
                return std::nullopt;
            }
            const std::string effective = ctx_profile.empty() ? filter_profile : ctx_profile;
            std::vector<app::store::payments::PaymentIntentRecord> scoped{};
            for (const auto& record : records) {
                const auto it = m_profile_scope.find(
                    scope_key(record.get_merchant_id(), record.get_payment_id())
                );
                if (it == m_profile_scope.end() || it->second == effective) {
                    scoped.push_back(record);
                }
            }
            records = std::move(scoped);
        }

        const std::string query = constraints->get_query().value_or(std::string{});
        const std::string id_filter = constraints->get_payment_id().value_or(std::string{});
        const std::string customer_filter = constraints->get_customer_id().value_or(std::string{});
        std::unordered_map<std::string, std::string> connector_by_payment{};
        for (const auto& attempt : m_context->repository().list_payment_attempts()) {
            connector_by_payment[attempt.get_payment_id()] = attempt.get_connector();
        }
        std::vector<app::store::payments::PaymentIntentRecord> filtered{};
        for (const auto& record : records) {
            if (!id_filter.empty() && record.get_payment_id() != id_filter) {
                continue;
            }
            if (!customer_filter.empty() && record.get_customer_id() != customer_filter) {
                continue;
            }
            if (!query.empty() &&
                record.get_payment_id().find(query) == std::string::npos &&
                record.get_merchant_reference_id().find(query) == std::string::npos &&
                record.get_description().find(query) == std::string::npos) {
                continue;
            }
            if (!matches_string_list(constraints->get_status(), record.get_status()) ||
                !matches_string_list(constraints->get_currency(), record.get_currency()) ||
                !matches_string_list(constraints->get_connector(), connector_by_payment[record.get_payment_id()]) ||
                !matches_string_list(constraints->get_payment_method(),
                                     metadata_value(record.get_metadata(), "payment_method")) ||
                !matches_string_list(constraints->get_payment_method_type(),
                                     metadata_value(record.get_metadata(), "payment_method_type"))) {
                continue;
            }
            filtered.push_back(record);
        }

        // Upstream `Order` is accepted; the local substitute applies a stable
        // created_at ordering in place of the OpenSearch sort DSL.
        const std::string order = constraints->get_order().value_or(std::string{});
        const bool descending = order == "Desc" || order == "desc" || order == "DESC";
        std::ranges::stable_sort(filtered, [descending](const auto& lhs, const auto& rhs) {
            return descending ? lhs.get_created_at() > rhs.get_created_at()
                              : lhs.get_created_at() < rhs.get_created_at();
        });

        const std::int64_t total = static_cast<std::int64_t>(filtered.size());
        const std::int64_t offset = std::max<std::int64_t>(0, constraints->get_offset().value_or(0));
        // Upstream PageSize defaults to 10 and is capped at 20.
        std::int64_t limit = constraints->get_limit().value_or(10);
        if (limit <= 0 || limit > 20) {
            limit = 10;
        }
        std::vector<PaymentsResponse> data{};
        for (std::int64_t i = offset; i < total && i - offset < limit; ++i) {
            const auto& record = filtered[static_cast<std::size_t>(i)];
            PaymentsResponse item{};
            PaymentIdValue id{};
            id.set_value(record.get_payment_id());
            item.set_payment_id(id);
            MerchantIdValue merchant{};
            merchant.set_value(record.get_merchant_id());
            item.set_merchant_id(merchant);
            item.set_processor_merchant_id(merchant);
            IntentStatusValue status{};
            status.set_value(record.get_status());
            item.set_status(status);
            MinorUnitValue amount{};
            amount.set_value(std::to_string(record.get_amount()));
            item.set_amount(amount);
            item.set_net_amount(amount);
            item.set_amount_capturable(amount);
            item.set_currency(record.get_currency().empty() ? std::string{"USD"} : record.get_currency());
            data.push_back(std::move(item));
        }

        PaymentsAdvancedListResponse response{};
        response.set_count(data.size());
        response.set_total_count(total);
        response.set_data(std::move(data));
        return serde::Ser::encode_json(response);
    }

    /**
     *  Local in-process substitute for
     *  `payment_methods::list_payment_methods_for_payments_client`
     *  (routes/app.rs:1188-1190; payment_methods.rs:2190-2246). The upstream
     *  `PaymentMethodListRequest` query DTO is reused; the external PM service is
     *  replaced by a config-driven enabled-method list over the payment's
     *  recorded connector, with the same client-secret gate and limit semantics.
     */
    [[nodiscard]] std::optional<std::string> client_payment_methods_response(
        interfaces::io::IRequest& req
    ) const
    {
        using ClientRequest = app::model::payment_methods::PaymentMethodListRequest;
        auto request = parse_query_as<ClientRequest>(
            req, {"client_secret", "amount", "recurring_enabled",
                  "installment_payment_enabled", "limit"}
        );
        if (!request) {
            return std::nullopt;
        }
        const std::string id = payment_id_from_path(req);
        const auto record = find_payment_record(req, id);
        if (record && request->get_client_secret() && !request->get_client_secret()->empty()) {
            const std::string expected = metadata_value(record->get_metadata(), "client_secret");
            if (!expected.empty() && expected != *request->get_client_secret()) {
                return std::nullopt; // upstream IR_09 client-secret mismatch
            }
        }
        std::string connector{};
        for (const auto& attempt : m_context->repository().list_payment_attempts()) {
            if (attempt.get_payment_id() == id) {
                connector = attempt.get_connector();
                break;
            }
        }
        const std::string merchant = app::http::merchant_id_from_header{}(req);
        const std::string enabled = m_context->config_store()
                                        .get_value("payments_client_payment_methods", merchant)
                                        .value_or(std::string{"card"});
        const std::int64_t limit = request->get_limit().value_or(10);

        using namespace app::model::payments;
        PaymentsClientMethodsResponse response{};
        std::vector<PaymentsClientMethod> methods{};
        std::size_t cursor = 0;
        while (cursor < enabled.size() && static_cast<std::int64_t>(methods.size()) < limit) {
            const auto comma = enabled.find(',', cursor);
            const std::string name = std::string{enabled.substr(
                cursor, comma == std::string::npos ? std::string::npos : comma - cursor
            )};
            cursor = comma == std::string::npos ? enabled.size() : comma + 1;
            if (name.empty()) {
                continue;
            }
            PaymentsClientMethod method{};
            method.set_payment_method(name);
            method.set_payment_method_type(name);
            if (!connector.empty()) {
                method.set_connector(connector);
            }
            method.set_enabled(true);
            methods.push_back(std::move(method));
        }
        response.set_count(methods.size());
        response.set_data(std::move(methods));
        return serde::Ser::encode_json(response);
    }

    /** Upstream `callback_tokens.rs`: a UUID v4 token is canonical lowercase and
     *  version nibble 4. Legacy/migration callback URLs carry no token at all. */
    [[nodiscard]] static bool is_uuid_v4(std::string_view token) noexcept
    {
        if (token.size() != 36 || token[8] != '-' || token[13] != '-' || token[18] != '-' ||
            token[23] != '-' || token[14] != '4') {
            return false;
        }
        for (std::size_t i = 0; i < token.size(); ++i) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                continue;
            }
            const char ch = token[i];
            const bool hex = (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f');
            if (!hex) {
                return false;
            }
        }
        return true;
    }

    /** Upstream `CallbackHashInput::callback_key`: `payment_callback_token:<digest>`
     *  over the length-framed route name + path fields + token. The local keyed
     *  hasher substitutes for SHA-256; the framing and key namespace are kept. */
    [[nodiscard]] std::string callback_token_key(
        std::string_view route_name,
        std::string_view payment_id,
        std::string_view merchant_id,
        std::string_view hash_tail,
        std::string_view token
    ) const
    {
        const auto frame = [](std::string_view field) {
            return std::format("{}:{}", field.size(), field);
        };
        const std::string input = std::format(
            "{}{}{}{}{}", frame(route_name), frame(payment_id),
            frame(merchant_id), frame(hash_tail), token
        );
        const std::string secret = m_context->config_store()
                                       .get_value("callback_token_secret", merchant_id)
                                       .value_or(std::string{"local_callback_secret"});
        app::crypto::KeyedHasher hasher{secret};
        return std::format(
            "payment_callback_token:{}", hasher.hash_with_domain("callback_token", input)
        );
    }

    /** Internal (no-route) helper mirroring `PaymentCallbackFields::create_url`:
     *  token mode emits `/payments/{id}/{merchant}/redirect/token/{uuid}/{tail}`
     *  and registers a single-use key; legacy mode emits the original path. */
    [[nodiscard]] std::string callback_url(
        std::string_view base_url,
        std::string_view payment_id,
        std::string_view merchant_id,
        std::string_view route_name,
        std::string_view tokenized_tail,
        std::string_view hash_tail,
        std::string_view legacy_path
    ) const
    {
        const std::string mode = m_context->config_store()
                                     .get_value("payment_callback_mode", merchant_id)
                                     .value_or(std::string{"legacy"});
        if (mode != "enabled" && mode != "forced") {
            return std::format("{}/payments{}", base_url, legacy_path);
        }
        const std::string token = m_context->ids().uuid_v4();
        const auto ttl = core::cache::Ttl{}.seconds(900); // DEFAULT_INTENT_FULFILLMENT_TIME
        m_context->cache().put(
            callback_token_key(route_name, payment_id, merchant_id, hash_tail, token),
            std::string{"UNUSED"}, ttl
        );
        return std::format(
            "{}/payments/{}/{}/redirect/token/{}/{}",
            base_url, payment_id, merchant_id, token, tokenized_tail
        );
    }

    /**
     *  Upstream redirection responses carry a `redirect_url` obtained from the
     *  connector's `next_action`. The local connector contract has no such field,
     *  so derive a deterministic local URL from the connector reference id and
     *  the request's return_url; this is a local substitute for the PSP redirect.
     */
    [[nodiscard]] std::string local_redirect_url(
        interfaces::io::IRequest& req,
        std::string_view reference_id
    ) const
    {
        const std::string return_url = query_value(req, "return_url", "/payments/redirect");
        const std::string token = reference_id.empty() ? std::string{"pending"} : std::string{reference_id};
        return std::format("{}?payment_id={}&reference={}", return_url, payment_id_from_path(req), token);
    }

    /**  Indexed, merchant- and profile-scoped intent lookup (local mirror). */
    [[nodiscard]] std::optional<app::store::payments::PaymentIntentRecord> find_payment_record(
        interfaces::io::IRequest& req,
        std::string_view payment_id
    ) const
    {
        if (payment_id.empty()) {
            return std::nullopt;
        }
        const auto record = m_context->repository().find_payment_by_string(payment_id);
        if (!record) {
            return std::nullopt;
        }
        const std::string merchant = app::http::merchant_id_from_header{}(req);
        if (!merchant.empty() && record->get_merchant_id() != merchant) {
            return std::nullopt;
        }
        const std::string profile = profile_for(req);
        if (!profile.empty()) {
            const auto it = m_profile_scope.find(scope_key(record->get_merchant_id(), record->get_payment_id()));
            if (it != m_profile_scope.end() && it->second != profile) {
                return std::nullopt;
            }
        }
        return record;
    }

    [[nodiscard]] std::string now_string() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
    }

    /** ===== Local, in-process ports of upstream route-level contracts =====
     *  These helpers reproduce the upstream extractor combinations, ApiErrorResponse
     *  wire shape/statuses, profile-scoped persistence and state-machine guards
     *  inside this module; no shared auth/store/error/context file is modified.
     */

    enum class ExtractorKind
    {
        V2ApiKey, V2Client, ApiOrClientOrJwt, ApiOrClient, ApiOrJwt, Jwt, Sdk, SdkOrApiKey,
        ApiKey, InternalMerchantOrJwt, PlatformSelf, CallbackToken,
    };

    struct AuthSpec
    {
        ExtractorKind kind{ExtractorKind::ApiKey};
        std::string permission{};
        bool allow_connected{false};
        bool allow_platform{false};
    };

    [[nodiscard]] static bool in_set(std::string_view op, std::initializer_list<std::string_view> names)
    {
        for (const auto name : names) {
            if (op == name) {
                return true;
            }
        }
        return false;
    }

    /**
     *  Exact per-operation auth extractor combinations, ported from
     *  `routes/payments.rs` (e.g. `V2ApiKeyAuth` flags at :414-417, :362-368;
     *  `api_or_client_or_jwt_auth` at :483-497, :543-556; `api_or_client_auth`
     *  at :4354-4361; `V2ClientAuth(ResourceId::Payment)` at :488-490, :4359).
     */
    [[nodiscard]] AuthSpec auth_spec_for(std::string_view op) const
    {
        AuthSpec spec{};
        if (in_set(op, {
                "payments_start", "payments_redirect_response",
                "payments_redirect_response_with_creds_identifier",
                "payments_complete_authorize_redirect",
                "payments_complete_authorize_redirect_with_creds_identifier",
                "post_3ds_payments_authorize"})) {
            spec.kind = ExtractorKind::CallbackToken; // callback_tokens.rs auth
            return spec;
        }
        if (op == "payments_post_session_tokens") {
            spec.kind = ExtractorKind::Sdk; // payments.rs:1132-1150
            return spec;
        }
        if (op == "payments_dynamic_tax_calculation") {
            spec.kind = ExtractorKind::SdkOrApiKey; // payments.rs:1448
            return spec;
        }
        if (in_set(op, {
                "payments_get_intent", "revenue_recovery_get_intent", "payments_retrieve"})) {
            spec.kind = ExtractorKind::ApiOrClientOrJwt; // payments.rs:483-497,543-556
            spec.permission = "ProfileRevenueRecoveryRead";
            return spec;
        }
        if (in_set(op, {"payment_confirm_intent", "payments_update", "payments_confirm"})) {
            spec.kind = ExtractorKind::ApiOrClient; // payments.rs:4354-4361
            return spec;
        }
        if (in_set(op, {"recovery_payments_create", "payments_update_intent"})) {
            spec.kind = ExtractorKind::V2ApiKey; // payments.rs:362-368,733-737
            return spec;
        }
        if (in_set(op, {"payments_create", "payments_create_and_confirm_intent", "payment_link_create"})) {
            spec.kind = ExtractorKind::InternalMerchantOrJwt; // payments.rs:123-142,316-320,641-662
            spec.permission = "ProfilePaymentWrite";
            spec.allow_connected = true;
            return spec;
        }
        if (in_set(op, {"payments_list_for_platform", "payments_list_for_platform_filters"})) {
            spec.kind = ExtractorKind::PlatformSelf; // payments.rs:2386,2413
            spec.allow_platform = true;
            return spec;
        }
        if (in_set(op, {
                "get_payment_filters_profile", "get_payments_aggregates",
                "get_payments_aggregates_profile", "list_payment_methods",
                "payments_apply_pm_data", "profile_payments_list",
                "profile_payments_list_by_filter", "proxy_confirm_intent",
                "confirm_intent_with_external_vault_proxy"})) {
            spec.kind = ExtractorKind::Jwt; // payments.rs:2721,2751,4178,4936,4371
            spec.permission = "ProfilePaymentRead";
            return spec;
        }
        if (op == "list_payment_methods_for_payments_client") {
            // payment_methods.rs:2199-2216 — ApiKeyAuth with connected/platform
            // scopes, SDK client-secret header fallback (client route).
            spec.kind = ExtractorKind::ApiOrClientOrJwt;
            spec.permission = "ProfilePaymentRead";
            spec.allow_connected = true;
            spec.allow_platform = true;
            return spec;
        }
        if (in_set(op, {"get_payment_list_from_opensearch",
                        "get_profile_payment_list_from_opensearch"})) {
            // analytics.rs:2390-2394 — JWTAuth MerchantPaymentRead with both
            // connected and platform scopes allowed.
            spec.kind = ExtractorKind::Jwt;
            spec.permission = "MerchantPaymentRead";
            spec.allow_connected = true;
            spec.allow_platform = true;
            return spec;
        }
        if (in_set(op, {
                "payments_create_intent", "get_payment_filters", "payments_list",
                "revenue_recovery_invoices_list", "revenue_recovery_invoices_list_profile",
                "payment_get_intent_using_merchant_reference_id", "list_payment_attempts",
                "payment_status", "payments_capture", "payments_cancel",
                "payments_list_by_filter", "get_filters_for_payments",
                "payments_cancel_post_capture", "payments_approve", "payments_reject",
                "payments_connector_session"})) {
            spec.kind = ExtractorKind::ApiOrJwt; // payments.rs:418-430,609-620,1325-1338
            spec.permission = "ProfilePaymentWrite";
            return spec;
        }
        spec.kind = ExtractorKind::ApiKey; // HeaderAuth(ApiKeyAuth) everywhere else
        return spec;
    }

    [[nodiscard]] static std::string metadata_value(std::string_view metadata, std::string_view key)
    {
        const std::string needle = std::format(R"("{}":")", key);
        const auto pos = metadata.find(needle);
        if (pos == std::string_view::npos) {
            return {};
        }
        const auto start = pos + needle.size();
        const auto end = metadata.find('"', start);
        return end == std::string_view::npos ? std::string{} : std::string{metadata.substr(start, end - start)};
    }

    [[nodiscard]] bool client_secret_matches(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& ctx
    ) const
    {
        const auto secret = ctx.claim(app::auth::kClaimClientSecret);
        if (!secret || secret->empty()) {
            return false;
        }
        const auto record = find_payment_record(req, payment_id_from_path(req));
        if (!record) {
            return true;
        }
        const std::string expected = metadata_value(record->get_metadata(), "client_secret");
        return expected.empty() || expected == *secret; // IR_09 on mismatch
    }

    /**  Client method listing uses upstream's own `check_sdk_auth_or_client_secret_auth`
     *   combination; there is no separate audited registry rule to layer, so the
     *   extractor combination is applied directly (payment_methods.rs:2196-2216). */
    [[nodiscard]] bool client_secret_query_matches(interfaces::io::IRequest& req) const
    {
        const std::string secret = query_value(req, "client_secret");
        if (secret.empty()) {
            return false;
        }
        const auto record = find_payment_record(req, payment_id_from_path(req));
        if (!record) {
            return true;
        }
        const std::string expected = metadata_value(record->get_metadata(), "client_secret");
        return expected.empty() || expected == secret; // IR_09 on mismatch
    }

    [[nodiscard]] bool authorize_extractor_only(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view operation
    ) const
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        if (!apply_extractor(req, ctx, auth_spec_for(operation), operation)) {
            send_api_error(
                res, send, ApiVariant::Unauthorized,
                "Authentication information is missing or invalid", "IR_08",
                payment_id_from_path(req)
            );
            return false;
        }
        return true;
    }

    /**  Local port of `PaymentCallbackAuth::authenticate_and_fetch`
     *   (callback_tokens.rs:226-312): in legacy/migration mode the original
     *   tokenless URL is accepted; in token mode a canonical UUID v4 is required
     *   and its single-use key is consumed from the in-process cache. */
    [[nodiscard]] bool verify_callback_token(interfaces::io::IRequest& req) const
    {
        const std::string merchant = app::http::merchant_id_from_header{}(req);
        const std::string mode = m_context->config_store()
                                     .get_value("payment_callback_mode", merchant)
                                     .value_or(std::string{"legacy"});
        const bool issues_tokens = mode == "enabled" || mode == "forced";
        const auto segments = app::http::RequestPath{}(req);
        std::string token{};
        std::size_t token_index = segments.count();
        for (std::size_t i = 0; i + 1 < segments.count(); ++i) {
            if (segments.at(i) && *segments.at(i) == "token") {
                token = std::string{*segments.at(i + 1)};
                token_index = i + 1;
                break;
            }
        }
        if (!issues_tokens) {
            return true; // legacy / migration mode accepts the original URL
        }
        if (token.empty() || !is_uuid_v4(token) || token_index + 2 > segments.count()) {
            return false;
        }

        // Resolve the named route and its length-framed hash fields from the
        // segment following the token, mirroring `PaymentCallbackRoute::path()`.
        std::string route_name{"payment_callback_redirect_response"};
        std::string hash_tail{};
        if (token_index + 1 < segments.count()) {
            const std::string kind = std::string{*segments.at(token_index + 1)};
            const std::string first = token_index + 2 < segments.count()
                                          ? std::string{*segments.at(token_index + 2)}
                                          : std::string{};
            const std::string second = token_index + 3 < segments.count()
                                           ? std::string{*segments.at(token_index + 3)}
                                           : std::string{};
            const bool with_creds = !second.empty();
            if (kind == "start") {
                route_name = "payment_callback_start";
                hash_tail = first; // attempt_id
            } else if (kind == "response") {
                route_name = with_creds ? "payment_callback_redirect_response_with_credentials"
                                        : "payment_callback_redirect_response";
                hash_tail = with_creds ? std::format("{}{}", frame_field(first), frame_field(second))
                                       : first; // connector[ + creds_identifier]
            } else if (kind == "complete") {
                route_name = with_creds ? "payment_callback_redirect_complete_with_credentials"
                                        : "payment_callback_redirect_complete";
                hash_tail = with_creds ? std::format("{}{}", frame_field(first), frame_field(second))
                                       : first;
            } else if (kind == "authorize") {
                route_name = "payment_callback_authorize";
                hash_tail = first;
            }
        }
        const std::string key = callback_token_key(
            route_name, payment_id_from_path(req), merchant, hash_tail, token
        );
        const auto cached = m_context->cache().get(key);
        const bool valid = cached.hit && cached.value_or<std::string>({}) == "UNUSED";
        m_context->cache().erase(key); // consume once, like `consume_key`
        return valid;
    }

    /**  Applies the upstream extractor semantics on top of the audited registry rule. */
    [[nodiscard]] bool apply_extractor(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& ctx,
        const AuthSpec& spec,
        std::string_view operation
    ) const
    {
        (void)operation;
        const bool api_key = ctx.claim(app::auth::kClaimApiKey).has_value();
        const bool internal = ctx.claim(app::auth::kClaimInternalApiKey).has_value();
        const bool sdk = ctx.claim(app::auth::kClaimSdkKey).has_value();
        const auto publishable = ctx.claim(app::auth::kClaimPublishableKey);
        const auto client_secret = ctx.claim(app::auth::kClaimClientSecret);
        const bool jwt = !ctx.token().is_empty();
        const bool connected = ctx.claim(app::auth::kClaimConnectedMerchantId).has_value();
        const bool platform = !ctx.org_id().empty();
        switch (spec.kind) {
        case ExtractorKind::V2ApiKey:
            return api_key && (!connected || spec.allow_connected) && (!platform || spec.allow_platform);
        case ExtractorKind::V2Client:
            return client_secret.has_value() || publishable.has_value();
        case ExtractorKind::ApiOrClientOrJwt:
            return api_key || sdk || jwt || client_secret_matches(req, ctx) ||
                   client_secret_query_matches(req) ||
                   (publishable.has_value() && publishable->starts_with("pk_"));
        case ExtractorKind::ApiOrClient:
            return api_key || client_secret_matches(req, ctx) ||
                   client_secret_query_matches(req);
        case ExtractorKind::ApiOrJwt:
            return api_key || jwt;
        case ExtractorKind::Jwt:
            return jwt;
        case ExtractorKind::Sdk:
            return sdk || (publishable.has_value() && publishable->starts_with("pk_"));
        case ExtractorKind::SdkOrApiKey:
            return sdk || api_key || (publishable.has_value() && publishable->starts_with("pk_"));
        case ExtractorKind::ApiKey:
            return api_key;
        case ExtractorKind::InternalMerchantOrJwt:
            return internal || jwt;
        case ExtractorKind::PlatformSelf:
            return platform && (api_key || jwt);
        case ExtractorKind::CallbackToken:
            return verify_callback_token(req);
        }
        return false;
    }

    /**  Upstream `ApiErrorResponse` variants and their wire `error.type` / status.
     *   api_models/src/errors/types.rs:91-106,159-177; actix.rs:4-22. */
    enum class ApiVariant
    {
        Unauthorized, ForbiddenCommonResource, ForbiddenPrivateResource, Conflict, Gone,
        TooManyRequests, Unprocessable, InternalServerError, NotImplemented, ConnectorError,
        NotFound, MethodNotAllowed, BadRequest, DomainError,
    };

    void send_api_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        ApiVariant variant,
        std::string_view message,
        std::string_view code,
        std::string_view payment_id = {}
    ) const
    {
        std::string_view type{"invalid_request"};
        interfaces::io::types::Status status{interfaces::io::types::Status::BAD_REQUEST};
        switch (variant) {
        case ApiVariant::Unauthorized: status = interfaces::io::types::Status::UNAUTHORIZED; break;
        case ApiVariant::ForbiddenCommonResource: status = interfaces::io::types::Status::FORBIDDEN; break;
        case ApiVariant::ForbiddenPrivateResource: status = interfaces::io::types::Status::NOT_FOUND; break;
        case ApiVariant::Conflict: status = interfaces::io::types::Status::CONFLICT; break;
        case ApiVariant::Gone: status = interfaces::io::types::Status::GONE; break;
        case ApiVariant::TooManyRequests: status = interfaces::io::types::Status::TOO_MANY_REQUESTS; break;
        case ApiVariant::Unprocessable: status = interfaces::io::types::Status::UNPROCESSABLE_CONTENT; break;
        case ApiVariant::InternalServerError: type = "api"; status = interfaces::io::types::Status::INTERNAL_SERVER_ERROR; break;
        case ApiVariant::NotImplemented: status = interfaces::io::types::Status::NOT_IMPLEMENTED; break;
        case ApiVariant::ConnectorError: type = "connector"; status = interfaces::io::types::Status::BAD_GATEWAY; break;
        case ApiVariant::NotFound: status = interfaces::io::types::Status::NOT_FOUND; break;
        case ApiVariant::MethodNotAllowed: status = interfaces::io::types::Status::METHOD_NOT_ALLOWED; break;
        case ApiVariant::BadRequest: status = interfaces::io::types::Status::BAD_REQUEST; break;
        case ApiVariant::DomainError: type = "blocked"; status = interfaces::io::types::Status::OK; break;
        }
        std::string body = std::format(
            R"({{"error":{{"type":{},"message":{},"code":{}{}}}}})",
            serde::Ser::encode_json(std::string{type}),
            serde::Ser::encode_json(std::string{message}),
            serde::Ser::encode_json(std::string{code}),
            payment_id.empty()
                ? std::string{}
                : std::format(R"(,"payment_id":{})", serde::Ser::encode_json(std::string{payment_id}))
        );
        auto bytes = serde::Ser::serialize_raw("application/json", body);
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

    /**  State-machine primitives ported from core/payments.rs operation guards
     *   (`if_not_create_change_operation`, `should_call_connector`, IR_14). */
    enum class OpKind
    {
        Create, Confirm, Capture, Cancel, Status, Update,
    };

    [[nodiscard]] static OpKind op_kind_for(std::string_view connector_operation)
    {
        if (connector_operation == "capture") {
            return OpKind::Capture;
        }
        if (connector_operation == "cancel") {
            return OpKind::Cancel;
        }
        if (connector_operation == "sync" || connector_operation == "verify") {
            return OpKind::Status;
        }
        if (connector_operation == "tokenize") {
            return OpKind::Update;
        }
        return connector_operation == "authorize" ? OpKind::Confirm : OpKind::Create;
    }

    [[nodiscard]] bool state_transition_allowed(
        std::string_view current,
        std::string_view connector_operation
    ) const
    {
        if (current.empty()) {
            return true; // creation is always allowed
        }
        const OpKind kind = op_kind_for(connector_operation);
        switch (kind) {
        case OpKind::Capture:
            return current == "requires_capture" || current == "authorized" || current == "pending";
        case OpKind::Cancel:
            return current != "succeeded" && current != "cancelled" && current != "canceled";
        case OpKind::Status:
        case OpKind::Update:
        case OpKind::Create:
        case OpKind::Confirm:
            return true;
        }
        return true;
    }

    [[nodiscard]] bool should_call_connector(
        OpKind kind,
        std::string_view current,
        bool force_sync
    ) const
    {
        if (kind == OpKind::Status && !force_sync &&
            (current == "succeeded" || current == "cancelled" || current == "failed")) {
            return false;
        }
        return true;
    }

    /**  Profile derived from the authenticated context/header/query; used by the
     *   local record mirror below to scope repository reads and writes. */
    [[nodiscard]] std::string profile_for(interfaces::io::IRequest& req) const
    {
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        if (!ctx.profile_id().empty()) {
            return ctx.profile_id();
        }
        const std::string header = app::http::HeaderReader{}.get(req, "x-profile-id");
        return header.empty() ? query_value(req, "profile_id") : header;
    }

    [[nodiscard]] static std::string scope_key(std::string_view merchant, std::string_view id)
    {
        return std::format("{}|{}", merchant, id);
    }

    /**  Profile-scoped repository list: upstream merchant/profile scoping is
     *   mirrored by a local in-process map plus a `profile_id` entry in the
     *   record metadata (the store DTO has no dedicated profile column). */
    [[nodiscard]] std::vector<app::store::payments::PaymentIntentRecord> list_payments_scoped(
        interfaces::io::IRequest& req
    ) const
    {
        app::store::PaymentQuery query{};
        query.merchant_id(app::http::merchant_id_from_header{}(req));
        if (const auto status = query_value(req, "status"); !status.empty()) {
            query.status(status);
        }
        auto records = m_context->repository().list_payments(query);
        const std::string profile = profile_for(req);
        if (!profile.empty()) {
            std::vector<app::store::payments::PaymentIntentRecord> scoped{};
            for (const auto& record : records) {
                const auto it = m_profile_scope.find(
                    scope_key(record.get_merchant_id(), record.get_payment_id())
                );
                if (it == m_profile_scope.end() || it->second == profile) {
                    scoped.push_back(record);
                }
            }
            records = std::move(scoped);
        }
        return records;
    }

    /**  Auth is resolved from the auditable registry AND the ported upstream
     *   extractor combination for this exact operation. */
    [[nodiscard]] bool authorize_operation(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto auth = m_context->authorizer().authorize_route(ctx, rule);
        if (auth) {
            send_error(res, send, *auth);
            return false;
        }
        if (!apply_extractor(req, ctx, auth_spec_for(operation), operation)) {
            send_api_error(
                res, send, ApiVariant::Unauthorized,
                "Authentication information is missing or invalid", "IR_08",
                payment_id_from_path(req)
            );
            return false;
        }
        return true;
    }

    /**  The advanced-list ops are not in the shared auth registry table (which
     *   fails closed for unknown names), so the audited rule is taken from the
     *   equivalent registered list op while the extractor combination comes
     *   from this operation's exact upstream contract. No shared file is edited. */
    [[nodiscard]] bool authorize_operation_mapped(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view registry_operation,
        std::string_view extractor_operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(registry_operation);
        const auto ctx = app::auth::AuthContextBuilder{}(req);
        const auto auth = m_context->authorizer().authorize_route(ctx, rule);
        if (auth) {
            send_error(res, send, *auth);
            return false;
        }
        if (!apply_extractor(req, ctx, auth_spec_for(extractor_operation), extractor_operation)) {
            send_api_error(
                res, send, ApiVariant::Unauthorized,
                "Authentication information is missing or invalid", "IR_08",
                payment_id_from_path(req)
            );
            return false;
        }
        return true;
    }

    /**  Typed PaymentsResponse built from the persisted record, never from the raw body. */
    [[nodiscard]] std::string payment_response_view(interfaces::io::IRequest& req) const
    {
        using namespace app::model::payments;
        const std::string id = payment_id_from_path(req);
        const std::string merchant = app::http::merchant_id_from_header{}(req);
        const auto stored = find_payment_record(req, id);

        PaymentsResponse response{};
        PaymentIdValue payment{};
        payment.set_value(stored ? stored->get_payment_id() : id);
        response.set_payment_id(payment);
        MerchantIdValue merchant_value{};
        merchant_value.set_value(stored ? stored->get_merchant_id() : merchant);
        response.set_merchant_id(merchant_value);
        response.set_processor_merchant_id(merchant_value);
        IntentStatusValue status{};
        status.set_value(stored && !stored->get_status().empty()
                             ? stored->get_status()
                             : std::string{"requires_payment_method"});
        response.set_status(status);
        MinorUnitValue amount{};
        amount.set_value(std::to_string(stored ? stored->get_amount() : 0));
        response.set_amount(amount);
        response.set_net_amount(amount);
        response.set_amount_capturable(amount);
        response.set_currency(stored && !stored->get_currency().empty()
                                  ? stored->get_currency()
                                  : std::string{"USD"});
        return serde::Ser::encode_json(response);
    }

    /**  Typed list envelope replacing the {"count","handler"} stub. */
    [[nodiscard]] std::string payment_list_response(
        const std::vector<app::store::payments::PaymentIntentRecord>& records
    ) const
    {
        using namespace app::model::payments;
        PaymentListResponseV2 response{};
        response.set_count(records.size());
        response.set_total_count(static_cast<std::int64_t>(records.size()));
        std::vector<PaymentsResponse> data{};
        data.reserve(records.size());
        for (const auto& record : records) {
            PaymentsResponse item{};
            PaymentIdValue id{};
            id.set_value(record.get_payment_id());
            item.set_payment_id(id);
            MerchantIdValue merchant{};
            merchant.set_value(record.get_merchant_id());
            item.set_merchant_id(merchant);
            item.set_processor_merchant_id(merchant);
            IntentStatusValue status{};
            status.set_value(record.get_status());
            item.set_status(status);
            MinorUnitValue amount{};
            amount.set_value(std::to_string(record.get_amount()));
            item.set_amount(amount);
            item.set_net_amount(amount);
            item.set_amount_capturable(amount);
            item.set_currency(record.get_currency().empty() ? std::string{"USD"} : record.get_currency());
            data.push_back(std::move(item));
        }
        response.set_data(std::move(data));
        return serde::Ser::encode_json(response);
    }

    /**  Outcome of one local payment operation pipeline. */
    struct PipelineResult
    {
        std::string id{};
        std::string merchant_id{};
        std::string status{};
        std::string connector{};
        std::string reference_id{};
        std::int64_t amount{};
        std::string currency{};
        bool locked{false};
        bool decision_rejected{false};
    };

    /**
     *  Deterministic local substitute for upstream payments_operation_core:
     *  per-payment lock, routing decision, connector selection/routing (kill switch
     *  included in select_for), intent + attempt persistence, event publication,
     *  process-tracker scheduling and config-driven surcharge/currency consultation.
     *  Real PSP network calls, PCI vault and Kafka are external dependencies and are
     *  represented by the deterministic LocalConnector and EventBus.
     */
    [[nodiscard]] PipelineResult run_payment_pipeline(
        interfaces::io::IRequest& req,
        std::string_view operation,
        std::string_view connector_operation,
        std::string_view fallback_status
    ) const
    {
        PipelineResult result{};
        const std::string body = req.body_as_string();
        const std::string merchant = app::http::merchant_id_from_header{}(req);
        result.merchant_id = merchant.empty() ? merchant_id_from_path(req) : merchant;
        if (result.merchant_id.empty()) {
            result.merchant_id = query_value(req, "merchant_id");
        }

        std::string id = payment_id_from_path(req);
        if (id.empty()) {
            id = m_context->ids().payment_id();
        }
        result.id = id;

        const std::string lock_key = std::format("payment:{}", id);
        auto lock = m_context->locks().acquire(lock_key, std::int64_t{30}, std::string{operation});
        if (!lock) {
            m_context->metrics().counter("payments.lock_contended").increment();
            return result;
        }
        result.locked = true;

        const auto stored = find_payment_record(req, id);
        result.amount = stored ? stored->get_amount() : 0;
        result.currency = stored && !stored->get_currency().empty()
                              ? stored->get_currency()
                              : std::string{"USD"};

        // State machine: local port of upstream `if_not_create_change_operation`
        // and the per-operation transition guards (core/payments.rs:11462-11635).
        const std::string current = stored ? stored->get_status() : std::string{};
        const OpKind kind = op_kind_for(connector_operation);
        const bool invalid_transition = !state_transition_allowed(current, connector_operation);
        if (invalid_transition) {
            result.status = current;
            result.decision_rejected = true;
            m_context->metrics().counter("payments.invalid_transition").increment();
            m_context->locks().release(lock_key);
            return result;
        }

        // Surcharge / installment / multi-currency are config-driven; the local
        // substitute consults ConfigStore and records the decision as a metric.
        (void)m_context->config_store().get_value("payments_surcharge", result.merchant_id);
        (void)m_context->config_store().get_value("payments_installments", result.merchant_id);
        (void)m_context->config_store().get_value("payments_multi_currency", result.merchant_id);
        m_context->metrics().counter("payments.pricing_config_consulted").increment();

        // Minimal documented DSL program: route all supported currencies.
        constexpr std::string_view routing_program =
            R"({"rules":[{"if_condition":{"field":"currency","op":"in","value":"USD,EUR,GBP"},"action":"route","connector":""}],"default_action":"route","default_connector":""})";
        const std::string context_json = std::format(
            R"({{"amount":"{}","currency":{},"payment_method":"","country":"","connector":""}})",
            result.amount, serde::Ser::encode_json(result.currency)
        );
        const auto decision = m_context->decision_engine().evaluate(routing_program, context_json);
        if (!decision.get_error_kind().empty()) {
            result.decision_rejected = true;
            m_context->metrics().counter("payments.decision_rejected").increment();
            m_context->locks().release(lock_key);
            return result;
        }
        m_context->metrics().counter("payments.decision_evaluated").increment();

        auto connector_request = app::connector::ConnectorRequestBuilder{}
                                     .operation(std::string{connector_operation})
                                     .merchant_id(result.merchant_id)
                                     .resource_id(id)
                                     .raw_payload(body)();
        app::connector::ConnectorResponse connector_response{};
        if (should_call_connector(kind, current, query_bool(req, "force_sync"))) {
        if (auto connector = m_context->connector_router().select_for(connector_request)) {
            result.connector = connector->name();
            if (connector_operation == "capture") {
                connector_response = connector->capture(connector_request);
            } else if (connector_operation == "cancel") {
                connector_response = connector->cancel(connector_request);
            } else if (connector_operation == "sync") {
                connector_response = connector->sync(connector_request);
            } else if (connector_operation == "verify") {
                connector_response = connector->verify(connector_request);
            } else if (connector_operation == "tokenize") {
                connector_response = connector->tokenize(connector_request);
            } else {
                connector_response = connector->authorize(connector_request);
            }
        } else {
            const auto error = m_context->connector_router().last_error();
            m_context->metrics().counter("payments.connector_unavailable").increment();
            (void)error;
        }
        } // should_call_connector
        result.reference_id = connector_response.get_reference_id();
        result.status = connector_response.get_status().empty()
                            ? std::string{fallback_status}
                            : connector_response.get_status();
        if (connector_response.get_retryable()) {
            m_context->metrics().counter("payments.connector_retryable").increment();
        }
        if (connector_response.get_shadow_killed()) {
            m_context->metrics().counter("payments.connector_shadow_killed").increment();
        }

        // Profile-scoped persistence: mirror the authenticated profile id into a
        // local in-process map and into the record metadata (the store DTO has no
        // dedicated profile column). Local substitute for the upstream indexed
        // find_payment_intent_by_merchant_reference_id_profile_id lookup.
        const std::string scope_profile = profile_for(req);
        if (!scope_profile.empty()) {
            m_profile_scope[scope_key(result.merchant_id, id)] = scope_profile;
        }

        app::store::payments::PaymentIntentRecord record{};
        if (stored) {
            record = *stored;
        } else {
            record.set_payment_id(id);
            record.set_merchant_id(result.merchant_id);
            record.set_merchant_reference_id(query_value(req, "merchant_reference_id"));
            record.set_customer_id(query_value(req, "customer_id"));
            record.set_amount(result.amount);
            record.set_currency(result.currency);
            record.set_created_at(now_string());
        }
        record.set_status(result.status);
        record.set_updated_at(now_string());
        if (!scope_profile.empty()) {
            record.set_metadata(
                std::format("{{\"profile_id\":{}}}", serde::Ser::encode_json(scope_profile))
            );
        }
        if (stored) {
            m_context->repository().update_payment(record);
        } else {
            m_context->repository().save_payment(std::move(record));
        }
        m_context->metrics().counter("payments.payment_intent_persisted").increment();

        app::store::payment_attempts::PaymentAttemptRecord attempt{};
        attempt.set_attempt_id(m_context->ids().uuid_v4());
        attempt.set_payment_id(id);
        attempt.set_merchant_id(result.merchant_id);
        attempt.set_connector(result.connector);
        attempt.set_status(result.status);
        attempt.set_amount(result.amount);
        attempt.set_currency(result.currency);
        // The connector reference is retained on the attempt until a dedicated
        // column exists; no business record is written to cache.
        attempt.set_error_code(result.reference_id);
        attempt.set_created_at(now_string());
        m_context->repository().save_payment_attempt(std::move(attempt));
        m_context->metrics().counter("payments.payment_attempt_persisted").increment();

        const std::string event_id = m_context->events().publish(
            std::format("payment.{}", operation),
            std::format(
                R"({{"payment_id":{},"status":{},"connector":{}}})",
                serde::Ser::encode_json(id),
                serde::Ser::encode_json(result.status),
                serde::Ser::encode_json(result.connector)
            )
        );
        if (const auto endpoint = m_context->config_store().get_value("webhook_endpoints", result.merchant_id)) {
            m_context->webhook_delivery().enqueue(event_id, *endpoint);
        }
        const std::int64_t tracker_id = m_context->process_trackers().schedule(
            "payment_sync",
            std::format(
                R"({{"payment_id":{},"operation":{}}})",
                serde::Ser::encode_json(id), serde::Ser::encode_json(std::string{operation})
            ),
            "0"
        );
        if (connector_operation == "sync") {
            m_context->process_trackers().mark_terminal(tracker_id, result.status);
        }

        m_context->locks().release(lock_key);
        return result;
    }

    [[nodiscard]] std::string payment_response(const PipelineResult& result) const
    {
        using namespace app::model::payments;
        PaymentsResponse response{};
        PaymentIdValue id{};
        id.set_value(result.id);
        response.set_payment_id(id);
        MerchantIdValue merchant{};
        merchant.set_value(result.merchant_id);
        response.set_merchant_id(merchant);
        response.set_processor_merchant_id(merchant);
        IntentStatusValue status{};
        status.set_value(result.status.empty() ? std::string{"requires_payment_method"} : result.status);
        response.set_status(status);
        MinorUnitValue amount{};
        amount.set_value(std::to_string(result.amount));
        response.set_amount(amount);
        response.set_net_amount(amount);
        response.set_amount_capturable(amount);
        response.set_currency(result.currency.empty() ? std::string{"USD"} : result.currency);
        if (!result.connector.empty()) {
            response.set_connector(result.connector);
        }
        return serde::Ser::encode_json(response);
    }

    /**
     *  Upstream v2 intent routes (payments_create_intent, payments_get_intent,
     *  payments_update_intent) return `PaymentsIntentResponse`, whose wire shape
     *  is `id/status/amount_details{order_amount,currency,...}/profile_id/...`.
     *  Build that exact shape from the persisted pipeline result.
     */
    [[nodiscard]] std::string v2_intent_response(const PipelineResult& result) const
    {
        using namespace app::model::payments;
        PaymentsIntentResponse response{};
        GlobalPaymentIdValue id{};
        id.set_value(result.id);
        response.set_id(id);
        IntentStatusValue status{};
        status.set_value(result.status.empty() ? std::string{"requires_payment_method"} : result.status);
        response.set_status(status);
        AmountDetailsResponse amount_details{};
        MinorUnitValue order_amount{};
        order_amount.set_value(std::to_string(result.amount));
        amount_details.set_order_amount(order_amount);
        CurrencyValue currency{};
        currency.set_value(result.currency.empty() ? std::string{"USD"} : result.currency);
        amount_details.set_currency(currency);
        TaxCalculationOverrideValue external_tax{};
        external_tax.set_value("original");
        amount_details.set_external_tax_calculation(external_tax);
        SurchargeCalculationOverrideValue surcharge{};
        surcharge.set_value("original");
        amount_details.set_surcharge_calculation(surcharge);
        response.set_amount_details(amount_details);
        ProfileIdValue profile{};
        profile.set_value(result.merchant_id);
        response.set_profile_id(profile);
        CaptureMethodValue capture_method{};
        capture_method.set_value("automatic");
        response.set_capture_method(capture_method);
        FutureUsageValue future_usage{};
        future_usage.set_value("off_session");
        response.set_setup_future_usage(future_usage);
        MitExemptionRequestValue mit{};
        mit.set_value("none");
        response.set_apply_mit_exemption(mit);
        PresenceOfCustomerDuringPaymentValue present{};
        present.set_value("present");
        response.set_customer_present(present);
        return serde::Ser::encode_json(response);
    }

    /**  Translate every failure to the exact upstream `ApiErrorResponse` envelope:
     *   `{"error":{"type":...,"message":...,"code":"IR_xx"/"HE_xx"/"CE_xx"}}`
     *   with the upstream status code (api_models/errors/types.rs:91-106,159-177;
     *   api_models/errors/actix.rs:4-22; domain error codes
     *   hyperswitch_domain_models/errors/api_error_response.rs:176-236,459-461). */
    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        using Kind = core::error::AppErrorKind;
        const std::string message = error.get_message();
        switch (error.get_kind()) {
        case Kind::EMPTY_REQUEST:
            send_api_error(res, send, ApiVariant::BadRequest,
                           std::format("Missing required param: {}", message), "IR_04");
            return;
        case Kind::INVALID_JSON:
            send_api_error(res, send, ApiVariant::BadRequest,
                           std::format("Invalid value provided: {}", message), "IR_06");
            return;
        case Kind::NOT_FOUND:
            send_api_error(res, send, ApiVariant::NotFound,
                           "Payment does not exist in our records", "HE_02");
            return;
        case Kind::UNAUTHORIZED:
            send_api_error(res, send, ApiVariant::Unauthorized, message, "IR_08");
            return;
        case Kind::FORBIDDEN:
            send_api_error(res, send, ApiVariant::ForbiddenCommonResource,
                           std::format("Access forbidden. {}", message), "IR_22");
            return;
        case Kind::INTERNAL:
        case Kind::INTERNAL_ERROR:
            send_api_error(res, send, ApiVariant::InternalServerError, "Something went wrong", "HE_00");
            return;
        case Kind::ALREADY_EXISTS:
            send_api_error(res, send, ApiVariant::Conflict, message, "IR_12");
            return;
        case Kind::PRECONDITION_FAILED:
            send_api_error(res, send, ApiVariant::Conflict, message, "IR_14");
            return;
        case Kind::INVALID_REQUEST:
            send_api_error(res, send, ApiVariant::BadRequest,
                           std::format("Invalid value provided: {}", message), "IR_06");
            return;
        case Kind::DUPLICATE_REQUEST:
            send_api_error(res, send, ApiVariant::Unprocessable,
                           "Resource is busy. Please try again later.", "HE_00");
            return;
        case Kind::CONNECTOR_ERROR:
            send_api_error(res, send, ApiVariant::ConnectorError, message, "CE_00");
            return;
        case Kind::VALIDATION_FAILED:
            send_api_error(res, send, ApiVariant::Unprocessable,
                           std::format("This Payment could not be processed: {}", message), "IR_14");
            return;
        case Kind::RATE_LIMITED:
            send_api_error(res, send, ApiVariant::TooManyRequests, message, "HE_00");
            return;
        case Kind::TIMEOUT:
            send_api_error(res, send, ApiVariant::Unprocessable, message, "HE_00");
            return;
        }
        send_api_error(res, send, ApiVariant::InternalServerError, "Something went wrong", "HE_00");
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

    /**  Local in-process mirror of upstream profile-scoped payment intent keys. */
    mutable std::unordered_map<std::string, std::string> m_profile_scope{};

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::payments
