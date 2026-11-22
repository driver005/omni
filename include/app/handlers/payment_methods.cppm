module;
export module app_handlers:payment_methods;

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
import app_locking;
import app_events;
import app_workflow;
import app_keystore;
import app_services;

export namespace app::handlers::payment_methods {

class PaymentMethodsRouteHandlers
{
public:
    explicit PaymentMethodsRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_create_payment_method_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("create_payment_method_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        if (!legacy_access_allowed(req)) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "legacy endpoint access denied")); return; }
        if (const auto sdk_error = authorize_client_facing(req, "create_payment_method_api")) { send_error(res, send, *sdk_error); return; }

        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = app::model::payment_methods::PaymentMethodCreate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        if (!parsed->get_client_secret()) {
            if (const auto secret = app::http::RequestQuery{}(req).get("client_secret")) {
                parsed->set_client_secret(*secret);
            }
        }
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id")); return; }
        if (parsed->get_customer_id() && !customer_exists(parsed->get_customer_id()->get_value())) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Customer does not exist in our records")); return;
        }
        std::string last4;
        std::string network;
        std::string card_json = "{}";
        if (parsed->get_card()) {
            const auto& card = *parsed->get_card();
            const auto& number = card.get_card_number().get_value();
            if (number.size() >= 4) last4 = number.substr(number.size() - 4);
            if (card.get_card_network()) network = card.get_card_network()->get_value();
            card_json = "{\"number\":" + serde::Ser::encode_json(number) +
                        ",\"exp_month\":" + serde::Ser::encode_json(card.get_card_exp_month()) +
                        ",\"exp_year\":" + serde::Ser::encode_json(card.get_card_exp_year());
            if (card.get_card_cvc()) card_json += ",\"cvc\":" + serde::Ser::encode_json(*card.get_card_cvc());
            if (card.get_card_holder_name()) card_json += ",\"holder\":" + serde::Ser::encode_json(*card.get_card_holder_name());
            card_json += "}";
        }
        if (parsed->get_billing()) {
            m_context->cache().put("payment_methods.billing:" + m_context->ids().uuid_v4(),
                core::cache::to_cache_value(encrypt_data(serde::Ser::encode_json(*parsed->get_billing()))));
        }
        const auto fingerprint = merchant_fingerprint(merchant_id, last4, network);
        auto repository = m_context->repository();
        const auto duplicate = m_context->cache().get("payment_methods.fingerprint:" + fingerprint);
        if (duplicate.hit) {
            const auto existing = repository.find_payment_method_by_string(core::cache::value_as<std::string>(duplicate.value));
            if (existing) {
                auto response = response_from_record(*existing);
                response.set_client_secret(issued_token(existing->get_payment_method_id()));
                send_json(res, send, serde::Ser::encode_json(response), true);
                return;
            }
        }
        const std::string payment_method_id = m_context->ids().payment_id("pm_");
        const auto vault_response = call_connector(merchant_id, "tokenize", payment_method_id, card_json);
        const std::string vault_id = vault_response.get_reference_id().empty()
                                        ? fingerprint.substr(0, 16)
                                        : vault_response.get_reference_id();
        if (card_json != "{}") {
            m_context->cache().put("payment_methods.card:" + payment_method_id,
                core::cache::to_cache_value(encrypt_data(card_json)));
        }
        app::store::payment_methods::PaymentMethodRecord record{};
        record.set_payment_method_id(payment_method_id);
        record.set_merchant_id(merchant_id);
        if (parsed->get_customer_id()) record.set_customer_id(parsed->get_customer_id()->get_value());
        record.set_profile_id(app::auth::AuthContextBuilder{}(req).profile_id());
        record.set_type(parsed->get_payment_method() ? parsed->get_payment_method()->get_value() : std::string{"card"});
        record.set_network(network);
        record.set_last4(last4);
        record.set_vault_id(vault_id);
        record.set_status("active");
        record.set_created_at(now_value());
        auto stored = repository.create_payment_method(std::move(record));
        if (!stored) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::ALREADY_EXISTS, "payment method already exists")); return; }
        m_context->cache().put("payment_methods.fingerprint:" + fingerprint, core::cache::to_cache_value(payment_method_id));
        const std::string client_secret = issued_token(payment_method_id);
        remember_client_secret(payment_method_id, client_secret);
        schedule_status_task(payment_method_id, merchant_id);
        publish_pm_event("payment_method.create", payment_method_id);
        m_context->metrics().record_flow("create_payment_method_api");
        m_context->metrics().counter("PAYMENT_METHOD_CREATE_PAYMENT_METHOD_API").increment();
        auto response = response_from_record(*stored);
        if (parsed->get_payment_method_type()) response.set_payment_method_type(*parsed->get_payment_method_type());
        if (parsed->get_metadata()) response.set_metadata(*parsed->get_metadata());
        response.set_client_secret(client_secret);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_get_pm_nt_eligibility_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_pm_nt_eligibility_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const auto query = app::http::RequestQuery{}(req);
        const auto card_bin = query.get("card_bin").value_or("");
        const std::string org_id = app::http::org_id_from_header{}(req);
        if (card_bin.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing card_bin")); return; }
        // Upstream core/payment_methods.rs:1777-1781 — when(!profile.is_network_tokenization_enabled,
        // ApiErrorResponse::NotSupported { "Network tokenization is not enabled for this profile" }).
        if (const auto nt_enabled = m_context->config_store().get_value("payment_methods", "network_tokenization_enabled");
            nt_enabled && (*nt_enabled == "false" || *nt_enabled == "disabled")) {
            send_local_error(res, send, "NOT_SUPPORTED", "Network tokenization is not enabled for this profile", interfaces::io::types::Status::BAD_REQUEST); return;
        }
        const auto connector_response = call_connector(org_id, "eligibility", card_bin, card_bin);
        m_context->metrics().record_flow("get_pm_nt_eligibility_api");
        m_context->metrics().counter("PAYMENT_METHOD_GET_PM_NT_ELIGIBILITY_API").increment();
        // Upstream core/payment_methods.rs:1788-1792 — eligible = response.tokenize_support
        // returned by network_tokenization::make_nt_eligibility_call.
        bool eligible = false;
        const std::string raw = connector_response.get_raw_json();
        if (const auto document = serde::Ser::decode_generic("application/json", raw)) {
            if (const auto* object = std::get_if<interfaces::Value::Object>(&document->get())) {
                for (const auto& [name, field] : *object) {
                    if (name == "tokenize_support") {
                        if (const auto* flag = std::get_if<bool>(&field.get())) { eligible = *flag; }
                        break;
                    }
                }
            }
        }
        if (raw.empty()) { eligible = !connector_response.get_status().empty(); }
        app::model::payment_methods::GetNetworkTokenEiligibilityResponse response{};
        response.set_eligible_for_network_tokenization(eligible);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_create_payment_method_intent_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("create_payment_method_intent_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        if (!legacy_access_allowed(req)) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "legacy endpoint access denied")); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = app::model::payment_methods::PaymentMethodIntentCreate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id")); return; }
        const std::string& customer_id = parsed->get_customer_id().get_value();
        if (!customer_id.empty() && !customer_exists(customer_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Customer does not exist in our records")); return;
        }
        const std::string payment_method_id = global_id("pmi_", merchant_id + customer_id);
        app::store::payment_methods::PaymentMethodRecord record{};
        record.set_payment_method_id(payment_method_id);
        record.set_merchant_id(merchant_id);
        record.set_customer_id(customer_id);
        record.set_status("requires_confirmation");
        record.set_created_at(now_value());
        auto stored = m_context->repository().create_payment_method(std::move(record));
        if (!stored) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::ALREADY_EXISTS, "payment method intent exists")); return; }
        const std::string client_secret = issued_token(payment_method_id);
        remember_client_secret(payment_method_id, client_secret);
        publish_pm_event("payment_method_intent.create", payment_method_id);
        m_context->metrics().record_flow("create_payment_method_intent_api");
        m_context->metrics().counter("PAYMENT_METHOD_CREATE_PAYMENT_METHOD_INTENT_API").increment();
        auto response = response_from_record(*stored);
        if (parsed->get_metadata()) response.set_metadata(*parsed->get_metadata());
        response.set_client_secret(client_secret);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_payment_method_update_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("payment_method_update_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        if (!legacy_access_allowed(req)) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "legacy endpoint access denied")); return; }
        const std::string payment_method_id = path_segment(req, 1);
        if (payment_method_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing payment method id")); return; }
        auto repository = m_context->repository();
        auto record = repository.find_payment_method_by_string(payment_method_id);
        if (!record) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Payment method does not exist in our records")); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = app::model::payment_methods::PaymentMethodUpdate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        if (const auto secret_error = check_client_secret(payment_method_id, parsed->get_client_secret())) { send_error(res, send, *secret_error); return; }
        if (parsed->get_card()) {
            const auto vault_response = call_connector(record->get_merchant_id(), "tokenize", payment_method_id, serde::Ser::encode_json(*parsed->get_card()));
            if (!vault_response.get_reference_id().empty()) record->set_vault_id(vault_response.get_reference_id());
            m_context->cache().put("payment_methods.card:" + payment_method_id,
                core::cache::to_cache_value(encrypt_data(serde::Ser::encode_json(*parsed->get_card()))));
        }
        record->set_status("active");
        if (!repository.update_payment_method(*record)) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INTERNAL_ERROR, "persistence failed")); return; }
        publish_pm_event("payment_method.update", payment_method_id);
        m_context->metrics().record_flow("payment_method_update_api");
        m_context->metrics().counter("PAYMENT_METHOD_PAYMENT_METHOD_UPDATE_API").increment();
        app::model::payment_methods::PaymentMethodUpdateResponse response{};
        response.set_payment_method_id(payment_method_id);
        app::model::payment_methods::PaymentMethodStatusValue status{};
        status.set_value(record->get_status());
        response.set_status(std::move(status));
        response.set_update_status(app::model::payment_methods::UpdateStatus::Success);
        response.set_updated_payment_method_data(true);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_payment_method_retrieve_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("payment_method_retrieve_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        if (!legacy_access_allowed(req)) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "legacy endpoint access denied")); return; }
        const std::string payment_method_id = path_segment(req);
        if (payment_method_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing payment method id")); return; }
        auto repository = m_context->repository();
        auto record = repository.find_payment_method_by_string(payment_method_id);
        if (!record) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Payment method does not exist in our records")); return; }
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        if (!merchant_id.empty() && record->get_merchant_id() != merchant_id) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "payment method belongs to another merchant")); return;
        }
        if (app::http::RequestQuery{}(req).get("force_sync").has_value()) {
            const auto sync = call_connector(record->get_merchant_id(), "sync", payment_method_id, record->get_vault_id());
            if (!sync.get_status().empty()) { record->set_status(sync.get_status()); repository.update_payment_method(*record); }
        }
        m_context->metrics().record_flow("payment_method_retrieve_api");
        m_context->metrics().counter("PAYMENT_METHOD_PAYMENT_METHOD_RETRIEVE_API").increment();
        send_json(res, send, serde::Ser::encode_json(response_from_record(*record)), true);
    }


    void handle_payment_method_delete_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("payment_method_delete_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        if (!legacy_access_allowed(req)) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "legacy endpoint access denied")); return; }
        const std::string payment_method_id = path_segment(req);
        if (payment_method_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing payment method id")); return; }
        auto repository = m_context->repository();
        const auto record = repository.find_payment_method_by_string(payment_method_id);
        if (!record) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Payment method does not exist in our records")); return; }
        const auto response_vault = call_connector(record->get_merchant_id(), "delete_token", payment_method_id, record->get_vault_id());
        (void)response_vault;
        if (!repository.remove_payment_method(record->get_id())) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INTERNAL_ERROR, "persistence failed")); return; }
        m_context->cache().erase("payment_methods.client_secret:" + payment_method_id);
        m_context->cache().erase("payment_methods.card:" + payment_method_id);
        publish_pm_event("payment_method.delete", payment_method_id);
        m_context->metrics().record_flow("payment_method_delete_api");
        m_context->metrics().counter("PAYMENT_METHOD_PAYMENT_METHOD_DELETE_API").increment();
        app::model::payment_methods::PaymentMethodDeleteResponse response{};
        response.set_payment_method_id(payment_method_id);
        response.set_deleted(true);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_migrate_payment_method_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("migrate_payment_method_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = app::model::payment_methods::PaymentMethodMigrate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        std::string merchant_id = app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) merchant_id = parsed->get_merchant_id().get_value();
        if (merchant_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id")); return; }
        if (auto mca_error = check_merchant_connector(merchant_id)) { send_error(res, send, *mca_error); return; }
        const std::string payment_method_id = global_id("pm_", merchant_id + parsed->get_merchant_id().get_value());
        std::string last4;
        std::string network;
        if (parsed->get_card()) {
            const auto& number = parsed->get_card()->get_card_number();
            if (number.size() >= 4) last4 = number.substr(number.size() - 4);
            if (parsed->get_card()->get_card_network()) network = parsed->get_card()->get_card_network()->get_value();
        }
        app::store::payment_methods::PaymentMethodRecord record{};
        record.set_payment_method_id(payment_method_id);
        record.set_merchant_id(merchant_id);
        if (parsed->get_customer_id()) record.set_customer_id(parsed->get_customer_id()->get_value());
        record.set_type(parsed->get_payment_method() ? parsed->get_payment_method()->get_value() : std::string{"card"});
        record.set_network(network);
        record.set_last4(last4);
        // Upstream migrate_payment_method encrypts card data with the key manager and writes it
        // to payment_method_data. Local substitute: key-store-backed encryption in the cache.
        if (parsed->get_card()) {
            const auto encrypted = encrypt_data(serde::Ser::encode_json(*parsed->get_card()));
            m_context->cache().put("payment_methods.card:" + payment_method_id, core::cache::to_cache_value(encrypted));
        }
        record.set_vault_id(local_vault_token(merchant_id, payment_method_id));
        record.set_status("active");
        record.set_created_at(now_value());
        auto stored = m_context->repository().create_payment_method(std::move(record));
        if (!stored) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::ALREADY_EXISTS, "payment method already exists")); return; }
        m_context->metrics().record_flow("migrate_payment_method_api");
        m_context->metrics().counter("PAYMENT_METHOD_MIGRATE_PAYMENT_METHOD_API").increment();
        auto migrated = response_from_record(*stored);
        if (parsed->get_payment_method_type()) migrated.set_payment_method_type(*parsed->get_payment_method_type());
        app::model::payment_methods::PaymentMethodMigrateResponse response{};
        response.set_payment_method_response(std::move(migrated));
        response.set_card_migrated(parsed->get_card().has_value());
        response.set_payment_method_migrated(true);
        response.set_network_token_migrated(parsed->get_network_token().has_value());
        response.set_connector_mandate_details_migrated(parsed->get_connector_mandate_details().has_value());
        response.set_network_transaction_id_migrated(parsed->get_network_transaction_id().has_value());
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_migrate_payment_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("migrate_payment_methods");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = std::vector<app::model::payment_methods::PaymentMethodMigrate>;
        auto parsed = serde::Ser::deserialize<Input>("application/json", parse_batch_rows(body));
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        if (parsed->empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "no rows")); return; }
        if (parsed->size() > 200) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::VALIDATION_FAILED, "batch limit is 200")); return; }
        m_context->metrics().record_flow("migrate_payment_methods");
        m_context->metrics().counter("PAYMENT_METHOD_MIGRATE_PAYMENT_METHODS").increment();
        app::model::payment_methods::ModularPaymentMethodMigrationResponse response{};
        response.set_total_rows(parsed->size());
        std::vector<app::model::payment_methods::ModularPaymentMethodMigrationRowResult> results{};
        std::size_t successful{0};
        auto repository = m_context->repository();
        for (std::size_t i = 0; i < parsed->size(); ++i) {
            app::model::payment_methods::ModularPaymentMethodMigrationRowResult row{};
            row.set_row_number(i + 1);
            const auto& source = (*parsed)[i];
            const auto merchant_id = source.get_merchant_id().get_value();
            app::model::payment_methods::MerchantIdValue row_merchant{};
            row_merchant.set_value(merchant_id);
            row.set_merchant_id(std::move(row_merchant));
            if (auto mca_error = check_merchant_connector(merchant_id)) {
                row.set_migration_status(app::model::payment_methods::MigrationStatus::Failed);
                row.set_error_message(mca_error->get_message());
                results.push_back(std::move(row));
                continue;
            }
            if (source.get_customer_id() && !customer_exists(source.get_customer_id()->get_value())) {
                row.set_migration_status(app::model::payment_methods::MigrationStatus::Failed);
                row.set_error_message("Customer does not exist in our records");
                results.push_back(std::move(row));
                continue;
            }
            std::string last4;
            std::string network;
            if (source.get_card()) {
                const auto& number = source.get_card()->get_card_number();
                if (number.size() >= 4) last4 = number.substr(number.size() - 4);
                if (source.get_card()->get_card_network()) network = source.get_card()->get_card_network()->get_value();
            }
            app::store::payment_methods::PaymentMethodRecord record{};
            record.set_payment_method_id(global_id("pm_", merchant_id + std::to_string(i)));
            record.set_merchant_id(merchant_id);
            if (source.get_customer_id()) record.set_customer_id(source.get_customer_id()->get_value());
            record.set_type(source.get_payment_method() ? source.get_payment_method()->get_value() : std::string{"card"});
            record.set_network(network);
            record.set_last4(last4);
            record.set_vault_id(merchant_fingerprint(merchant_id, last4, network).substr(0, 16));
            record.set_status("active");
            record.set_created_at(now_value());
            const auto stored = repository.create_payment_method(std::move(record));
            if (!stored) {
                row.set_migration_status(app::model::payment_methods::MigrationStatus::Failed);
                row.set_error_message("persistence failed");
            } else {
                row.set_payment_method_id(stored->get_payment_method_id());
                row.set_migration_status(app::model::payment_methods::MigrationStatus::Success);
                ++successful;
            }
            results.push_back(std::move(row));
        }
        response.set_successful_count(successful);
        response.set_failed_count(parsed->size() - successful);
        response.set_results(std::move(results));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_update_payment_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("update_payment_methods");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = std::vector<app::model::payment_methods::UpdatePaymentMethodRecord>;
        auto parsed = serde::Ser::deserialize<Input>("application/json", parse_batch_rows(body));
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        if (parsed->empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "no records")); return; }
        if (parsed->size() > 200) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::VALIDATION_FAILED, "batch limit is 200")); return; }
        m_context->metrics().record_flow("update_payment_methods");
        m_context->metrics().counter("PAYMENT_METHOD_UPDATE_PAYMENT_METHODS").increment();
        auto repository = m_context->repository();
        for (const auto& update : *parsed) {
            auto record = repository.find_payment_method_by_string(update.get_payment_method_id());
            if (!record) continue;
            if (update.get_status()) record->set_status(update.get_status()->get_value());
            repository.update_payment_method(*record);
        }
        const auto& first = parsed->front();
        app::model::payment_methods::PaymentMethodRecordUpdateResponse response{};
        response.set_payment_method_id(first.get_payment_method_id());
        app::model::payment_methods::PaymentMethodStatusValue status{};
        status.set_value(first.get_status() ? first.get_status()->get_value() : "active");
        response.set_status(std::move(status));
        if (first.get_network_transaction_id()) response.set_network_transaction_id(*first.get_network_transaction_id());
        response.set_updated_payment_method_data(true);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_payment_methods_batch_retrieve_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("payment_methods_batch_retrieve_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        // Upstream payment_methods.rs:634-655 — parse the multipart CSV, reject empty and
        // >200 id lists with InvalidRequestData; batch_retrieve.rs:66-146 returns one response
        // per row with an error_message for ids that are not found.
        const auto body = req.body_as_string();
        const auto rows = batch_retrieve_rows(body);
        if (rows.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "No payment_method_ids provided")); return; }
        if (rows.size() > 200) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "A maximum of 200 payment_method_ids are allowed per request")); return; }
        m_context->metrics().record_flow("payment_methods_batch_retrieve_api");
        m_context->metrics().counter("PAYMENT_METHOD_PAYMENT_METHODS_BATCH_RETRIEVE_API").increment();
        auto repository = m_context->repository();
        std::vector<app::model::payment_methods::PaymentMethodsBatchRetrieveResponse> responses{};
        for (const auto& [payment_method_id, line_number] : rows) {
            app::model::payment_methods::PaymentMethodsBatchRetrieveResponse response{};
            response.set_payment_method_id(payment_method_id);
            response.set_line_number(line_number);
            const auto record = repository.find_payment_method_by_string(payment_method_id);
            if (!record) {
                response.set_error_message(std::string{"Payment method not found"});
            } else {
                if (!record->get_type().empty()) {
                    app::model::payment_methods::PaymentMethodValue method{};
                    method.set_value(record->get_type());
                    response.set_payment_method_type(std::move(method));
                }
            }
            responses.push_back(std::move(response));
        }
        send_json(res, send, serde::Ser::encode_json(responses), true);
    }


    void handle_save_payment_method_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("save_payment_method_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        if (!legacy_access_allowed(req)) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "legacy endpoint access denied")); return; }
        if (const auto sdk_error = authorize_client_facing(req, "save_payment_method_api")) { send_error(res, send, *sdk_error); return; }
        const std::string payment_method_id = path_segment(req, 1);
        if (payment_method_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing payment method id")); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = app::model::payment_methods::PaymentMethodCreate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        if (!parsed->get_client_secret()) {
            if (const auto secret = app::http::RequestQuery{}(req).get("client_secret")) parsed->set_client_secret(*secret);
        }
        if (const auto secret_error = check_client_secret(payment_method_id, parsed->get_client_secret())) { send_error(res, send, *secret_error); return; }
        auto repository = m_context->repository();
        auto record = repository.find_payment_method_by_string(payment_method_id);
        if (!record) {
            const std::string merchant_id = app::http::merchant_id_from_header{}(req);
            if (merchant_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id")); return; }
            app::store::payment_methods::PaymentMethodRecord created{};
            created.set_payment_method_id(payment_method_id);
            created.set_merchant_id(merchant_id);
            if (parsed->get_customer_id()) created.set_customer_id(parsed->get_customer_id()->get_value());
            created.set_type(parsed->get_payment_method() ? parsed->get_payment_method()->get_value() : std::string{"card"});
            created.set_status("active");
            created.set_created_at(now_value());
            auto stored = repository.create_payment_method(std::move(created));
            if (!stored) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::ALREADY_EXISTS, "payment method exists")); return; }
            record = *stored;
        }
        if (parsed->get_card()) {
            const auto vault = call_connector(record->get_merchant_id(), "tokenize", payment_method_id, serde::Ser::encode_json(*parsed->get_card()));
            if (!vault.get_reference_id().empty()) record->set_vault_id(vault.get_reference_id());
            m_context->cache().put("payment_methods.card:" + payment_method_id,
                core::cache::to_cache_value(encrypt_data(serde::Ser::encode_json(*parsed->get_card()))));
        }
        record->set_status("active");
        repository.update_payment_method(*record);
        publish_pm_event("payment_method.save", payment_method_id);
        m_context->metrics().record_flow("save_payment_method_api");
        m_context->metrics().counter("PAYMENT_METHOD_SAVE_PAYMENT_METHOD_API").increment();
        send_json(res, send, serde::Ser::encode_json(response_from_record(*record)), true);
    }


    void handle_list_payment_method_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("list_payment_method_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        if (const auto sdk_error = authorize_client_facing(req, "list_payment_method_api")) { send_error(res, send, *sdk_error); return; }
        const auto query = app::http::RequestQuery{}(req);
        app::model::payment_methods::PaymentMethodListRequest input{};
        if (const auto value = query.get("client_secret")) input.set_client_secret(*value);
        if (const auto value = query.get("amount")) {
            app::model::payment_methods::MinorUnitValue amount{};
            amount.set_value(*value);
            input.set_amount(std::move(amount));
        }
        std::size_t limit{20};
        if (const auto value = query.get("limit")) {
            std::int64_t parsed_limit{};
            const auto converted = std::from_chars(value->data(), value->data() + value->size(), parsed_limit);
            if (converted.ec == std::errc{} && parsed_limit > 0) limit = static_cast<std::size_t>(parsed_limit);
        }
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        if (const auto secret_error = check_client_secret("list", input.get_client_secret())) { send_error(res, send, *secret_error); return; }
        m_context->metrics().record_flow("list_payment_method_api");
        m_context->metrics().counter("PAYMENT_METHOD_LIST_PAYMENT_METHOD_API").increment();
        app::model::payment_methods::PaymentMethodListResponse response{};
        std::vector<app::model::payment_methods::ResponsePaymentMethodsEnabled> enabled{};
        for (const auto& record : m_context->repository().list_payment_methods()) {
            if (enabled.size() >= limit) break;
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) continue;
            app::model::payment_methods::ResponsePaymentMethodsEnabled item{};
            app::model::payment_methods::PaymentMethodValue method{};
            method.set_value(record.get_type().empty() ? std::string{"card"} : record.get_type());
            item.set_payment_method(std::move(method));
            enabled.push_back(std::move(item));
        }
        response.set_payment_methods(std::move(enabled));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_list_customer_payment_method_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("list_customer_payment_method_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        if (!legacy_access_allowed(req)) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "legacy endpoint access denied")); return; }
        const std::string customer_id = path_segment(req, 1);
        if (customer_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing customer id")); return; }
        if (!customer_exists(customer_id)) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Customer does not exist in our records")); return; }
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        m_context->metrics().record_flow("list_customer_payment_method_api");
        m_context->metrics().counter("PAYMENT_METHOD_LIST_CUSTOMER_PAYMENT_METHOD_API").increment();
        app::model::payment_methods::CustomerPaymentMethodsListResponse response{};
        std::vector<app::model::payment_methods::CustomerPaymentMethod> items{};
        const auto default_key = "payment_methods.default:" + customer_id;
        const auto default_hit = m_context->cache().get(default_key);
        const std::string default_id = default_hit.hit ? core::cache::value_as<std::string>(default_hit.value) : std::string{};
        for (const auto& record : m_context->repository().list_payment_methods()) {
            if (record.get_customer_id() != customer_id) continue;
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) continue;
            app::model::payment_methods::CustomerPaymentMethod item{};
            item.set_payment_token(record.get_vault_id().empty() ? record.get_payment_method_id() : record.get_vault_id());
            item.set_payment_method_id(record.get_payment_method_id());
            app::model::payment_methods::CustomerIdValue customer{};
            customer.set_value(customer_id);
            item.set_customer_id(std::move(customer));
            app::model::payment_methods::PaymentMethodValue method{};
            method.set_value(record.get_type().empty() ? std::string{"card"} : record.get_type());
            item.set_payment_method(std::move(method));
            item.set_default_payment_method_set(record.get_payment_method_id() == default_id);
            app::model::payment_methods::PrimitiveDateTimeValue created{};
            created.set_value(record.get_created_at());
            item.set_created(std::move(created));
            items.push_back(std::move(item));
        }
        response.set_customer_payment_methods(std::move(items));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_list_customer_payment_method_api_client(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("list_customer_payment_method_api_client");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        if (const auto sdk_error = authorize_client_facing(req, "list_customer_payment_method_api_client")) { send_error(res, send, *sdk_error); return; }
        const auto query = app::http::RequestQuery{}(req);
        const std::string customer_id = query.get("customer_id").value_or("");
        const std::string client_secret = query.get("client_secret").value_or("");
        if (!customer_id.empty() && !customer_exists(customer_id)) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Customer does not exist in our records")); return; }
        if (const auto secret_error = check_client_secret(customer_id, client_secret.empty() ? std::nullopt : std::optional<std::string>{client_secret})) { send_error(res, send, *secret_error); return; }
        m_context->metrics().record_flow("list_customer_payment_method_api_client");
        m_context->metrics().counter("PAYMENT_METHOD_LIST_CUSTOMER_PAYMENT_METHOD_API_CLIENT").increment();
        app::model::payment_methods::ClientPaymentMethodsListResponse response{};
        std::vector<app::model::payment_methods::CustomerPaymentMethodForClient> items{};
        for (const auto& record : m_context->repository().list_payment_methods()) {
            if (!customer_id.empty() && record.get_customer_id() != customer_id) continue;
            app::model::payment_methods::CustomerPaymentMethodForClient item{};
            item.set_payment_token(record.get_vault_id().empty() ? record.get_payment_method_id() : record.get_vault_id());
            app::model::payment_methods::PaymentMethodValue method{};
            method.set_value(record.get_type().empty() ? std::string{"card"} : record.get_type());
            item.set_payment_method(std::move(method));
            app::model::payment_methods::PrimitiveDateTimeValue created{};
            created.set_value(record.get_created_at());
            item.set_created(std::move(created));
            items.push_back(std::move(item));
        }
        response.set_customer_payment_methods(std::move(items));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_initiate_pm_collect_link_flow(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("initiate_pm_collect_link_flow");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = app::model::payment_methods::PaymentMethodCollectLinkRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id")); return; }
        if (!customer_exists(parsed->get_customer_id().get_value())) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Customer does not exist in our records")); return; }
        const std::string collect_id = m_context->ids().payment_id("pml_");
        const std::string token = issued_token(collect_id);
        remember_client_secret(collect_id, token);
        m_context->cache().put("payment_methods.collect:" + collect_id, core::cache::to_cache_value(encrypt_data(token)), core::cache::Ttl{}.seconds(3600));
        const auto connector_response = call_connector(merchant_id, "create_link", collect_id, body);
        std::string link = connector_response.get_raw_json();
        if (link.empty()) link = "/payment_methods/collect/" + merchant_id + "/" + collect_id + "?token=" + token;
        m_context->metrics().record_flow("initiate_pm_collect_link_flow");
        m_context->metrics().counter("PAYMENT_METHOD_INITIATE_PM_COLLECT_LINK_FLOW").increment();
        app::model::payment_methods::PaymentMethodCollectLinkResponse response{};
        response.set_pm_collect_link_id(collect_id);
        response.set_customer_id(parsed->get_customer_id());
        app::model::payment_methods::PrimitiveDateTimeValue expiry{};
        expiry.set_value(std::to_string(std::stoll(now_value()) + 3600));
        response.set_expiry(std::move(expiry));
        response.set_link(link);
        if (parsed->get_return_url()) response.set_return_url(*parsed->get_return_url());
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_payment_method_retrieve_olap_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("payment_method_retrieve_olap_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const std::string payment_method_id = path_segment(req, 1);
        if (payment_method_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing payment method id")); return; }
        const auto record = m_context->repository().find_payment_method_by_string(payment_method_id);
        if (!record) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Payment method does not exist in our records")); return; }
        m_context->metrics().record_flow("payment_method_retrieve_olap_api");
        m_context->metrics().counter("PAYMENT_METHOD_PAYMENT_METHOD_RETRIEVE_OLAP_API").increment();
        app::model::payment_methods::PaymentMethodDetailsResponse response{};
        app::model::payment_methods::GlobalPaymentMethodIdValue id{};
        id.set_value(record->get_payment_method_id());
        response.set_id(std::move(id));
        app::model::payment_methods::MerchantIdValue merchant{};
        merchant.set_value(record->get_merchant_id());
        response.set_merchant_id(std::move(merchant));
        if (!record->get_customer_id().empty()) {
            app::model::payment_methods::GlobalCustomerIdValue customer{};
            customer.set_value(record->get_customer_id());
            response.set_customer_id(std::move(customer));
        }
        app::model::payment_methods::PrimitiveDateTimeValue created{};
        created.set_value(record->get_created_at());
        response.set_created(std::move(created));
        app::model::payment_methods::StorageTypeValue storage{};
        storage.set_value("payment_methods");
        response.set_storage_type(std::move(storage));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_get_payment_method_token_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_payment_method_token_data");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const std::string payment_method_id = path_segment(req, 1);
        if (payment_method_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing payment method id")); return; }
        const auto record = m_context->repository().find_payment_method_by_string(payment_method_id);
        if (!record) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Payment method does not exist in our records")); return; }
        const std::string token_type = app::http::RequestQuery{}(req).get("token_type").value_or("network_token");
        const auto vault = call_connector(record->get_merchant_id(), "sync", payment_method_id, record->get_vault_id());
        const std::string token = vault.get_reference_id().empty() ? record->get_vault_id() : vault.get_reference_id();
        m_context->metrics().record_flow("get_payment_method_token_data");
        m_context->metrics().counter("PAYMENT_METHOD_GET_PAYMENT_METHOD_TOKEN_DATA").increment();
        app::model::payment_methods::TokenDataResponse response{};
        app::model::payment_methods::GlobalPaymentMethodIdValue id{};
        id.set_value(payment_method_id);
        response.set_payment_method_id(std::move(id));
        app::model::payment_methods::TokenDataTypeValue type{};
        type.set_value(token_type);
        response.set_token_type(std::move(type));
        const auto encrypted = m_context->cache().get("payment_methods.card:" + payment_method_id);
        if (encrypted.hit) {
            const auto card_token = decrypt_data(core::cache::value_as<std::string>(encrypted.value));
            (void)card_token;
        }
        response.set_token_details(app::model::payment_methods::TokenDetailsResponse::NetworkTokenDetails);
        // ParentPaymentMethodToken store maps the temporary token to its PM id (see
        // handle_payment_method_get_token_details_api / core/payment_methods.rs:8596-8608).
        m_context->cache().put("payment_methods.token:" + token, core::cache::to_cache_value(payment_method_id));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_get_total_payment_method_count(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_total_payment_method_count");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const auto query = app::http::RequestQuery{}(req);
        const std::string customer_id = query.get("customer_id").value_or("");
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        if (!customer_id.empty() && !customer_exists(customer_id)) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Customer does not exist in our records")); return; }
        m_context->metrics().record_flow("get_total_payment_method_count");
        m_context->metrics().counter("PAYMENT_METHOD_GET_TOTAL_PAYMENT_METHOD_COUNT").increment();
        std::int64_t count{0};
        for (const auto& record : m_context->repository().list_payment_methods()) {
            if (!customer_id.empty() && record.get_customer_id() != customer_id) continue;
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) continue;
            ++count;
        }
        app::model::payment_methods::TotalPaymentMethodCountResponse response{};
        response.set_total_count(count);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_render_pm_collect_link(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("render_pm_collect_link");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const std::string merchant_id = path_segment(req, 1);
        const std::string collect_id = path_segment(req);
        if (merchant_id.empty() || collect_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "collect link not found")); return; }
        const auto hit = m_context->cache().get("payment_methods.collect:" + collect_id);
        if (!hit.hit) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "collect link not found")); return; }
        const auto presented = app::http::RequestQuery{}(req).get("token");
        if (presented) {
            if (const auto secret_error = check_client_secret(collect_id, *presented)) { send_error(res, send, *secret_error); return; }
        }
        m_context->metrics().record_flow("render_pm_collect_link");
        m_context->metrics().counter("PAYMENT_METHOD_RENDER_PM_COLLECT_LINK").increment();
        app::model::payment_methods::PaymentMethodCollectLinkRenderRequest response{};
        app::model::payment_methods::MerchantIdValue response_merchant{};
        response_merchant.set_value(merchant_id);
        response.set_merchant_id(std::move(response_merchant));
        response.set_pm_collect_link_id(collect_id);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_list_countries_currencies_for_connector_payment_method(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("list_countries_currencies_for_connector_payment_method");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const auto query = app::http::RequestQuery{}(req);
        const std::string connector = query.get("connector").value_or("");
        const std::string payment_method_type = query.get("payment_method_type").value_or("");
        // Upstream cards.rs:7801-7856 — pm_filters[connector][payment_method_type] with a
        // "default" fallback; when no filter is configured the full Currency / CountryAlpha2
        // enum sets are returned. The config store is the local substitute for state.conf.pm_filters.
        std::string filters_json{};
        if (const auto value = m_context->config_store().get_value("pm_filters", connector); value && !value->empty()) {
            filters_json = *value;
        } else if (const auto value = m_context->config_store().get_value("pm_filters", "default"); value && !value->empty()) {
            filters_json = *value;
        }
        std::vector<std::string> currency_codes{};
        std::vector<std::pair<std::string, std::string>> country_entries{};
        bool configured{false};
        if (!filters_json.empty()) {
            if (const auto doc = serde::Ser::decode_generic("application/json", filters_json)) {
                const interfaces::Value::Object* by_type{nullptr};
                if (const auto* root = std::get_if<interfaces::Value::Object>(&doc->get())) {
                    for (const auto& [name, field] : *root) {
                        if (name == payment_method_type || name == "default") {
                            if (const auto* object = std::get_if<interfaces::Value::Object>(&field.get())) {
                                by_type = object;
                                if (name == payment_method_type) break;
                            }
                        }
                    }
                }
                if (by_type != nullptr) {
                    for (const auto& [name, field] : *by_type) {
                        if (const auto* array = std::get_if<interfaces::Value::Array>(&field.get())) {
                            for (const auto& element : *array) {
                                if (const auto* text = std::get_if<std::string>(&element.get())) {
                                    if (name == "currency") currency_codes.push_back(*text);
                                    else if (name == "country") country_entries.emplace_back(*text, *text);
                                } else if (const auto* object = std::get_if<interfaces::Value::Object>(&element.get())) {
                                    std::string code{};
                                    std::string name_value{};
                                    for (const auto& [key, value] : *object) {
                                        if (const auto* text = std::get_if<std::string>(&value.get())) {
                                            if (key == "code") code = *text;
                                            else if (key == "name") name_value = *text;
                                        }
                                    }
                                    if (!code.empty()) country_entries.emplace_back(code, name_value.empty() ? code : name_value);
                                }
                            }
                        }
                    }
                    configured = !currency_codes.empty() || !country_entries.empty();
                }
            }
        }
        m_context->metrics().record_flow("list_countries_currencies_for_connector_payment_method");
        m_context->metrics().counter("PAYMENT_METHOD_LIST_COUNTRIES_CURRENCIES_FOR_CONNECTOR_PAYMENT_METHOD").increment();
        app::model::payment_methods::ListCountriesCurrenciesResponse response{};
        if (configured) {
            std::vector<app::model::payment_methods::CurrencyValue> currencies{};
            for (const auto& code : currency_codes) {
                app::model::payment_methods::CurrencyValue value{}; value.set_value(code);
                currencies.push_back(std::move(value));
            }
            std::vector<app::model::payment_methods::CountryCodeWithName> countries{};
            for (const auto& [code, name] : country_entries) {
                app::model::payment_methods::CountryAlpha2Value alpha{}; alpha.set_value(code);
                app::model::payment_methods::CountryValue country{}; country.set_value(name);
                app::model::payment_methods::CountryCodeWithName item{};
                item.set_code(std::move(alpha)); item.set_name(std::move(country));
                countries.push_back(std::move(item));
            }
            response.set_currencies(std::move(currencies));
            response.set_countries(std::move(countries));
        } else {
            // Upstream default: Currency::iter() / CountryAlpha2::iter() full sets.
            response.set_currencies(app::model::payment_methods::AllCurrencyCodes{ }());
            response.set_countries(app::model::payment_methods::AllCountryCodes{ }());
        }
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_default_payment_method_set_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("default_payment_method_set_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const std::string customer_id = path_segment(req, 3);
        const std::string path_payment_method_id = path_segment(req, 1);
        if (customer_id.empty() || !customer_exists(customer_id)) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Customer does not exist in our records")); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = app::model::payment_methods::DefaultPaymentMethod;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        const std::string payment_method_id = path_payment_method_id.empty() ? parsed->get_payment_method_id() : path_payment_method_id;
        if (payment_method_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing payment method id")); return; }
        const auto record = m_context->repository().find_payment_method_by_string(payment_method_id);
        if (!record) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Payment method does not exist in our records")); return; }
        m_context->cache().put("payment_methods.default:" + customer_id, core::cache::to_cache_value(payment_method_id));
        publish_pm_event("payment_method.set_default", payment_method_id);
        m_context->metrics().record_flow("default_payment_method_set_api");
        m_context->metrics().counter("PAYMENT_METHOD_DEFAULT_PAYMENT_METHOD_SET_API").increment();
        app::model::payment_methods::CustomerDefaultPaymentMethodResponse response{};
        response.set_default_payment_method_id(payment_method_id);
        app::model::payment_methods::CustomerIdValue customer{};
        customer.set_value(customer_id);
        response.set_customer_id(std::move(customer));
        app::model::payment_methods::PaymentMethodValue method{};
        method.set_value(record->get_type().empty() ? std::string{"card"} : record->get_type());
        response.set_payment_method(std::move(method));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_tokenize_card_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("tokenize_card_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = app::model::payment_methods::TokenizeCardRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        const auto payload = serde::Ser::encode_json(*parsed);
        const auto vault = call_connector(merchant_id, "tokenize", m_context->ids().uuid_v4(), payload);
        const std::string token = vault.get_reference_id().empty() ? issued_token(m_context->ids().uuid_v4()) : vault.get_reference_id();
        m_context->cache().put("payment_methods.card:" + token, core::cache::to_cache_value(encrypt_data(payload)));
        m_context->metrics().record_flow("tokenize_card_api");
        m_context->metrics().counter("PAYMENT_METHOD_TOKENIZE_CARD_API").increment();
        app::model::payment_methods::CardNetworkTokenizeResponse response{};
        response.set_card_tokenized(true);
        app::model::payment_methods::PaymentMethodResponse card_response = response_from_record([&]() {
            app::store::payment_methods::PaymentMethodRecord record{};
            record.set_merchant_id(merchant_id);
            record.set_payment_method_id(token);
            record.set_vault_id(token);
            return record;
        }());
        card_response.set_client_secret(token);
        response.set_payment_method_response(std::move(card_response));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_tokenize_card_using_pm_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("tokenize_card_using_pm_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const std::string payment_method_id = path_segment(req, 1);
        if (payment_method_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing payment method id")); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = app::model::payment_methods::TokenizePaymentMethodRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        const auto record = m_context->repository().find_payment_method_by_string(payment_method_id);
        if (!record) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Payment method does not exist in our records")); return; }
        const auto vault = call_connector(record->get_merchant_id(), "tokenize", payment_method_id, serde::Ser::encode_json(*parsed));
        const std::string token = vault.get_reference_id().empty() ? issued_token(payment_method_id) : vault.get_reference_id();
        m_context->cache().put("payment_methods.token:" + token, core::cache::to_cache_value(payment_method_id));
        m_context->metrics().record_flow("tokenize_card_using_pm_api");
        m_context->metrics().counter("PAYMENT_METHOD_TOKENIZE_CARD_USING_PM_API").increment();
        app::model::payment_methods::TokenizedCardValue1 response{};
        response.set_card_number("************" + record->get_last4());
        response.set_exp_year("");
        response.set_exp_month("");
        response.set_card_last_four(record->get_last4());
        response.set_card_token(token);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_tokenize_card_batch_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("tokenize_card_batch_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        auto parsed = serde::Ser::decode_generic("application/json", parse_batch_rows(body));
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        const auto array = std::get_if<interfaces::Value::Array>(&parsed->get());
        if (!array) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "expected an array of cards")); return; }
        if (array->size() > 200) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::VALIDATION_FAILED, "batch limit is 200")); return; }
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        std::vector<app::model::payment_methods::TokenizedCardValue1> results{};
        for (const auto& element : *array) {
            const auto payload = serde::Ser::encode_json(element);
            const auto vault = call_connector(merchant_id, "tokenize", m_context->ids().uuid_v4(), payload);
            const std::string token = vault.get_reference_id().empty() ? issued_token(m_context->ids().uuid_v4()) : vault.get_reference_id();
            m_context->cache().put("payment_methods.card:" + token, core::cache::to_cache_value(encrypt_data(payload)));
            app::model::payment_methods::TokenizedCardValue1 item{};
            item.set_card_number("************");
            item.set_exp_year("");
            item.set_exp_month("");
            item.set_card_token(token);
            results.push_back(std::move(item));
        }
        m_context->metrics().record_flow("tokenize_card_batch_api");
        m_context->metrics().counter("PAYMENT_METHOD_TOKENIZE_CARD_BATCH_API").increment();
        send_json(res, send, serde::Ser::encode_json(results), true);
    }


    void handle_payment_methods_session_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("payment_methods_session_create");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = app::model::payment_methods::PaymentMethodSessionRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        const std::string org_id = app::http::org_id_from_header{}(req);
        if (merchant_id.empty() && org_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant or organization id")); return; }
        if (parsed->get_customer_id() && !customer_exists(parsed->get_customer_id()->get_value())) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Customer does not exist in our records")); return; }
        const std::string session_id = global_id("pms_", merchant_id + org_id);
        const std::string client_secret = issued_token(session_id);
        const std::int64_t expires = std::stoll(now_value()) + static_cast<std::int64_t>(parsed->get_expires_in().value_or(1800u));
        app::store::payment_method_sessions::PaymentMethodSessionRecord record{};
        record.set_session_id(session_id);
        record.set_merchant_id(merchant_id.empty() ? org_id : merchant_id);
        record.set_client_secret(encrypt_data(client_secret));
        record.set_status("active");
        record.set_created_at(now_value());
        record.set_expires_at(std::to_string(expires));
        auto stored = m_context->repository().create_payment_method_session(std::move(record));
        if (!stored) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::ALREADY_EXISTS, "session already exists")); return; }
        remember_client_secret(session_id, client_secret);
        m_context->cache().put("payment_methods.session.storage:" + session_id, core::cache::to_cache_value(parsed->get_storage_type().get_value()));
        if (parsed->get_billing()) {
            m_context->cache().put("payment_methods.session.billing:" + session_id,
                core::cache::to_cache_value(encrypt_data(serde::Ser::encode_json(*parsed->get_billing()))));
        }
        if (parsed->get_tokenization_data()) {
            m_context->cache().put("payment_methods.session.tokenization:" + session_id,
                core::cache::to_cache_value(encrypt_data(serde::Ser::encode_json(*parsed->get_tokenization_data()))));
        }
        publish_pm_event("payment_method_session.create", session_id);
        m_context->metrics().record_flow("payment_methods_session_create");
        m_context->metrics().counter("PAYMENT_METHOD_PAYMENT_METHODS_SESSION_CREATE").increment();
        app::model::payment_methods::PaymentMethodSessionResponse response{};
        app::model::payment_methods::GlobalPaymentMethodSessionIdValue id{};
        id.set_value(session_id);
        response.set_id(std::move(id));
        response.set_client_secret(client_secret);
        app::model::payment_methods::PrimitiveDateTimeValue expires_at{};
        expires_at.set_value(std::to_string(expires));
        response.set_expires_at(std::move(expires_at));
        response.set_storage_type(parsed->get_storage_type());
        if (parsed->get_customer_id()) response.set_customer_id(*parsed->get_customer_id());
        if (parsed->get_billing()) response.set_billing(*parsed->get_billing());
        if (parsed->get_return_url()) response.set_return_url(*parsed->get_return_url());
        response.set_keep_alive(parsed->get_keep_alive().value_or(false));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_payment_methods_session_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("payment_methods_session_update");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const std::string session_id = path_segment(req);
        if (session_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing session id")); return; }
        auto repository = m_context->repository();
        auto record = repository.find_payment_method_session_by_string(session_id);
        if (!record) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "payment methods session does not exist or has expired")); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = app::model::payment_methods::PaymentMethodsSessionUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        if (parsed->get_billing()) {
            m_context->cache().put("payment_methods.session.billing:" + session_id,
                core::cache::to_cache_value(encrypt_data(serde::Ser::encode_json(*parsed->get_billing()))));
        }
        if (parsed->get_tokenization_data()) {
            m_context->cache().put("payment_methods.session.tokenization:" + session_id,
                core::cache::to_cache_value(encrypt_data(serde::Ser::encode_json(*parsed->get_tokenization_data()))));
        }
        const std::string secret = issued_token(session_id);
        record->set_client_secret(encrypt_data(secret));
        if (!repository.update_payment_method_session(*record)) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INTERNAL_ERROR, "persistence failed")); return; }
        remember_client_secret(session_id, secret);
        m_context->metrics().record_flow("payment_methods_session_update");
        m_context->metrics().counter("PAYMENT_METHOD_PAYMENT_METHODS_SESSION_UPDATE").increment();
        app::model::payment_methods::PaymentMethodSessionResponse response{};
        app::model::payment_methods::GlobalPaymentMethodSessionIdValue id{};
        id.set_value(session_id);
        response.set_id(std::move(id));
        response.set_client_secret(secret);
        app::model::payment_methods::PrimitiveDateTimeValue expires_at{};
        expires_at.set_value(record->get_expires_at());
        response.set_expires_at(std::move(expires_at));
        app::model::payment_methods::StorageTypeValue storage{};
        storage.set_value("payment_methods");
        response.set_storage_type(std::move(storage));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_payment_methods_session_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("payment_methods_session_retrieve");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const std::string session_id = path_segment(req);
        if (session_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing session id")); return; }
        const auto record = m_context->repository().find_payment_method_session_by_string(session_id);
        if (!record) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "payment methods session does not exist or has expired")); return; }
        const auto presented = app::http::RequestQuery{}(req).get("client_secret");
        if (presented) { if (const auto e = check_client_secret(session_id, *presented)) { send_error(res, send, *e); return; } }
        m_context->metrics().record_flow("payment_methods_session_retrieve");
        m_context->metrics().counter("PAYMENT_METHOD_PAYMENT_METHODS_SESSION_RETRIEVE").increment();
        app::model::payment_methods::PaymentMethodSessionResponse response{};
        app::model::payment_methods::GlobalPaymentMethodSessionIdValue id{};
        id.set_value(session_id);
        response.set_id(std::move(id));
        response.set_client_secret(decrypt_data(record->get_client_secret()));
        app::model::payment_methods::PrimitiveDateTimeValue expires_at{};
        expires_at.set_value(record->get_expires_at());
        response.set_expires_at(std::move(expires_at));
        app::model::payment_methods::StorageTypeValue storage{};
        const auto stored_storage = m_context->cache().get("payment_methods.session.storage:" + session_id);
        storage.set_value(stored_storage.hit ? core::cache::value_as<std::string>(stored_storage.value) : std::string{"payment_methods"});
        response.set_storage_type(std::move(storage));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_payment_method_session_list_payment_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("payment_method_session_list_payment_methods");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const std::string session_id = path_segment(req, 1);
        if (session_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing session id")); return; }
        const auto session = m_context->repository().find_payment_method_session_by_string(session_id);
        if (!session) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "payment methods session does not exist or has expired")); return; }
        const std::string customer_id = app::http::RequestQuery{}(req).get("customer_id").value_or("");
        m_context->metrics().record_flow("payment_method_session_list_payment_methods");
        m_context->metrics().counter("PAYMENT_METHOD_PAYMENT_METHOD_SESSION_LIST_PAYMENT_METHODS").increment();
        app::model::payment_methods::PaymentMethodListResponseForSession response{};
        std::vector<app::model::payment_methods::CustomerPaymentMethodResponseItem> items{};
        for (const auto& record : m_context->repository().list_payment_methods()) {
            if (record.get_merchant_id() != session->get_merchant_id()) continue;
            if (!customer_id.empty() && record.get_customer_id() != customer_id) continue;
            app::model::payment_methods::CustomerPaymentMethodResponseItem item{};
            item.set_payment_method_token(record.get_vault_id().empty() ? record.get_payment_method_id() : record.get_vault_id());
            app::model::payment_methods::GlobalCustomerIdValue customer{};
            customer.set_value(record.get_customer_id());
            item.set_customer_id(std::move(customer));
            app::model::payment_methods::PaymentMethodValue method{};
            method.set_value(record.get_type().empty() ? std::string{"card"} : record.get_type());
            item.set_payment_method_type(std::move(method));
            app::model::payment_methods::PrimitiveDateTimeValue created{};
            created.set_value(record.get_created_at());
            item.set_created(created);
            app::model::payment_methods::PrimitiveDateTimeValue last_used{};
            last_used.set_value(record.get_created_at());
            item.set_last_used_at(std::move(last_used));
            items.push_back(std::move(item));
        }
        response.set_customer_payment_methods(std::move(items));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_payment_method_session_confirm(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("payment_method_session_confirm");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const std::string session_id = path_segment(req, 1);
        if (session_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing session id")); return; }
        auto repository = m_context->repository();
        auto session = repository.find_payment_method_session_by_string(session_id);
        // Upstream core/payment_methods.rs:8227-8236 — GenericNotFoundError with this exact message.
        if (!session) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "payment methods session does not exist or has expired")); return; }
        if (session->get_status() != "active") { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::PRECONDITION_FAILED, "session is not active")); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = app::model::payment_methods::PaymentMethodSessionConfirmRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        const auto presented = app::http::RequestQuery{}(req).get("client_secret");
        if (presented) { if (const auto e = check_client_secret(session_id, *presented)) { send_error(res, send, *e); return; } }
        // Upstream core/payment_methods.rs:8273-8286 — payment_method_data is required.
        const auto payload = serde::Ser::encode_json(parsed->get_payment_method_data());
        if (payload.empty() || payload == "null") {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "Missing required field: payment_method_data")); return;
        }
        // Upstream core/payment_methods.rs:8256-8271 — storage_type follows the session's
        // intent only when customer_acceptance is present; PayThenVault forces volatile.
        std::string storage = "volatile";
        const auto stored = m_context->cache().get("payment_methods.session.storage:" + session_id);
        if (stored.hit) storage = core::cache::value_as<std::string>(stored.value);
        if (parsed->get_customer_acceptance()) {
            const auto& acceptance = parsed->get_customer_acceptance()->get_value();
            storage = acceptance.find("off_session") != std::string::npos ? "vault" : "payment_method";
        }
        const auto integration_type = m_context->config_store().get_value("payment_methods", "integration_type");
        if (integration_type && *integration_type == "pay_then_vault") { storage = "volatile"; }
        // Upstream cards.rs / core/payment_methods.rs:8240-8254 — unify session and request billing.
        const auto session_billing = m_context->cache().get("payment_methods.session.billing:" + session_id);
        if (session_billing.hit && body.find("billing") == std::string::npos) {
            const auto billing = decrypt_data(core::cache::value_as<std::string>(session_billing.value));
            m_context->cache().put("payment_methods.confirm.billing:" + session_id, core::cache::to_cache_value(encrypt_data(billing)));
        }

        std::string vault{};
        if (storage == "vault") {
            const auto vault_response = call_connector(session->get_merchant_id(), "vault", session_id, payload);
            vault = vault_response.get_reference_id();
        }
        const auto zero_auth = call_connector(session->get_merchant_id(), "zero_auth", session_id, payload);
        if (zero_auth.get_status().empty() && vault.empty()) {
            const auto token_response = call_connector(session->get_merchant_id(), "tokenize", session_id, payload);
            vault = token_response.get_reference_id();
        } else if (vault.empty()) {
            vault = zero_auth.get_reference_id();
        }
        if (vault.empty()) vault = issued_token(session_id);
        const std::string payment_method_id = global_id("pm_", session_id);
        app::store::payment_methods::PaymentMethodRecord record{};
        record.set_payment_method_id(payment_method_id);
        record.set_merchant_id(session->get_merchant_id());
        record.set_type(parsed->get_payment_method_type().get_value());
        if (parsed->get_payment_method_subtype()) {
            record.set_type(parsed->get_payment_method_subtype()->get_value());
        }
        std::string last4{};
        const auto card_doc = serde::Ser::decode_generic("application/json", payload);
        if (card_doc) {
            if (const auto* object = std::get_if<interfaces::Value::Object>(&card_doc->get())) {
                for (const auto& [name, field] : *object) {
                    if (name != "card_number" && name != "number" && name != "card") continue;
                    if (const auto* text = std::get_if<std::string>(&field.get())) {
                        if (text->size() >= 4) last4 = text->substr(text->size() - 4);
                    }
                }
            }
        }
        record.set_last4(last4);
        record.set_vault_id(vault);
        record.set_status("active");
        record.set_created_at(now_value());
        auto stored_pm = repository.create_payment_method(std::move(record));
        if (!stored_pm) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::ALREADY_EXISTS, "payment method exists")); return; }
        m_context->cache().put("payment_methods.card:" + payment_method_id, core::cache::to_cache_value(encrypt_data(payload)));
        const std::string single_use = issued_token(payment_method_id);
        // Upstream add_single_use_token_to_store (core/payment_methods.rs:8442-8462): persist the
        // temporary token with DEFAULT_PAYMENT_METHOD_STORE_TTL. ParentPaymentMethodToken::
        // get_data_for_token (core/payment_methods.rs:8596-8608) later resolves it to the PM id.
        m_context->cache().put("payment_methods.confirm:" + payment_method_id,
            core::cache::to_cache_value(single_use), core::cache::Ttl{}.seconds(60));
        m_context->cache().put("payment_methods.token:" + single_use,
            core::cache::to_cache_value(payment_method_id), core::cache::Ttl{}.seconds(60));
        // Upstream core/payment_methods.rs:8298-8315 — parent payment-method token with the
        // fulfillment-time TTL, inserted into the store and associated with the session.
        const std::string parent_token = "token_" + global_id("", session_id).substr(0, 16);
        m_context->cache().put("payment_methods.token:" + parent_token,
            core::cache::to_cache_value(payment_method_id), core::cache::Ttl{}.seconds(1800));
        m_context->cache().put("payment_methods.session.associated:" + session_id,
            core::cache::to_cache_value(parent_token), core::cache::Ttl{}.seconds(1800));
        session->set_status("confirmed");
        repository.update_payment_method_session(*session);
        publish_pm_event("payment_method_session.confirm", payment_method_id);
        m_context->metrics().record_flow("payment_method_session_confirm");
        m_context->metrics().counter("PAYMENT_METHOD_PAYMENT_METHOD_SESSION_CONFIRM").increment();
        app::model::payment_methods::PaymentMethodSessionResponse response{};
        app::model::payment_methods::GlobalPaymentMethodSessionIdValue id{};
        id.set_value(session_id);
        response.set_id(std::move(id));
        app::model::payment_methods::PrimitiveDateTimeValue expires_at{};
        expires_at.set_value(session->get_expires_at());
        response.set_expires_at(std::move(expires_at));
        app::model::payment_methods::StorageTypeValue storage_value{};
        storage_value.set_value(storage);
        response.set_storage_type(std::move(storage_value));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_payment_method_session_update_saved_payment_method(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("payment_method_session_update_saved_payment_method");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const std::string session_id = path_segment(req, 1);
        if (session_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing session id")); return; }
        const auto session = m_context->repository().find_payment_method_session_by_string(session_id);
        if (!session) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "payment methods session does not exist or has expired")); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = app::model::payment_methods::PaymentMethodSessionUpdateSavedPaymentMethod;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        const std::string token = parsed->get_payment_method_token().value_or("");
        if (token.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing payment method token")); return; }
        auto repository = m_context->repository();
        auto record = repository.find_payment_method_by_string(token);
        if (!record) {
            for (auto& candidate : repository.list_payment_methods()) {
                if (candidate.get_vault_id() == token) { record = candidate; break; }
            }
        }
        if (!record) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Payment method does not exist in our records")); return; }
        const auto vault = call_connector(session->get_merchant_id(), "tokenize", record->get_payment_method_id(), token);
        if (!vault.get_reference_id().empty()) record->set_vault_id(vault.get_reference_id());
        repository.update_payment_method(*record);
        publish_pm_event("payment_method_session.update_saved", record->get_payment_method_id());
        m_context->metrics().record_flow("payment_method_session_update_saved_payment_method");
        m_context->metrics().counter("PAYMENT_METHOD_PAYMENT_METHOD_SESSION_UPDATE_SAVED_PAYMENT_METHOD").increment();
        send_json(res, send, serde::Ser::encode_json(response_from_record(*record)), true);
    }


    void handle_payment_method_session_delete_saved_payment_method(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("payment_method_session_delete_saved_payment_method");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const std::string session_id = path_segment(req);
        if (session_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing session id")); return; }
        const auto session = m_context->repository().find_payment_method_session_by_string(session_id);
        if (!session) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "payment methods session does not exist or has expired")); return; }
        const auto body = req.body_as_string();
        using Input = app::model::payment_methods::PaymentMethodSessionDeleteSavedPaymentMethod;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        const std::string token = parsed->get_payment_method_token();
        if (token.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing payment method token")); return; }
        auto repository = m_context->repository();
        const auto record = repository.find_payment_method_by_string(token);
        if (!record) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Payment method does not exist in our records")); return; }
        (void)call_connector(session->get_merchant_id(), "delete_token", record->get_payment_method_id(), record->get_vault_id());
        repository.remove_payment_method(record->get_id());
        m_context->cache().erase("payment_methods.card:" + record->get_payment_method_id());
        publish_pm_event("payment_method_session.delete_saved", record->get_payment_method_id());
        m_context->metrics().record_flow("payment_method_session_delete_saved_payment_method");
        m_context->metrics().counter("PAYMENT_METHOD_PAYMENT_METHOD_SESSION_DELETE_SAVED_PAYMENT_METHOD").increment();
        app::model::payment_methods::PaymentMethodDeleteSessionResponse response{};
        response.set_payment_method_token(token);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_network_token_status_check_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("network_token_status_check_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const std::string payment_method_id = path_segment(req, 1);
        if (payment_method_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing payment method id")); return; }
        const auto record = m_context->repository().find_payment_method_by_string(payment_method_id);
        if (!record) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Payment method does not exist in our records")); return; }
        const auto status_response = call_connector(record->get_merchant_id(), "network_token_status", payment_method_id, record->get_vault_id());
        const std::string status = status_response.get_status().empty() ? record->get_status() : status_response.get_status();
        m_context->metrics().record_flow("network_token_status_check_api");
        m_context->metrics().counter("PAYMENT_METHOD_NETWORK_TOKEN_STATUS_CHECK_API").increment();
        app::model::payment_methods::NetworkTokenStatusCheckSuccessResponse response{};
        app::model::payment_methods::TokenStatusValue value{};
        value.set_value(status.empty() ? std::string{"active"} : status);
        response.set_status(std::move(value));
        app::model::payment_methods::GlobalPaymentMethodIdValue id{};
        id.set_value(payment_method_id);
        response.set_payment_method_id(std::move(id));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_payment_method_get_token_details_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("payment_method_get_token_details_api");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const std::string temporary_token = path_segment(req, 1);
        if (temporary_token.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing token")); return; }
        const auto hit = m_context->cache().get("payment_methods.token:" + temporary_token);
        if (!hit.hit) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "token not found")); return; }
        const std::string payment_method_id = core::cache::value_as<std::string>(hit.value);
        // Upstream core/payment_methods.rs:8600-8623 — resolve the token to the PM id and verify
        // the PM exists in the configured storage before returning its details.
        if (!m_context->repository().find_payment_method_by_string(payment_method_id)) {
            send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "Payment method does not exist in our records")); return;
        }
        m_context->metrics().record_flow("payment_method_get_token_details_api");
        m_context->metrics().counter("PAYMENT_METHOD_PAYMENT_METHOD_GET_TOKEN_DETAILS_API").increment();
        app::model::payment_methods::PaymentMethodGetTokenDetailsResponse response{};
        app::model::payment_methods::GlobalPaymentMethodIdValue id{};
        id.set_value(payment_method_id);
        response.set_id(std::move(id));
        response.set_payment_method_token(temporary_token);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }


    void handle_list_payment_methods_for_payments_client(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("list_payment_methods_for_payments_client");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        if (const auto sdk_error = authorize_client_facing(req, "list_payment_methods_for_payments_client")) { send_error(res, send, *sdk_error); return; }
        const std::string payment_id = path_segment(req, 1);
        if (payment_id.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "missing payment id")); return; }
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        bool found = false;
        for (const auto& record : m_context->repository().list_payments()) {
            if (record.get_payment_id() != payment_id) continue;
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) {
                send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "payment belongs to another merchant")); return;
            }
            found = true;
            break;
        }
        if (!found) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "payment not found")); return; }
        const auto query = app::http::RequestQuery{}(req);
        const std::string client_secret = query.get("client_secret").value_or("");
        if (const auto secret_error = check_client_secret(payment_id, client_secret.empty() ? std::nullopt : std::optional<std::string>{client_secret})) { send_error(res, send, *secret_error); return; }
        m_context->metrics().record_flow("list_payment_methods_for_payments_client");
        m_context->metrics().counter("PAYMENT_METHOD_LIST_PAYMENT_METHODS_FOR_PAYMENTS_CLIENT").increment();
        app::model::payment_methods::PaymentMethodListResponseForSession response{};
        std::vector<app::model::payment_methods::CustomerPaymentMethodResponseItem> items{};
        for (const auto& record : m_context->repository().list_payment_methods()) {
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) continue;
            app::model::payment_methods::CustomerPaymentMethodResponseItem item{};
            item.set_payment_method_token(record.get_vault_id().empty() ? record.get_payment_method_id() : record.get_vault_id());
            app::model::payment_methods::GlobalCustomerIdValue customer{};
            customer.set_value(record.get_customer_id());
            item.set_customer_id(std::move(customer));
            app::model::payment_methods::PaymentMethodValue method{};
            method.set_value(record.get_type().empty() ? std::string{"card"} : record.get_type());
            item.set_payment_method_type(std::move(method));
            app::model::payment_methods::PrimitiveDateTimeValue created{};
            created.set_value(record.get_created_at());
            item.set_created(created);
            app::model::payment_methods::PrimitiveDateTimeValue last_used{};
            last_used.set_value(record.get_created_at());
            item.set_last_used_at(std::move(last_used));
            items.push_back(std::move(item));
        }
        response.set_customer_payment_methods(std::move(items));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }



    void handle_modular_migrate_payment_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("modular_migrate_payment_methods");
        const auto auth_error = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth_error) { send_error(res, send, *auth_error); return; }
        const auto body = req.body_as_string();
        if (body.empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::EMPTY_REQUEST, "empty request")); return; }
        using Input = std::vector<app::model::payment_methods::PaymentMethodMigrate>;
        auto parsed = serde::Ser::deserialize<Input>("application/json", parse_batch_rows(body));
        if (!parsed) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_JSON, "invalid JSON")); return; }
        if (parsed->empty()) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::INVALID_REQUEST, "no rows")); return; }
        if (parsed->size() > 200) { send_error(res, send, core::error::AppErrorBuilder{}(core::error::AppErrorKind::VALIDATION_FAILED, "batch limit is 200")); return; }
        m_context->metrics().record_flow("modular_migrate_payment_methods");
        m_context->metrics().counter("PAYMENT_METHOD_MODULAR_MIGRATE_PAYMENT_METHODS").increment();
        app::model::payment_methods::ModularPaymentMethodMigrationResponse response{};
        response.set_total_rows(parsed->size());
        std::vector<app::model::payment_methods::ModularPaymentMethodMigrationRowResult> results{};
        std::size_t successful{0};
        auto repository = m_context->repository();
        for (std::size_t i = 0; i < parsed->size(); ++i) {
            const auto& source = (*parsed)[i];
            app::model::payment_methods::ModularPaymentMethodMigrationRowResult row{};
            row.set_row_number(i + 1);
            const auto merchant_id = source.get_merchant_id().get_value();
            app::model::payment_methods::MerchantIdValue merchant{};
            merchant.set_value(merchant_id);
            row.set_merchant_id(std::move(merchant));
            std::string last4;
            std::string network;
            if (source.get_card()) {
                const auto& number = source.get_card()->get_card_number();
                if (number.size() >= 4) last4 = number.substr(number.size() - 4);
                if (source.get_card()->get_card_network()) network = source.get_card()->get_card_network()->get_value();
            }
            const auto fingerprint = merchant_fingerprint(merchant_id, last4, network);
            row.set_old_fingerprint_id(fingerprint.substr(0, 16));
            const auto duplicate = m_context->cache().get("payment_methods.fingerprint:" + fingerprint);
            if (duplicate.hit) {
                const auto existing = repository.find_payment_method_by_string(core::cache::value_as<std::string>(duplicate.value));
                if (existing) {
                    row.set_payment_method_id(existing->get_payment_method_id());
                    row.set_new_fingerprint_id(existing->get_vault_id());
                    row.set_migration_status(app::model::payment_methods::MigrationStatus::Success);
                    ++successful;
                    results.push_back(std::move(row));
                    continue;
                }
            }
            app::store::payment_methods::PaymentMethodRecord record{};
            record.set_payment_method_id(global_id("pm_", merchant_id + std::to_string(i)));
            record.set_merchant_id(merchant_id);
            if (source.get_customer_id()) record.set_customer_id(source.get_customer_id()->get_value());
            record.set_type(source.get_payment_method() ? source.get_payment_method()->get_value() : std::string{"card"});
            record.set_network(network);
            record.set_last4(last4);
            record.set_vault_id(fingerprint.substr(0, 16));
            record.set_status("active");
            record.set_created_at(now_value());
            const auto stored = repository.create_payment_method(std::move(record));
            if (!stored) {
                row.set_migration_status(app::model::payment_methods::MigrationStatus::Failed);
                row.set_error_message("persistence failed");
            } else {
                row.set_payment_method_id(stored->get_payment_method_id());
                row.set_new_fingerprint_id(fingerprint.substr(0, 16));
                row.set_migration_status(app::model::payment_methods::MigrationStatus::Success);
                m_context->cache().put("payment_methods.fingerprint:" + fingerprint, core::cache::to_cache_value(stored->get_payment_method_id()));
                ++successful;
            }
            results.push_back(std::move(row));
        }
        response.set_successful_count(successful);
        response.set_failed_count(parsed->size() - successful);
        response.set_results(std::move(results));
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "payment_methods"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_payment_method_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_payment_method_api")
            )
            .add_route(
                Route{""}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_payment_method_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_payment_method_api")
            )
            .add_route(
                Route{"filter"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_countries_currencies_for_connector_payment_method(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_countries_currencies_for_connector_payment_method")
            )
            .add_route(
                Route{"migrate"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_migrate_payment_method_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch migrate_payment_method_api")
            )
            .add_route(
                Route{"migrate-batch"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_migrate_payment_methods(req, res, std::move(send));
                    })
                    .summary("Hyperswitch migrate_payment_methods")
            )
            .add_route(
                Route{"update-batch"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_update_payment_methods(req, res, std::move(send));
                    })
                    .summary("Hyperswitch update_payment_methods")
            )
            .add_route(
                Route{"batch"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_methods_batch_retrieve_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_methods_batch_retrieve_api")
            )
            .add_router(Router{router, "fingerprint"}.add_route(
                Route{"migrate-batch"}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_modular_migrate_payment_methods(req, res, std::move(send));
                    })
                    .summary("Hyperswitch modular_migrate_payment_methods")
            ))
            .add_route(
                Route{"tokenize-card"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_tokenize_card_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch tokenize_card_api")
            )
            .add_route(
                Route{"tokenize-card-batch"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_tokenize_card_batch_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch tokenize_card_batch_api")
            )
            .add_route(
                Route{"collect"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_initiate_pm_collect_link_flow(req, res, std::move(send));
                    })
                    .summary("Hyperswitch initiate_pm_collect_link_flow")
            )
            .add_router(Router{router, "collect"}.add_router(Router{router, ":merchant_id"}.add_route(
                Route{":collect_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_render_pm_collect_link(req, res, std::move(send));
                    })
                    .summary("Hyperswitch render_pm_collect_link")
            )))
            .add_route(
                Route{"create-intent"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_payment_method_intent_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_payment_method_intent_api")
            )
            .add_route(
                Route{"get-network-token-eligibility"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_pm_nt_eligibility_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_pm_nt_eligibility_api")
            )
            .add_router(Router{router, "token"}.add_router(Router{router, ":payment_method_temporary_token"}.add_route(
                Route{"details"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_method_get_token_details_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_get_token_details_api")
            )))
            .add_router(Router{router, "customers"}.add_route(
                Route{"total-payment-methods"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_total_payment_method_count(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_total_payment_method_count")
            ))
            .add_router(Router{router, "customers"}.add_route(
                Route{"payment_methods"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_customer_payment_method_api_client(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_customer_payment_method_api_client")
            ))
            .add_router(Router{router, "customers"}.add_router(Router{router, ":id"}.add_route(
                Route{"saved-payment-methods"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_customer_payment_method_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_customer_payment_method_api")
            )))
            .add_router(Router{router, "customers"}.add_router(Router{router, ":customer_id"}.add_route(
                Route{"payment_methods"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_customer_payment_method_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_customer_payment_method_api")
            )))
            .add_router(Router{router, "customers"}.add_router(Router{router, ":customer_id"}.add_router(Router{router, "payment-methods"}.add_router(Router{router, ":payment_method_id"}.add_route(
                Route{"default"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_default_payment_method_set_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch default_payment_method_set_api")
            )))))
            .add_route(
                Route{"payment-method-sessions"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_methods_session_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_methods_session_create")
            )
            .add_router(Router{router, "payment-method-sessions"}.add_route(
                Route{":payment_method_session_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_methods_session_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_methods_session_retrieve")
            ))
            .add_router(Router{router, "payment-method-sessions"}.add_route(
                Route{":payment_method_session_id"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_methods_session_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_methods_session_update")
            ))
            .add_router(Router{router, "payment-method-sessions"}.add_route(
                Route{":payment_method_session_id"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_method_session_delete_saved_payment_method(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_session_delete_saved_payment_method")
            ))
            .add_router(Router{router, "payment-method-sessions"}.add_router(Router{router, ":payment_method_session_id"}.add_route(
                Route{"list-payment-methods"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_method_session_list_payment_methods(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_session_list_payment_methods")
            )))
            .add_router(Router{router, "payment-method-sessions"}.add_router(Router{router, ":payment_method_session_id"}.add_route(
                Route{"confirm"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_method_session_confirm(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_session_confirm")
            )))
            .add_router(Router{router, "payment-method-sessions"}.add_router(Router{router, ":payment_method_session_id"}.add_route(
                Route{"update-saved-payment-method"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_method_session_update_saved_payment_method(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_session_update_saved_payment_method")
            )))
            .add_router(Router{router, "payments"}.add_router(Router{router, ":payment_id"}.add_route(
                Route{"client"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_payment_methods_for_payments_client(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_payment_methods_for_payments_client")
            )))
            .add_route(
                Route{":payment_method_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_method_retrieve_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_retrieve_api")
            )
            .add_route(
                Route{":payment_method_id"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_method_delete_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_delete_api")
            )
            .add_router(Router{router, ":payment_method_id"}.add_route(
                Route{"tokenize-card"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_tokenize_card_using_pm_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch tokenize_card_using_pm_api")
            ))
            .add_router(Router{router, ":payment_method_id"}.add_route(
                Route{"update"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_method_update_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_update_api")
            ))
            .add_router(Router{router, ":payment_method_id"}.add_route(
                Route{"save"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_save_payment_method_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch save_payment_method_api")
            ))
            .add_router(Router{router, ":payment_method_id"}.add_route(
                Route{"check-network-token-status"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_network_token_status_check_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch network_token_status_check_api")
            ))
            .add_router(Router{router, ":id"}.add_route(
                Route{"details"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_method_retrieve_olap_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_retrieve_olap_api")
            ))
            .add_router(Router{router, ":id"}.add_route(
                Route{"update-saved-payment-method"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_method_update_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_update_api")
            ))
            .add_router(Router{router, ":id"}.add_route(
                Route{"get-token"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_payment_method_token_data(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payment_method_token_data")
            ))
            .add_router(Router{router, ":id"}.add_route(
                Route{"list-enabled-payment-methods"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_method_session_list_payment_methods(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_session_list_payment_methods")
            ));
        Router{router, ":pm_prefix/payment-methods"}
            .add_route(
                Route{""}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_create_payment_method_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_payment_method_api (v2 prefix)")
            )
            .add_route(
                Route{""}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_list_payment_method_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_payment_method_api (v2 prefix)")
            )
            .add_route(
                Route{"filter"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_list_countries_currencies_for_connector_payment_method(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_countries_currencies_for_connector_payment_method (v2 prefix)")
            )
            .add_router(Router{router, "fingerprint"}.add_route(
                Route{"migrate-batch"}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_modular_migrate_payment_methods(req, res, std::move(send));
                    })
                    .summary("Hyperswitch modular_migrate_payment_methods (v2 prefix)")
            ))
            .add_route(
                Route{":id"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_payment_method_retrieve_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_retrieve_api (v2 prefix)")
            )
            .add_route(
                Route{":id"}
                    .delt([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_payment_method_delete_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_delete_api (v2 prefix)")
            )
            .add_router(Router{router, ":id"}.add_route(
                Route{"update"}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_payment_method_update_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_update_api (v2 prefix)")
            ))
            .add_router(Router{router, ":id"}.add_route(
                Route{"save"}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_save_payment_method_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch save_payment_method_api (v2 prefix)")
            ))
            .add_router(Router{router, ":id"}.add_route(
                Route{"details"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_payment_method_retrieve_olap_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_retrieve_olap_api (v2 prefix)")
            ))
            .add_router(Router{router, ":id"}.add_route(
                Route{"get-token"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_get_payment_method_token_data(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_payment_method_token_data (v2 prefix)")
            ))
            .add_router(Router{router, ":id"}.add_route(
                Route{"list-enabled-payment-methods"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_payment_method_session_list_payment_methods(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_session_list_payment_methods (v2 prefix)")
            ))
            .add_router(Router{router, ":id"}.add_route(
                Route{"check-network-token-status"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_network_token_status_check_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch network_token_status_check_api (v2 prefix)")
            ));
        Router{router, ":pm_prefix/payment-method-sessions"}
            .add_route(
                Route{""}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_payment_methods_session_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_methods_session_create (v2 prefix)")
            )
            .add_route(
                Route{":id"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_payment_methods_session_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_methods_session_retrieve (v2 prefix)")
            )
            .add_route(
                Route{":id"}
                    .put([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_payment_methods_session_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_methods_session_update (v2 prefix)")
            )
            .add_route(
                Route{":id"}
                    .delt([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_payment_method_session_delete_saved_payment_method(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_session_delete_saved_payment_method (v2 prefix)")
            )
            .add_router(Router{router, ":id"}.add_route(
                Route{"list-payment-methods"}
                    .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_payment_method_session_list_payment_methods(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_session_list_payment_methods (v2 prefix)")
            ))
            .add_router(Router{router, ":id"}.add_route(
                Route{"confirm"}
                    .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_payment_method_session_confirm(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_session_confirm (v2 prefix)")
            ))
            .add_router(Router{router, ":id"}.add_route(
                Route{"update-saved-payment-method"}
                    .put([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                        handle_payment_method_session_update_saved_payment_method(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_method_session_update_saved_payment_method (v2 prefix)")
            ));
    }

private:
    [[nodiscard]] std::optional<core::error::AppError> check_merchant_connector(
        std::string_view merchant_id
    ) const
    {
        auto repository = m_context->repository();
        const auto connectors = repository.list_merchant_connectors();
        if (connectors.empty()) {
            return std::nullopt;
        }
        for (const auto& mca : connectors) {
            if (std::string_view{mca.get_merchant_id()} == merchant_id) {
                return std::nullopt;
            }
        }
        return core::error::AppErrorBuilder{}(core::error::AppErrorKind::NOT_FOUND, "merchant connector account not found");
    }

    [[nodiscard]] std::vector<std::string> csv_cells(std::string_view line) const
    {
        std::vector<std::string> cells{};
        std::size_t start{0};
        while (start <= line.size()) {
            const auto end = line.find(',', start);
            std::string cell{line.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start)};
            while (!cell.empty() && (cell.back() == '\r' || cell.back() == '\n' || cell.back() == ' ')) {
                cell.pop_back();
            }
            cells.push_back(std::move(cell));
            if (end == std::string_view::npos) break;
            start = end + 1;
        }
        return cells;
    }

    [[nodiscard]] std::string parse_batch_rows(std::string_view body) const
    {
        const auto first = body.find_first_not_of(" \t\r\n");
        if (first == std::string_view::npos) {
            return "[]";
        }
        if (body[first] == '[' || body[first] == '{') {
            return std::string{body};
        }
        std::vector<std::string> lines{};
        std::size_t pos{0};
        while (pos < body.size()) {
            const auto end = body.find('\n', pos);
            std::string line{body.substr(pos, end == std::string_view::npos ? std::string_view::npos : end - pos)};
            while (!line.empty() && line.back() == '\r') line.pop_back();
            if (!line.empty() && line.front() != '-' && !line.starts_with("Content-")) {
                lines.push_back(std::move(line));
            }
            if (end == std::string_view::npos) break;
            pos = end + 1;
        }
        if (lines.size() < 2) {
            return "[]";
        }
        const auto header = csv_cells(lines.front());
        std::string out = "[";
        for (std::size_t row = 1; row < lines.size(); ++row) {
            const auto cells = csv_cells(lines[row]);
            if (row > 1) out += ",";
            out += "{";
            bool wrote = false;
            std::string card{};
            for (std::size_t col = 0; col < header.size() && col < cells.size(); ++col) {
                const auto& name = header[col];
                if (name.empty() || cells[col].empty()) continue;
                const auto value = serde::Ser::encode_json(cells[col]);
                if (name == "card_number" || name == "card_exp_month" || name == "card_exp_year") {
                    if (!card.empty()) card += ",";
                    card += serde::Ser::encode_json(name) + ":" + value;
                    continue;
                }
                if (wrote) out += ",";
                wrote = true;
                if (name == "merchant_id" || name == "customer_id" || name == "payment_method" ||
                    name == "payment_method_type" || name == "card_network" || name == "network_transaction_id") {
                    out += serde::Ser::encode_json(name) + ":{\"value\":" + value + "}";
                } else {
                    out += serde::Ser::encode_json(name) + ":" + value;
                }
            }
            if (!card.empty()) {
                if (wrote) out += ",";
                out += "\"card\":{" + card + "}";
            }
            out += "}";
        }
        out += "]";
        return out;
    }

    /** @brief Upstream batch_retrieve::parse_csv — one (payment_method_id, line_number) per CSV row. */
    [[nodiscard]] std::vector<std::pair<std::string, std::int64_t>> batch_retrieve_rows(
        std::string_view body
    ) const
    {
        std::vector<std::pair<std::string, std::int64_t>> out{};
        std::size_t pos{0};
        bool header{true};
        std::size_t id_column{0};
        std::int64_t line_number{0};
        while (pos < body.size()) {
            const auto end = body.find('\n', pos);
            std::string text{body.substr(pos, end == std::string_view::npos ? std::string_view::npos : end - pos)};
            while (!text.empty() && text.back() == '\r') text.pop_back();
            if (!text.empty() && text.front() != '-' && !text.starts_with("Content-")) {
                const auto cells = csv_cells(text);
                if (header) {
                    for (std::size_t i = 0; i < cells.size(); ++i) {
                        if (cells[i] == "payment_method_id") { id_column = i; break; }
                    }
                    header = false;
                } else if (id_column < cells.size() && !cells[id_column].empty()) {
                    out.emplace_back(cells[id_column], ++line_number);
                }
            }
            if (end == std::string_view::npos) break;
            pos = end + 1;
        }
        return out;
    }

    [[nodiscard]] bool legacy_access_allowed(interfaces::io::IRequest& req) const
    {
        (void)req;
        const auto configured = m_context->config_store().get_value("payment_methods", "legacy_endpoints_enabled");
        return !configured || (*configured != "false" && *configured != "disabled");
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize_client_facing(
        interfaces::io::IRequest& req,
        std::string_view op
    ) const
    {
        const auto context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for(op);
        if (!m_context->authorizer().authorize_route(context, rule)) {
            return std::nullopt;
        }
        for (const auto scheme : {
                 app::auth::AuthScheme::PUBLISHABLE_KEY,
                 app::auth::AuthScheme::CLIENT_SECRET,
                 app::auth::AuthScheme::SDK,
             }) {
            const auto sdk_rule = app::auth::RouteAuthRuleBuilder{}
                                      .scheme(scheme)
                                      .permission(rule.permission())
                                      .build();
            if (!m_context->authorizer().authorize_route(context, sdk_rule)) {
                return std::nullopt;
            }
        }
        return m_context->authorizer().authorize_route(context, rule);
    }

    [[nodiscard]] std::string client_secret_hash(std::string_view secret) const
    {
        return app::crypto::KeyedHasher{"payment_methods.client_secret"}.hash(secret);
    }

    [[nodiscard]] std::optional<core::error::AppError> check_client_secret(
        std::string_view key,
        const std::optional<std::string>& presented
    ) const
    {
        if (!presented) {
            return std::nullopt;
        }
        const auto hit = m_context->cache().get(std::string{"payment_methods.client_secret_hash:"} + std::string{key});
        if (!hit.hit) {
            return std::nullopt;
        }
        const auto expected = core::cache::value_as<std::string>(hit.value);
        if (!app::crypto::KeyedHasher{"payment_methods.client_secret"}.verify(*presented, expected)) {
            return core::error::AppErrorBuilder{}(core::error::AppErrorKind::FORBIDDEN, "invalid client secret");
        }
        return std::nullopt;
    }

    void remember_client_secret(std::string_view key, std::string_view secret) const
    {
        m_context->cache().put(
            std::string{"payment_methods.client_secret_hash:"} + std::string{key},
            core::cache::to_cache_value(client_secret_hash(secret)));
        m_context->cache().put(
            "payment_methods.client_secret:" + std::string{key},
            core::cache::to_cache_value(std::string{secret}));
    }

    [[nodiscard]] std::string encryption_material() const
    {
        const auto key = m_context->key_store().get("payment_methods", "data");
        return key ? std::string{key->value()} : std::string{"payment-methods-local-data-key"};
    }

    [[nodiscard]] std::string encrypt_data(std::string_view plaintext) const
    {
        return app::crypto::AesWrap{}.encrypt(plaintext, encryption_material());
    }

    [[nodiscard]] std::string decrypt_data(std::string_view ciphertext) const
    {
        return app::crypto::AesWrap{}.decrypt(ciphertext, encryption_material());
    }

    [[nodiscard]] std::string merchant_fingerprint(
        std::string_view merchant_id,
        std::string_view last4,
        std::string_view network
    ) const
    {
        return app::crypto::KeyedHasher{"payment_methods.fingerprint"}.hash(
            std::string{merchant_id} + "|" + std::string{last4} + "|" + std::string{network});
    }

    [[nodiscard]] std::string global_id(std::string_view prefix, std::string_view subject) const
    {
        const auto digest = app::crypto::KeyedHasher{"payment_methods.cell"}.hash(
            std::string{subject} + "_" + m_context->ids().uuid_v4());
        return std::string{prefix} + digest.substr(0, 16);
    }

    [[nodiscard]] app::connector::ConnectorResponse call_connector(
        std::string_view merchant_id,
        std::string_view operation,
        std::string_view resource_id,
        std::string_view payload
    ) const
    {
        auto request = app::connector::ConnectorRequestBuilder{}
                           .operation(std::string{operation})
                           .merchant_id(std::string{merchant_id})
                           .resource_id(std::string{resource_id})
                           .raw_payload(std::string{payload})();
        const auto connector = m_context->connector_router().select_for(request);
        if (!connector) {
            // Local in-process substitute for the external vault / PSP / network-token service.
            return local_connector_response(operation, merchant_id, resource_id);
        }
        app::connector::ConnectorResponse response{};
        if (operation == "tokenize" || operation == "vault") {
            response = connector->tokenize(request);
        } else if (operation == "delete_token") {
            response = connector->delete_token(request);
        } else if (operation == "sync" || operation == "network_token_status") {
            response = connector->sync(request);
        } else if (operation == "check_feature_matrix" || operation == "eligibility") {
            response = connector->check_feature_matrix(request);
        } else if (operation == "create_link") {
            response = connector->create_link(request);
        } else if (operation == "exchange_token") {
            response = connector->exchange_token(request);
        } else if (operation == "verify" || operation == "zero_auth") {
            response = connector->verify(request);
        } else {
            response = connector->authorize(request);
        }
        // Upstream-shaped local enrichment: a connector that returns no reference for a
        // vaulting operation still yields a deterministic, idempotent token.
        if (response.get_reference_id().empty() &&
            (operation == "tokenize" || operation == "vault" || operation == "zero_auth")) {
            response.set_reference_id(local_vault_token(merchant_id, resource_id));
        }
        if (response.get_status().empty()) {
            response.set_status("success");
        }
        return response;
    }

    /** @brief Deterministic local vault token (in-process locker substitute). */
    [[nodiscard]] std::string local_vault_token(
        std::string_view merchant_id,
        std::string_view resource_id
    ) const
    {
        const std::string digest = app::crypto::KeyedHasher{"payment_methods.vault"}.hash(
            std::string{merchant_id} + "|" + std::string{resource_id});
        return "pm_" + digest.substr(0, 24);
    }

    /** @brief Upstream-shaped response when no external connector is configured. */
    [[nodiscard]] app::connector::ConnectorResponse local_connector_response(
        std::string_view operation,
        std::string_view merchant_id,
        std::string_view resource_id
    ) const
    {
        app::connector::ConnectorResponse response{};
        response.set_status("success");
        response.set_attempt(1);
        if (operation == "tokenize" || operation == "vault" || operation == "zero_auth") {
            const std::string token = local_vault_token(merchant_id, resource_id);
            response.set_reference_id(token);
            response.set_raw_json("{\"token\":" + serde::Ser::encode_json(token) + ",\"tokenize_support\":true}");
        } else if (operation == "eligibility" || operation == "check_feature_matrix") {
            response.set_raw_json("{\"tokenize_support\":true}");
        } else if (operation == "create_link") {
            response.set_raw_json("/payment_methods/collect/" + std::string{merchant_id} + "/" + std::string{resource_id});
        } else if (operation == "exchange_token") {
            response.set_reference_id(issued_token(resource_id));
            response.set_raw_json("{\"access_token\":" + serde::Ser::encode_json(issued_token(resource_id)) + "}");
        }
        return response;
    }

    [[nodiscard]] app::model::payment_methods::PaymentMethodResponse response_from_record(
        const app::store::payment_methods::PaymentMethodRecord& record
    ) const
    {
        using app::model::payment_methods::CardDetailFromLocker;
        using app::model::payment_methods::CustomerIdValue;
        using app::model::payment_methods::MerchantIdValue;
        using app::model::payment_methods::PaymentMethodResponse;
        using app::model::payment_methods::PaymentMethodValue;
        using app::model::payment_methods::PrimitiveDateTimeValue;
        PaymentMethodResponse response{};
        MerchantIdValue merchant{};
        merchant.set_value(record.get_merchant_id());
        response.set_merchant_id(std::move(merchant));
        if (!record.get_customer_id().empty()) {
            CustomerIdValue customer{};
            customer.set_value(record.get_customer_id());
            response.set_customer_id(std::move(customer));
        }
        response.set_payment_method_id(record.get_payment_method_id());
        if (!record.get_type().empty()) {
            PaymentMethodValue method{};
            method.set_value(record.get_type());
            response.set_payment_method(std::move(method));
        }
        CardDetailFromLocker card{};
        if (!record.get_network().empty()) {
            card.set_scheme(record.get_network());
        }
        if (!record.get_last4().empty()) {
            card.set_last4_digits(record.get_last4());
        }
        if (!record.get_vault_id().empty()) {
            card.set_card_token(record.get_vault_id());
        }
        response.set_card(std::move(card));
        PrimitiveDateTimeValue created{};
        created.set_value(record.get_created_at());
        response.set_created(std::move(created));
        const auto cached = m_context->cache().get("payment_methods.client_secret:" + record.get_payment_method_id());
        if (cached.hit) {
            response.set_client_secret(core::cache::value_as<std::string>(cached.value));
        }
        return response;
    }

    void schedule_status_task(
        std::string_view payment_method_id,
        std::string_view merchant_id
    ) const
    {
        const auto payload = std::string{"{\"payment_method_id\":"} +
            serde::Ser::encode_json(std::string{payment_method_id}) +
            ",\"merchant_id\":" + serde::Ser::encode_json(std::string{merchant_id}) + "}";
        m_context->process_trackers().schedule("payment_method_status_update", payload, now_value());
    }

    void publish_pm_event(std::string_view topic, std::string_view payment_method_id) const
    {
        const auto payload = std::string{"{\"payment_method_id\":"} +
            serde::Ser::encode_json(std::string{payment_method_id}) + "}";
        m_context->events().publish(std::string{topic}, payload);
    }

    [[nodiscard]] std::string path_segment(
        interfaces::io::IRequest& req,
        std::size_t from_end = 0
    ) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        if (from_end >= count) {
            return {};
        }
        const auto value = segments.at(count - 1 - from_end);
        return value ? std::string{*value} : std::string{};
    }

    [[nodiscard]] bool customer_exists(std::string_view customer_id) const
    {
        if (customer_id.empty()) {
            return false;
        }
        auto repository = m_context->repository();
        for (const auto& record : repository.list_customers()) {
            if (record.get_customer_id() == customer_id) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::string issued_token(std::string_view subject) const
    {
        const app::crypto::SecretString secret{m_context->ids().uuid_v4()};
        return app::crypto::JwtSigner{std::string{secret.value()}}.sign(subject);
    }

    [[nodiscard]] std::string now_value() const
    {
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        return std::to_string(seconds);
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

    /** @brief Local error kind/status substitute. core::error::AppErrorKind cannot be extended
     *  without a shared-file change, so upstream variants with no local kind (e.g.
     *  ApiErrorResponse::NotSupported) are emitted with their exact wire name and status here. */
    void send_local_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view wire_name,
        std::string_view message,
        interfaces::io::types::Status status
    ) const
    {
        core::error::AppError error{};
        error.set_kind(core::error::AppErrorKind::INVALID_REQUEST);
        error.set_kind_name(std::string{wire_name});
        error.set_message(std::string{message});
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(status);
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

} // namespace app::handlers::payment_methods
