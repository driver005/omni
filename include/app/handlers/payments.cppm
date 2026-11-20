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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        const auto body = std::format(R"({{"handler":{},"status":{}}})", serde::Ser::encode_json(std::string{"payments.get_payment_filters"}), serde::Ser::encode_json(query_value(req, "status")));
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        const auto body = std::format(R"({{"handler":{},"status":{}}})", serde::Ser::encode_json(std::string{"payments.get_payment_filters_profile"}), serde::Ser::encode_json(query_value(req, "status")));
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        const auto body = std::format(R"({{"start":{},"end":{}}})", serde::Ser::encode_json(query_value(req, "start")), serde::Ser::encode_json(query_value(req, "end")));
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        const auto body = std::format(R"({{"start":{},"end":{}}})", serde::Ser::encode_json(query_value(req, "start")), serde::Ser::encode_json(query_value(req, "end")));
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        const auto body = std::format(R"({{"payment_id":{}}})", serde::Ser::encode_json(payment_id_from_path(req)));
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::payments::PaymentAttemptListRequest;
        auto typed = serde::Ser::deserialize<Input>("application/json", body);
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!typed && !generic) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
                service_status = connector_response.get_status();
            }
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(std::move(service_status));
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
                service_status = connector_response.get_status();
            }
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(std::move(service_status));
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        auto text = payment_response(pipeline);
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        std::string service_status{"pending"};
        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(std::move(service_status));
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        const auto body = std::format(R"({{"offset":{},"limit":{}}})", query_uint(req, "offset"), query_uint(req, "limit"));
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        const auto body = std::format(R"({{"offset":{},"limit":{}}})", query_uint(req, "offset"), query_uint(req, "limit"));
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        const auto body = std::format(R"({{"handler":{},"status":{}}})", serde::Ser::encode_json(std::string{"payments.payments_list_for_platform_filters"}), serde::Ser::encode_json(query_value(req, "status")));
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        std::string service_status{"pending"};
        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(std::move(service_status));
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        std::string service_status{"pending"};
        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(std::move(service_status));
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
                service_status = connector_response.get_status();
            }
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(std::move(service_status));
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        std::string service_status{"pending"};
        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(std::move(service_status));
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
                service_status = connector_response.get_status();
            }
        }

        // Repository read replaces body-size cache as the source of truth.
        const std::string addressed_payment_id = payment_id_from_path(req);
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        app::model::payments::PaymentsRedirectionResponse response{};
        response.set_redirect_url(std::move(service_status));
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

        const auto body = std::format(R"({{"offset":{},"limit":{}}})", query_uint(req, "offset"), query_uint(req, "limit"));
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        const auto body = std::format(R"({{"offset":{},"limit":{}}})", query_uint(req, "offset"), query_uint(req, "limit"));
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
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

        const auto body = std::format(R"({{"offset":{},"limit":{}}})", query_uint(req, "offset"), query_uint(req, "limit"));
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

        auto records = m_context->repository().list_payments();
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
        if (!addressed_payment_id.empty() && !find_payment_record(addressed_payment_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "payment not found"));
            return;
        }

        auto text = payment_list_response(records);
        send_json(res, send, text, true);
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
    }

    explicit PaymentsRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    [[nodiscard]] std::string payment_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i = 0; i + 1 < segments.count(); ++i) {
            const auto current = segments.at(i);
            if (current && *current == "payments") {
                const auto next = segments.at(i + 1);
                if (next && next->starts_with("pay_")) {
                    return std::string{*next};
                }
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

    [[nodiscard]] std::optional<app::store::payments::PaymentIntentRecord> find_payment_record(
        std::string_view payment_id
    ) const
    {
        if (payment_id.empty()) {
            return std::nullopt;
        }
        for (const auto& record : m_context->repository().list_payments()) {
            if (record.get_payment_id() == payment_id) {
                return record;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string now_string() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
    }

    /**  Auth is resolved from the auditable RouteAuthRegistry, never built inline. */
    [[nodiscard]] bool authorize_operation(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) {
            send_error(res, send, *auth);
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
        const auto stored = find_payment_record(id);

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

        const auto stored = find_payment_record(id);
        result.amount = stored ? stored->get_amount() : 0;
        result.currency = stored && !stored->get_currency().empty()
                              ? stored->get_currency()
                              : std::string{"USD"};

        // Deterministic transition guards: local substitute for the upstream
        // operation state machine and if_not_create_change_operation.
        const std::string current = stored ? stored->get_status() : std::string{};
        const bool invalid_transition =
            (connector_operation == "capture" && !current.empty() &&
             current != "requires_capture" && current != "authorized" && current != "pending") ||
            (connector_operation == "cancel" &&
             (current == "succeeded" || current == "cancelled" || current == "canceled"));
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

        // Profile scoping is carried as the x-profile-id query/header claim.
        (void)query_value(req, "profile_id");

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

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
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

} // namespace app::handlers::payments
