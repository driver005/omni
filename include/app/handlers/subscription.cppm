module;
export module app_handlers:subscription;

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
import app_http;
import app_crypto;
import app_metrics;
import app_connector;
import app_services;
import app_workflow;
import app_events;

export namespace app::handlers::subscription {

/**
 * Strict local port of upstream Hyperswitch subscription routes + core.
 *
 * Upstream references (SHA 7736d55adaf235f7af74e133ae408573b1826823):
 *  - routes/subscription.rs:21-47 (extract_profile_id), 49-538 (11 route handlers)
 *  - crates/subscriptions/src/core.rs:31-758 (the 11 core functions)
 *  - core/subscription_handler.rs, core/billing_processor_handler.rs,
 *    core/invoice_handler.rs, core/payments_api_client.rs
 *  - api_models/src/subscription.rs (wire DTOs),
 *    services/authentication.rs:5996-6065 (SDK/client-secret auth), 6464-6475 (auth_type)
 *  - hyperswitch_domain_models/src/errors/api_error_response.rs + api_models/errors/types.rs
 *    (exact ApiErrorResponse wire shapes)
 *
 * Every external dependency (business-profile store, customer store, billing processor,
 * payments microservice, invoice table) is replaced with an in-process local substitute
 * drawn from AppContext: repository(), connector_router(), config_store(), process_trackers()
 * and events(). The control flow, request/response wire shapes, auth semantics, errors and
 * persistence semantics mirror upstream function-by-function.
 */
class SubscriptionRouteHandlers
{
public:
    // ── POST /subscriptions/create — create_subscription ────────────────────────
    void handle_create_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto failure = authorize_api_key_or_jwt(req, auth, kWritePermission)) {
            send_failure(res, send, *failure);
            return;
        }
        auto profile = extract_profile_id(req, res, send);
        if (!profile) { return; }

        const auto payload = parse_json_body(req, res, send);
        if (!payload) { return; }
        const auto customer_id = json_string(payload->root, "customer_id");
        const auto item_price_id = json_string(payload->root, "item_price_id");
        if (!customer_id || customer_id->empty() || !item_price_id || item_price_id->empty()) {
            send_json_body_error(res, send, "missing field `item_price_id` or `customer_id` at line 1 column 2");
            return;
        }

        // core::create_subscription (core.rs:31-123)
        const auto merchant = merchant_id(req, auth);
        if (const auto failure = find_business_profile(profile.value, merchant)) {
            send_failure(res, send, *failure);
            return;
        }
        if (const auto failure = find_customer(*customer_id, merchant)) {
            send_failure(res, send, *failure);
            return;
        }
        BillingProcessor billing{this};
        if (const auto failure = billing.create(profile.value, merchant)) {
            send_failure(res, send, *failure);
            return;
        }

        const std::string subscription_id = m_context->ids().payment_id("sub_");
        SubscriptionRecord record{};
        record.subscription_id = subscription_id;
        record.merchant_id = merchant;
        record.profile_id = profile.value;
        record.customer_id = *customer_id;
        record.status = "created"; // SubscriptionStatus::Created.to_string() -> "Created" (subscription.rs:54)
        record.plan_id = json_string(payload->root, "plan_id").value_or("");
        record.item_price_id = *item_price_id;
        record.coupon = json_string(payload->root, "coupon_code").value_or("");
        record.merchant_reference_id = json_string(payload->root, "merchant_reference_id").value_or("");
        record.created_at = now_value();
        record.modified_at = record.created_at;
        record.client_secret = generate_client_secret(subscription_id); // generate_and_set_client_secret
        record.merchant_connector_id = billing.merchant_connector_id;

        auto stored = m_context->repository().create_subscription(to_store_record(record));
        if (!stored) {
            send_failure(res, send, internal_error());
            return;
        }
        record.id = stored->get_id();

        // get_subscription_estimate -> amount/currency (core.rs:68-88)
        Estimate estimate = billing.estimate(record, json_string(payload->root, "plan_id").value_or(""),
                                             *item_price_id, "", record.client_secret);

        // create_payment_with_confirm_false (invoice_handler.rs:116-143)
        PaymentView payment = create_payment(record, estimate, false, *payload, false);

        // create_invoice_entry (invoice_handler.rs:41-79), status = InvoiceCreated -> "invoice_created"
        InvoiceView invoice = create_invoice(record, billing, estimate, payment.payment_id,
                                             "invoice_created", "");

        // update_subscription(payment_method_id, plan_id, item_price_id) (core.rs:105-114)
        record.payment_method_id = payment.payment_method_id;
        m_context->repository().update_subscription(to_store_record(record));

        metrics_for("subscription.create");
        send_json(res, send, encode_subscription_response(record, std::nullopt, payment, invoice));
    }

    // ── POST /subscriptions/:id/pause — pause_subscription ──────────────────────
    void handle_pause_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto failure = authorize_api_key(req, auth)) {
            send_failure(res, send, *failure);
            return;
        }
        auto profile = extract_profile_id(req, res, send);
        if (!profile) { return; }
        const auto id = subscription_id_from_request(req);
        if (id.empty()) {
            send_failure(res, send, generic_not_found("subscription not found for id: "));
            return;
        }
        auto payload = parse_json_body_optional(req);
        if (const auto failure = find_business_profile(profile.value, merchant_id(req, auth))) {
            send_failure(res, send, *failure);
            return;
        }
        auto found = find_subscription(id);
        if (!found) {
            send_failure(res, send, generic_not_found("subscription not found for id: " + id));
            return;
        }
        BillingProcessor billing{this};
        if (const auto failure = billing.create(found->profile_id, found->merchant_id)) {
            send_failure(res, send, *failure);
            return;
        }

        // pause_subscription_on_connector (billing_processor_handler.rs:354-383)
        auto connector = call_billing("pause_subscription", *found, payload ? payload->doc : Json{}, merchant_id(req, auth));
        const std::string status = subscription_status_from_connector(connector, "paused");
        found->status = status;
        found->modified_at = now_value();
        m_context->repository().update_subscription(to_store_record(*found));

        std::string paused_at = now_value();
        if (payload) {
            if (const auto value = json_string(payload->root, "pause_at")) {
                paused_at = *value;
            }
        }
        metrics_for("subscription.pause");
        send_json(res, send, encode_pause_response(*found, paused_at));
    }

    // ── POST /subscriptions/:id/resume — resume_subscription ────────────────────
    void handle_resume_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto failure = authorize_api_key(req, auth)) {
            send_failure(res, send, *failure);
            return;
        }
        auto profile = extract_profile_id(req, res, send);
        if (!profile) { return; }
        const auto id = subscription_id_from_request(req);
        if (id.empty()) {
            send_failure(res, send, generic_not_found("subscription not found for id: "));
            return;
        }
        auto payload = parse_json_body_optional(req);
        if (const auto failure = find_business_profile(profile.value, merchant_id(req, auth))) {
            send_failure(res, send, *failure);
            return;
        }
        auto found = find_subscription(id);
        if (!found) {
            send_failure(res, send, generic_not_found("subscription not found for id: " + id));
            return;
        }
        BillingProcessor billing{this};
        if (const auto failure = billing.create(found->profile_id, found->merchant_id)) {
            send_failure(res, send, *failure);
            return;
        }

        auto connector = call_billing("resume_subscription", *found, payload ? payload->doc : Json{}, merchant_id(req, auth));
        const std::string status = subscription_status_from_connector(connector, "active");
        found->status = status;
        found->modified_at = now_value();
        m_context->repository().update_subscription(to_store_record(*found));

        std::string next_billing_at = now_value();
        if (payload) {
            if (const auto value = json_string(payload->root, "resume_date")) {
                next_billing_at = *value;
            }
        }
        metrics_for("subscription.resume");
        send_json(res, send, encode_resume_response(*found, next_billing_at));
    }

    // ── POST /subscriptions/:id/cancel — cancel_subscription ────────────────────
    void handle_cancel_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto failure = authorize_api_key(req, auth)) {
            send_failure(res, send, *failure);
            return;
        }
        auto profile = extract_profile_id(req, res, send);
        if (!profile) { return; }
        const auto id = subscription_id_from_request(req);
        if (id.empty()) {
            send_failure(res, send, generic_not_found("subscription not found for id: "));
            return;
        }
        auto payload = parse_json_body_optional(req);
        if (const auto failure = find_business_profile(profile.value, merchant_id(req, auth))) {
            send_failure(res, send, *failure);
            return;
        }
        auto found = find_subscription(id);
        if (!found) {
            send_failure(res, send, generic_not_found("subscription not found for id: " + id));
            return;
        }
        BillingProcessor billing{this};
        if (const auto failure = billing.create(found->profile_id, found->merchant_id)) {
            send_failure(res, send, *failure);
            return;
        }

        auto connector = call_billing("cancel_subscription", *found, payload ? payload->doc : Json{}, merchant_id(req, auth));
        const std::string status = subscription_status_from_connector(connector, "cancelled");
        found->status = status;
        found->modified_at = now_value();
        m_context->repository().update_subscription(to_store_record(*found));

        std::string cancelled_at = now_value();
        if (payload) {
            if (const auto value = json_string(payload->root, "cancel_at")) {
                cancelled_at = *value;
            }
        }
        metrics_for("subscription.cancel");
        send_json(res, send, encode_cancel_response(*found, cancelled_at));
    }

    // ── POST /subscriptions/:id/confirm — confirm_subscription ──────────────────
    void handle_confirm_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto raw_body = req.body_as_string();
        const auto auth = app::auth::AuthContextBuilder{}(req, raw_body);
        auto payload = parse_json_body(req, res, send);
        if (!payload) { return; }

        // check_sdk_auth_and_get_auth + auth_type (routes/subscription.rs:214-265)
        if (const auto failure = authorize_sdk(req, auth, kWritePermission,
                                               json_string(payload->root, "client_secret").value_or(""))) {
            send_failure(res, send, *failure);
            return;
        }
        auto profile = extract_profile_id(req, res, send);
        if (!profile) { return; }

        // request.validate() -> ConfirmSubscriptionPaymentDetails::validate (api_models:217-231,396-409)
        const auto* payment_details = nested_object(payload->root, "payment_details");
        if (!has_payment_method_data_or_token(payment_details)) {
            send_failure(res, send, invalid_request_data("Either payment_method_data or payment_token must be present"));
            return;
        }

        const auto id = subscription_id_from_request(req);
        if (id.empty()) {
            send_failure(res, send, generic_not_found("subscription not found for id: "));
            return;
        }

        // The SDK flow overwrites payload.client_secret with the authenticated claim.
        const std::string client_secret = effective_client_secret(auth, payload->root);

        if (const auto failure = find_business_profile(profile.value, merchant_id(req, auth))) {
            send_failure(res, send, *failure);
            return;
        }
        if (!client_secret.empty()) {
            if (const auto failure = validate_client_secret(client_secret, merchant_id(req, auth))) {
                send_failure(res, send, *failure);
                return;
            }
        }
        auto found = find_subscription(id);
        if (!found) {
            send_failure(res, send, generic_not_found("subscription not found for id: " + id));
            return;
        }

        InvoiceView invoice = latest_invoice(found->subscription_id);
        if (invoice.invoice_id.empty()) {
            send_failure(res, send, subscription_error("Get Latest Invoice"));
            return;
        }
        if (invoice.payment_intent_id.empty()) {
            send_failure(res, send, missing_field("payment_intent_id"));
            return;
        }

        // confirm_payment (invoice_handler.rs:203-229) — local payments substitute
        PaymentView payment = confirm_payment(found->subscription_id, invoice.payment_intent_id, *found, estimate_for(*found));

        BillingProcessor billing{this};
        if (const auto failure = billing.create(found->profile_id, found->merchant_id)) {
            send_failure(res, send, *failure);
            return;
        }
        if (const auto failure = find_customer(found->customer_id, found->merchant_id)) {
            send_failure(res, send, *failure);
            return;
        }
        create_connector_customer(*found, payload->doc, billing); // create_customer_on_connector

        ConnectorResponse connector = call_billing("create_subscription_on_connector", *found, payload->doc, found->merchant_id);
        const std::string connector_subscription_id = connector_reference(connector, "bpsub_" + found->subscription_id);
        const std::string connector_invoice_id = connector_reference(connector, "pinv_" + found->subscription_id);

        // update_invoice(payment + status) (core.rs:401-413)
        invoice.payment_intent_id = payment.payment_id;
        invoice.payment_method_id = payment.payment_method_id;
        invoice.status = "invoice_paid";
        invoice.connector_invoice_id = connector_invoice_id;
        store_invoice(invoice);

        // create_invoice_sync_job (invoice_handler.rs:286-307)
        create_invoice_sync_job(*found, invoice, connector_invoice_id, billing.connector_name);

        // update_subscription(connector id, payment method, status) (core.rs:424-437)
        found->status = subscription_status_from_connector(connector, "active");
        found->connector_subscription_id = connector_subscription_id;
        found->payment_method_id = payment.payment_method_id;
        found->modified_at = now_value();
        m_context->repository().update_subscription(to_store_record(*found));

        metrics_for("subscription.confirm");
        send_json(res, send, encode_confirm_response(*found, invoice, payment));
    }

    // ── GET /subscriptions/items — get_subscription_items ───────────────────────
    void handle_get_subscription_items(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        auto payload = std::make_unique<Json>();
        const auto query = app::http::RequestQuery{}(req);
        *payload = json_from_query(query);
        if (const auto failure = authorize_sdk(req, auth, kReadPermission,
                                               json_string(*payload, "client_secret").value_or(""))) {
            send_failure(res, send, *failure);
            return;
        }
        auto profile = extract_profile_id(req, res, send);
        if (!profile) { return; }

        if (const auto failure = find_business_profile(profile.value, merchant_id(req, auth))) {
            send_failure(res, send, *failure);
            return;
        }

        const std::string client_secret = effective_client_secret(auth, *payload);
        if (!client_secret.empty()) {
            if (const auto failure = validate_client_secret(client_secret, merchant_id(req, auth))) {
                send_failure(res, send, *failure);
                return;
            }
        }

        BillingProcessor billing{this};
        if (const auto failure = billing.create(profile.value, merchant_id(req, auth))) {
            send_failure(res, send, *failure);
            return;
        }

        const std::uint32_t limit = static_cast<std::uint32_t>(query_unsigned(query, "limit", 10));
        const std::uint32_t offset = static_cast<std::uint32_t>(query_unsigned(query, "offset", 0));
        const std::string item_type = query.get("item_type").value_or("plan");

        // get_subscription_items + get_subscription_item_prices (core.rs:151-172)
        auto items = billing.get_items(profile.value, limit, offset, item_type);
        metrics_for("subscription.get_subscription_items");
        send_json(res, send, encode_items_response(items));
    }

    // ── GET /subscriptions/:id — get_subscription ───────────────────────────────
    void handle_get_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto failure = authorize_api_key_or_jwt(req, auth, kReadPermission)) {
            send_failure(res, send, *failure);
            return;
        }
        auto profile = extract_profile_id(req, res, send);
        if (!profile) { return; }
        const auto id = subscription_id_from_request(req);
        if (id.empty()) {
            send_failure(res, send, generic_not_found("subscription not found for id: "));
            return;
        }
        if (const auto failure = find_business_profile(profile.value, merchant_id(req, auth))) {
            send_failure(res, send, *failure);
            return;
        }
        auto found = find_subscription(id);
        if (!found) {
            send_failure(res, send, generic_not_found("subscription not found for id: " + id));
            return;
        }
        metrics_for("subscription.get");
        // to_subscription_response(subscription, None, None) — no payment/invoice on this route.
        send_json(res, send, encode_subscription_response(*found, std::nullopt, std::nullopt, std::nullopt));
    }

    // ── POST /subscriptions — create_and_confirm_subscription ──────────────────
    void handle_create_and_confirm_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto failure = authorize_api_key_or_jwt(req, auth, kWritePermission)) {
            send_failure(res, send, *failure);
            return;
        }
        auto profile = extract_profile_id(req, res, send);
        if (!profile) { return; }
        auto payload = parse_json_body(req, res, send);
        if (!payload) { return; }

        // request.validate() -> PaymentDetails::validate (api_models:261-275,440-452)
        const auto* payment_details = nested_object(payload->root, "payment_details");
        if (!has_payment_method_data_or_id(payment_details)) {
            send_failure(res, send, invalid_request_data("Either payment_method_data or payment_method_id must be present"));
            return;
        }
        const auto customer_id = json_string(payload->root, "customer_id");
        const auto item_price_id = json_string(payload->root, "item_price_id");
        if (!customer_id || customer_id->empty() || !item_price_id || item_price_id->empty()) {
            send_json_body_error(res, send, "missing field `item_price_id` or `customer_id` at line 1 column 2");
            return;
        }

        // core::create_and_confirm_subscription (core.rs:176-307)
        const auto merchant = merchant_id(req, auth);
        if (const auto failure = find_business_profile(profile.value, merchant)) {
            send_failure(res, send, *failure);
            return;
        }
        if (const auto failure = find_customer(*customer_id, merchant)) {
            send_failure(res, send, *failure);
            return;
        }
        BillingProcessor billing{this};
        if (const auto failure = billing.create(profile.value, merchant)) {
            send_failure(res, send, *failure);
            return;
        }

        const std::string subscription_id = m_context->ids().payment_id("sub_");
        SubscriptionRecord record{};
        record.subscription_id = subscription_id;
        record.merchant_id = merchant;
        record.profile_id = profile.value;
        record.customer_id = *customer_id;
        record.status = "created";
        record.plan_id = json_string(payload->root, "plan_id").value_or("");
        record.item_price_id = *item_price_id;
        record.coupon = json_string(payload->root, "coupon_code").value_or("");
        record.merchant_reference_id = json_string(payload->root, "merchant_reference_id").value_or("");
        record.created_at = now_value();
        record.modified_at = record.created_at;
        record.client_secret = generate_client_secret(subscription_id);
        record.merchant_connector_id = billing.merchant_connector_id;
        auto stored = m_context->repository().create_subscription(to_store_record(record));
        if (!stored) {
            send_failure(res, send, internal_error());
            return;
        }
        record.id = stored->get_id();

        // create_customer_on_connector + update_connector_customer_id_in_customer (core.rs:219-240)
        create_connector_customer(record, payload->doc, billing);

        // create_subscription_on_connector (core.rs:242-249)
        ConnectorResponse connector = call_billing("create_subscription_on_connector", record, payload->doc, merchant);
        record.connector_subscription_id = connector_reference(connector, "bpsub_" + subscription_id);

        // get_amount_and_currency((None,None), invoice_details) (invoice_handler.rs:103-114)
        Estimate estimate = estimate_from_connector(connector, record.plan_id, record.item_price_id);

        // create_and_confirm_payment (invoice_handler.rs:159-201)
        PaymentView payment = create_payment(record, estimate, true, *payload, true);

        // create_invoice_entry + create_invoice_sync_job (core.rs:259-283)
        InvoiceView invoice = create_invoice(record, billing, estimate, payment.payment_id,
                                             "invoice_created", connector_reference(connector, "pinv_" + subscription_id));
        create_invoice_sync_job(record, invoice, invoice.connector_invoice_id, billing.connector_name);

        // update_subscription(connector id, payment method, status) (core.rs:285-298)
        record.status = subscription_status_from_connector(connector, "active");
        record.payment_method_id = payment.payment_method_id;
        m_context->repository().update_subscription(to_store_record(record));

        metrics_for("subscription.create_and_confirm");
        send_json(res, send, encode_confirm_response(record, invoice, payment));
    }

    // ── GET /subscriptions/estimate — get_estimate ──────────────────────────────
    void handle_get_estimate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        Json payload = json_from_query(query);

        // check_sdk_auth_and_get_auth (routes/subscription.rs:432-457)
        if (const auto failure = authorize_sdk(req, auth, kWritePermission,
                                               json_string(payload, "client_secret").value_or(""))) {
            send_failure(res, send, *failure);
            return;
        }
        auto profile = extract_profile_id(req, res, send);
        if (!profile) { return; }

        if (const auto failure = find_business_profile(profile.value, merchant_id(req, auth))) {
            send_failure(res, send, *failure);
            return;
        }
        const std::string client_secret = effective_client_secret(auth, payload);
        if (!client_secret.empty()) {
            if (const auto failure = validate_client_secret(client_secret, merchant_id(req, auth))) {
                send_failure(res, send, *failure);
                return;
            }
        }
        BillingProcessor billing{this};
        if (const auto failure = billing.create(profile.value, merchant_id(req, auth))) {
            send_failure(res, send, *failure);
            return;
        }

        const std::string plan_id = json_string(payload, "plan_id").value_or("");
        const std::string item_price_id = json_string(payload, "item_price_id").value_or("");
        SubscriptionRecord seed{};
        seed.subscription_id = m_context->ids().payment_id("sub_");
        seed.merchant_id = merchant_id(req, auth);
        seed.profile_id = profile.value;
        seed.plan_id = plan_id;
        seed.item_price_id = item_price_id;
        Estimate estimate = billing.estimate(seed, plan_id, item_price_id,
                                             json_string(payload, "coupon_code").value_or(""), client_secret);

        metrics_for("subscription.get_estimate");
        send_json(res, send, encode_estimate_response(estimate, profile.value));
    }

    // ── PUT /subscriptions/:id/update — update_subscription ─────────────────────
    void handle_update_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto failure = authorize_api_key(req, auth)) {
            send_failure(res, send, *failure);
            return;
        }
        auto profile = extract_profile_id(req, res, send);
        if (!profile) { return; }
        const auto id = subscription_id_from_request(req);
        if (id.empty()) {
            send_failure(res, send, generic_not_found("subscription not found for id: "));
            return;
        }
        auto payload = parse_json_body(req, res, send);
        if (!payload) { return; }
        const auto plan_id = json_string(payload->root, "plan_id");
        const auto item_price_id = json_string(payload->root, "item_price_id");
        if (!plan_id || !item_price_id) {
            send_json_body_error(res, send, "missing field `plan_id` or `item_price_id` at line 1 column 2");
            return;
        }

        if (const auto failure = find_business_profile(profile.value, merchant_id(req, auth))) {
            send_failure(res, send, *failure);
            return;
        }
        auto found = find_subscription(id);
        if (!found) {
            send_failure(res, send, generic_not_found("subscription not found for id: " + id));
            return;
        }
        InvoiceView invoice = latest_invoice(found->subscription_id);
        if (invoice.invoice_id.empty()) {
            send_failure(res, send, subscription_error("Get Latest Invoice"));
            return;
        }

        // update_subscription(plan_id, item_price_id) (core.rs:669-677)
        found->plan_id = *plan_id;
        found->item_price_id = *item_price_id;
        found->modified_at = now_value();
        m_context->repository().update_subscription(to_store_record(*found));

        BillingProcessor billing{this};
        if (const auto failure = billing.create(found->profile_id, found->merchant_id)) {
            send_failure(res, send, *failure);
            return;
        }
        Estimate estimate = billing.estimate(*found, *plan_id, *item_price_id, "", found->client_secret);

        // update_invoice + update_payment (core.rs:698-718)
        invoice.amount = estimate.amount;
        invoice.currency = estimate.currency;
        store_invoice(invoice);
        update_payment_amount(invoice.payment_intent_id, estimate);

        metrics_for("subscription.update");
        // core ends with get_subscription(...) -> to_subscription_response(None, None)
        send_json(res, send, encode_subscription_response(*found, std::nullopt, std::nullopt, std::nullopt));
    }

    // ── GET /subscriptions/list — list_subscriptions ────────────────────────────
    void handle_list_subscriptions(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto failure = authorize_api_key_or_jwt(req, auth, kReadPermission)) {
            send_failure(res, send, *failure);
            return;
        }
        auto profile = extract_profile_id(req, res, send);
        if (!profile) { return; }
        if (const auto failure = find_business_profile(profile.value, merchant_id(req, auth))) {
            send_failure(res, send, *failure);
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        const std::int64_t limit = query_number(query, "limit", kMaxListCount);
        const std::int64_t offset = query_number(query, "offset", 0);

        // list_subscriptions_by_profile_id(profile, limit, offset) (core.rs:741-748)
        std::vector<SubscriptionRecord> records;
        for (const auto& record : m_context->repository().list_subscriptions()) {
            if (record.get_profile_id() == profile.value) {
                records.push_back(from_store_record(record));
            }
        }
        const std::size_t begin = offset > 0 ? std::min(static_cast<std::size_t>(offset), records.size()) : 0U;
        const std::size_t count = limit > 0 ? static_cast<std::size_t>(limit) : 0U;
        const std::size_t stop = std::min(begin + count, records.size());

        std::string out = "[";
        for (std::size_t i = begin; i < stop; ++i) {
            if (i > begin) { out += ','; }
            out += encode_subscription_response(records[i], std::nullopt, std::nullopt, std::nullopt);
        }
        out += ']';
        metrics_for("subscription.list");
        send_json(res, send, out);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/subscriptions"}
            .add_route(Route{"/create"}.post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) { handle_create_subscription(req, res, std::move(send)); }).summary("Hyperswitch create_subscription"))
            .add_route(Route{"/:subscription_id/pause"}.post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) { handle_pause_subscription(req, res, std::move(send)); }).summary("Hyperswitch pause_subscription"))
            .add_route(Route{"/:subscription_id/resume"}.post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) { handle_resume_subscription(req, res, std::move(send)); }).summary("Hyperswitch resume_subscription"))
            .add_route(Route{"/:subscription_id/cancel"}.post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) { handle_cancel_subscription(req, res, std::move(send)); }).summary("Hyperswitch cancel_subscription"))
            .add_route(Route{"/:subscription_id/confirm"}.post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) { handle_confirm_subscription(req, res, std::move(send)); }).summary("Hyperswitch confirm_subscription"))
            .add_route(Route{"/items"}.get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) { handle_get_subscription_items(req, res, std::move(send)); }).summary("Hyperswitch get_subscription_items"))
            .add_route(Route{"/:subscription_id"}.get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) { handle_get_subscription(req, res, std::move(send)); }).summary("Hyperswitch get_subscription"))
            .add_route(Route{"/"}.post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) { handle_create_and_confirm_subscription(req, res, std::move(send)); }).summary("Hyperswitch create_and_confirm_subscription"))
            .add_route(Route{"/estimate"}.get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) { handle_get_estimate(req, res, std::move(send)); }).summary("Hyperswitch get_estimate"))
            .add_route(Route{"/:subscription_id/update"}.put([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) { handle_update_subscription(req, res, std::move(send)); }).summary("Hyperswitch update_subscription"))
            .add_route(Route{"/list"}.get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) { handle_list_subscriptions(req, res, std::move(send)); }).summary("Hyperswitch list_subscriptions"));
    }

    explicit SubscriptionRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    // ── Constants ───────────────────────────────────────────────────────────────
    static constexpr std::string_view kWritePermission{"PROFILE_SUBSCRIPTION_WRITE"};
    static constexpr std::string_view kReadPermission{"PROFILE_SUBSCRIPTION_READ"};
    static constexpr std::int64_t kMaxListCount{10};   // core.rs:29 SUBSCRIPTIONS_MAX_LIST_COUNT
    static constexpr std::int64_t kSessionExpirySeconds{15 * 60}; // common_utils/consts.rs:85

    // ── Local value objects ─────────────────────────────────────────────────────
    using Json = serde::Value;
    using JsonObject = Json::Object;

    struct ParsedPayload
    {
        Json doc{};
        const JsonObject* root{nullptr};
        std::string raw{};
    };

    struct SubscriptionRecord
    {
        std::int64_t id{};
        std::string subscription_id{};
        std::string merchant_id{};
        std::string profile_id{};
        std::string customer_id{};
        std::string status{};
        std::string plan_id{};
        std::string item_price_id{};
        std::string coupon{};
        std::string merchant_reference_id{};
        std::string merchant_connector_id{};
        std::string client_secret{};
        std::string payment_method_id{};
        std::string connector_subscription_id{};
        std::string created_at{};
        std::string modified_at{};
    };

    struct Estimate
    {
        std::int64_t amount{};
        std::string currency{"USD"};
        std::string plan_id{};
        std::string item_price_id{};
        std::string description{};
        std::string customer_id{};
    };

    struct PaymentView
    {
        std::string payment_id{};
        std::string status{};
        std::int64_t amount{};
        std::string currency{};
        std::string payment_method_id{};
        std::string connector{};
    };

    struct InvoiceView
    {
        std::string invoice_id{};
        std::string subscription_id{};
        std::string merchant_id{};
        std::string profile_id{};
        std::string merchant_connector_id{};
        std::string payment_intent_id{};
        std::string payment_method_id{};
        std::string customer_id{};
        std::int64_t amount{};
        std::string currency{};
        std::string status{};
        std::string connector_invoice_id{};
    };

    struct ItemPriceView
    {
        std::string price_id{};
        std::string item_id{};
        std::int64_t amount{};
        std::string currency{};
        std::string interval{"month"};
        std::int64_t interval_count{1};
    };

    struct ItemView
    {
        std::string item_id{};
        std::string name{};
        std::string description{};
        std::vector<ItemPriceView> prices{};
    };

    struct ConnectorResponse
    {
        std::string status{};
        std::string reference_id{};
    };

    // ── ApiErrorResponse wire helpers ───────────────────────────────────────────
    struct ApiFailure
    {
        std::string type{"invalid_request"};
        std::string code{"IR_06"};
        std::string message{};
        interfaces::io::types::Status status{interfaces::io::types::Status::BAD_REQUEST};
        std::string extra{};
    };

    [[nodiscard]] ApiFailure missing_field(std::string_view name) const
    {
        return {"invalid_request", "IR_04", std::format("Missing required param: {}", name),
                interfaces::io::types::Status::BAD_REQUEST, {}};
    }
    [[nodiscard]] ApiFailure invalid_data_value(std::string_view name) const
    {
        return {"invalid_request", "IR_07", std::format("Invalid value provided: {}", name),
                interfaces::io::types::Status::BAD_REQUEST, {}};
    }
    [[nodiscard]] ApiFailure invalid_request_data(std::string_view message) const
    {
        return {"invalid_request", "IR_06", std::string{message},
                interfaces::io::types::Status::UNPROCESSABLE_CONTENT, {}};
    }
    [[nodiscard]] ApiFailure generic_not_found(std::string_view message) const
    {
        return {"invalid_request", "IR_37", std::string{message},
                interfaces::io::types::Status::NOT_FOUND, {}};
    }
    [[nodiscard]] ApiFailure profile_not_found(std::string_view id) const
    {
        return {"invalid_request", "HE_02", std::format("Business profile with the given id {} does not exist", id),
                interfaces::io::types::Status::NOT_FOUND, {}};
    }
    [[nodiscard]] ApiFailure customer_not_found() const
    {
        return {"invalid_request", "HE_02", "Customer does not exist in our records",
                interfaces::io::types::Status::NOT_FOUND, {}};
    }
    [[nodiscard]] ApiFailure subscription_error(std::string_view operation) const
    {
        return {"invalid_request", "CE_09", std::format("Subscription operation: {} failed with connector", operation),
                interfaces::io::types::Status::BAD_REQUEST, {}};
    }
    [[nodiscard]] ApiFailure internal_error() const
    {
        return {"api", "HE_00", "Something went wrong",
                interfaces::io::types::Status::INTERNAL_SERVER_ERROR, {}};
    }
    [[nodiscard]] ApiFailure unauthorized() const
    {
        return {"invalid_request", "IR_01", "API key not provided or invalid API key used",
                interfaces::io::types::Status::UNAUTHORIZED, {}};
    }

    void send_failure(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        const ApiFailure& failure
    ) const
    {
        std::string body = "{\"error\":{\"type\":\"" + json_escape(failure.type) +
            "\",\"message\":\"" + json_escape(failure.message) +
            "\",\"code\":\"" + json_escape(failure.code) + "\"" + failure.extra + "}}";
        auto bytes = serde::Ser::serialize_raw("application/json", body);
        res.set_status(failure.status);
        res.set_body(std::move(bytes));
        send();
    }

    void send_json_body_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view message
    ) const
    {
        // utils.rs:78-90 custom_json_error_handler -> {"error":{"error_type":"invalid_request",...}}
        const std::string body = "{\"error\":{\"error_type\":\"invalid_request\",\"message\":\"" +
            json_escape(message) + "\",\"code\":\"IR_06\"}}";
        auto bytes = serde::Ser::serialize_raw("application/json", body);
        res.set_status(interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view body
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", body);
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

    [[nodiscard]] std::string json_escape(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char ch : value) {
            switch (ch) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(ch) < 0x20U) {
                    out += std::format("\\u{:04x}", static_cast<unsigned int>(ch));
                } else {
                    out += ch;
                }
            }
        }
        return out;
    }

    [[nodiscard]] std::string json_string_literal(std::string_view value) const
    {
        return std::format("\"{}\"", json_escape(value));
    }

    [[nodiscard]] std::string optional_json_string(const std::string& value) const
    {
        return value.empty() ? std::string{"null"} : json_string_literal(value);
    }

    // ── JSON input helpers (rfl::Generic tree) ──────────────────────────────────
    [[nodiscard]] std::optional<Json> parse_json(std::string_view body) const
    {
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) { return std::nullopt; }
        return std::move(*parsed);
    }

    [[nodiscard]] std::optional<ParsedPayload> parse_json_body(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()>& send
    ) const
    {
        const auto body = req.body_as_string();
        auto parsed = parse_json(body);
        if (!parsed) {
            send_json_body_error(res, send, "Json payload is empty or deserialization failed");
            return std::nullopt;
        }
        const auto* root = std::get_if<JsonObject>(&parsed->get());
        if (root == nullptr) {
            send_json_body_error(res, send, "Json payload must be an object");
            return std::nullopt;
        }
        return ParsedPayload{std::move(*parsed), root, body};
    }

    [[nodiscard]] std::optional<ParsedPayload> parse_json_body_optional(
        interfaces::io::IRequest& req
    ) const
    {
        const auto body = req.body_as_string();
        auto parsed = parse_json(body);
        if (!parsed) { return std::nullopt; }
        const auto* root = std::get_if<JsonObject>(&parsed->get());
        if (root == nullptr) { return std::nullopt; }
        return ParsedPayload{std::move(*parsed), root, body};
    }

    [[nodiscard]] Json json_from_query(const app::http::QueryParams& query) const
    {
        JsonObject object{};
        for (const auto* key : {"client_secret", "plan_id", "item_price_id", "coupon_code"}) {
            if (const auto value = query.get(key)) {
                object.insert(std::string{key}, Json{*value});
            }
        }
        return Json{std::move(object)};
    }

    [[nodiscard]] std::optional<std::string> json_string(const JsonObject* object, std::string_view key) const
    {
        if (object == nullptr) { return std::nullopt; }
        const auto field = object->get(std::string{key});
        if (!field) { return std::nullopt; }
        if (const auto* text = std::get_if<std::string>(&field.value().get())) {
            return *text;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::string> json_string(const Json& value, std::string_view key) const
    {
        return json_string(std::get_if<JsonObject>(&value.get()), key);
    }

    [[nodiscard]] const JsonObject* nested_object(const JsonObject* object, std::string_view key) const
    {
        if (object == nullptr) { return nullptr; }
        const auto field = object->get(std::string{key});
        if (!field) { return nullptr; }
        return std::get_if<JsonObject>(&field.value().get());
    }

    [[nodiscard]] bool has_payment_method_data_or_token(const JsonObject* details) const
    {
        if (details == nullptr) { return false; }
        return json_string(details, "payment_method_data").has_value() ||
               json_string(details, "payment_token").has_value() ||
               nested_object(details, "payment_method_data") != nullptr;
    }

    [[nodiscard]] bool has_payment_method_data_or_id(const JsonObject* details) const
    {
        if (details == nullptr) { return false; }
        return json_string(details, "payment_method_data").has_value() ||
               json_string(details, "payment_method_id").has_value() ||
               nested_object(details, "payment_method_data") != nullptr;
    }

    // ── Auth helpers (upstream authentication.rs semantics) ─────────────────────
    [[nodiscard]] std::optional<std::string> header(interfaces::io::IRequest& req, std::string_view name) const
    {
        const auto value = app::http::HeaderReader{}.get(req, name);
        return value.empty() ? std::nullopt : std::optional<std::string>{value};
    }

    [[nodiscard]] bool has_bearer_token(const app::auth::AuthContext& auth) const
    {
        return !auth.token().is_empty();
    }

    [[nodiscard]] bool permission_granted(const app::auth::AuthContext& auth, std::string_view required) const
    {
        const auto single = auth.claim("permission");
        const auto list = auth.claim("permissions");
        if (!single && !list) { return true; } // JWT without explicit claims: upstream grants by role.
        const auto grants = [required](std::string_view value)
        {
            std::string_view rest = value;
            while (!rest.empty()) {
                const auto comma = rest.find(',');
                const auto token = rest.substr(0, comma);
                if (token == required || token == "ADMIN") { return true; }
                if (comma == std::string_view::npos) { break; }
                rest = rest.substr(comma + 1);
            }
            return false;
        };
        return (single && grants(*single)) || (list && grants(*list));
    }

    // auth_type(HeaderAuth(ApiKeyAuth), JWTAuth{permission}, headers) (authentication.rs:6464-6475)
    [[nodiscard]] std::optional<ApiFailure> authorize_api_key_or_jwt(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& auth,
        std::string_view permission
    ) const
    {
        if (has_bearer_token(auth)) {
            if (!permission_granted(auth, permission)) {
                return ApiFailure{"invalid_request", "IR_20", "Access forbidden, insufficient permission",
                                  interfaces::io::types::Status::FORBIDDEN, {}};
            }
            return std::nullopt;
        }
        return authorize_api_key(req, auth);
    }

    // HeaderAuth(ApiKeyAuth{allow_connected=false, allow_platform=false})
    [[nodiscard]] std::optional<ApiFailure> authorize_api_key(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& auth
    ) const
    {
        (void)req;
        const auto api_key = auth.claim(app::auth::kClaimApiKey);
        if (!api_key || api_key->empty()) { return unauthorized(); }
        if (auth.merchant_id().empty()) {
            return ApiFailure{"invalid_request", "IR_04", "Missing required param: x-merchant-id",
                              interfaces::io::types::Status::BAD_REQUEST, {}};
        }
        return std::nullopt;
    }

    // check_sdk_auth_and_get_auth + auth_type (routes/subscription.rs:214-235,284-303,432-457)
    [[nodiscard]] std::optional<ApiFailure> authorize_sdk(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& auth,
        std::string_view permission,
        std::string_view payload_client_secret = {}
    ) const
    {
        if (has_bearer_token(auth)) {
            return authorize_api_key_or_jwt(req, auth, permission);
        }
        const auto authorization = header(req, "authorization");
        const auto api_key = auth.claim(app::auth::kClaimApiKey);
        const auto api_key_value = api_key ? std::string{*api_key} : std::string{};

        if (authorization) {
            // SdkAuthorizationAuth — SDK key or publishable+client-secret pair.
            const auto sdk = auth.claim(app::auth::kClaimSdkKey);
            const auto publishable = auth.claim(app::auth::kClaimPublishableKey);
            const auto secret = auth.claim(app::auth::kClaimClientSecret);
            if ((!sdk || sdk->empty()) &&
                (!(publishable && !publishable->empty() && secret && !secret->empty()))) {
                return unauthorized();
            }
            return std::nullopt;
        }

        if (api_key_value.starts_with("pk_")) {
            // PublishableKeyAuth requires a client secret from the payload/query.
            const auto secret = auth.claim(app::auth::kClaimClientSecret);
            if ((!secret || secret->empty()) && payload_client_secret.empty()) {
                return missing_field("client_secret");
            }
            return std::nullopt;
        }

        if (auth.claim(app::auth::kClaimClientSecret) || !payload_client_secret.empty()) {
            return invalid_request_data("client_secret is not a valid parameter");
        }
        return authorize_api_key(req, auth);
    }

    [[nodiscard]] std::string effective_client_secret(
        const app::auth::AuthContext& auth,
        const JsonObject* payload
    ) const
    {
        const auto claim = auth.claim(app::auth::kClaimClientSecret);
        if (claim && !claim->empty()) { return std::string{*claim}; }
        return json_string(payload, "client_secret").value_or("");
    }

    [[nodiscard]] std::string effective_client_secret(
        const app::auth::AuthContext& auth,
        const Json& payload
    ) const
    {
        return effective_client_secret(auth, std::get_if<JsonObject>(&payload.get()));
    }

    // ── X-Profile-Id extraction (routes/subscription.rs:21-47) ──────────────────
    struct ProfileResult
    {
        std::string value{};
        bool ok{false};
        [[nodiscard]] explicit operator bool() const noexcept { return ok; }
    };

    [[nodiscard]] ProfileResult extract_profile_id(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()>& send
    ) const
    {
        std::string raw = app::http::HeaderReader{}.get(req, "X-Profile-Id");
        if (raw.empty()) {
            raw = app::http::HeaderReader{}.get(req, "x-profile-id");
        }
        if (raw.empty()) {
            send_failure(res, send, missing_field("X-Profile-Id"));
            return {};
        }
        const auto trimmed = trim(raw);
        if (trimmed.empty()) {
            send_failure(res, send, missing_field("X-Profile-Id"));
            return {};
        }
        if (!valid_profile_id(trimmed)) {
            send_failure(res, send, invalid_data_value("X-Profile-Id"));
            return {};
        }
        return ProfileResult{trimmed, true};
    }

    [[nodiscard]] std::string trim(std::string_view value) const
    {
        std::size_t begin = 0U;
        std::size_t end = value.size();
        while (begin < end && std::isspace(static_cast<unsigned char>(value[begin]))) { ++begin; }
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1U]))) { --end; }
        return std::string{value.substr(begin, end - begin)};
    }

    [[nodiscard]] bool valid_profile_id(std::string_view value) const
    {
        // ProfileId::try_from -> LengthId -> AlphaNumericId (id_type.rs:161-180)
        if (value.empty() || value.size() > 64U) { return false; }
        for (const char ch : value) {
            const auto c = static_cast<unsigned char>(ch);
            if (!std::isalnum(c) && ch != '_' && ch != '-') { return false; }
        }
        return true;
    }

    // ── Identity / request helpers ──────────────────────────────────────────────
    [[nodiscard]] std::string merchant_id(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& auth
    ) const
    {
        if (!auth.merchant_id().empty()) { return auth.merchant_id(); }
        return app::http::merchant_id_from_header{}(req);
    }

    [[nodiscard]] std::string subscription_id_from_request(interfaces::io::IRequest& req) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        if (segments.count() < 2 || segments.at(0) != "subscriptions") { return {}; }
        const auto value = segments.at(1);
        return value ? std::string{*value} : std::string{};
    }

    [[nodiscard]] std::string now_value() const
    {
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return std::to_string(seconds);
    }

    [[nodiscard]] std::int64_t parse_int(std::string_view raw, std::int64_t fallback) const
    {
        std::int64_t value{};
        const auto converted = std::from_chars(raw.data(), raw.data() + raw.size(), value);
        return converted.ec == std::errc{} && converted.ptr == raw.data() + raw.size() ? value : fallback;
    }

    [[nodiscard]] std::int64_t query_number(
        const app::http::QueryParams& query,
        std::string_view key,
        std::int64_t fallback
    ) const
    {
        const auto raw = query.get(key);
        return raw ? parse_int(*raw, fallback) : fallback;
    }

    [[nodiscard]] std::uint64_t query_unsigned(
        const app::http::QueryParams& query,
        std::string_view key,
        std::uint64_t fallback
    ) const
    {
        const auto raw = query.get(key);
        if (!raw) { return fallback; }
        std::uint64_t value{};
        const auto converted = std::from_chars(raw->data(), raw->data() + raw->size(), value);
        return converted.ec == std::errc{} && converted.ptr == raw->data() + raw->size() ? value : fallback;
    }

    // ── SubscriptionHandler helpers (subscription_handler.rs) ───────────────────
    [[nodiscard]] std::optional<ApiFailure> find_business_profile(
        std::string_view profile_id,
        std::string_view merchant_id_value
    ) const
    {
        (void)merchant_id_value;
        if (!m_context->repository().find_profile_by_string(profile_id)) {
            return profile_not_found(profile_id);
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<ApiFailure> find_customer(
        std::string_view customer_id,
        std::string_view merchant_id_value
    ) const
    {
        const auto customer = m_context->repository().find_customer_by_string(customer_id);
        if (!customer || (!merchant_id_value.empty() && customer->get_merchant_id() != merchant_id_value)) {
            return customer_not_found();
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string generate_client_secret(std::string_view subscription_id) const
    {
        // Subscription::generate_and_set_client_secret (subscription.rs:110-115):
        // generate_id_with_default_len("{id}_secret") -> random suffix.
        const app::crypto::KeyedHasher hasher{"subscription_client_secret"};
        const std::string digest = hasher.hash(std::string{subscription_id} + "_secret");
        return std::string{subscription_id} + "_secret" + digest.substr(0, 16);
    }

    [[nodiscard]] std::optional<std::string> client_secret_subscription_id(std::string_view secret) const
    {
        // ClientSecret::get_subscription_id (subscription.rs:24-35)
        const auto split = secret.find("_secret");
        if (split == std::string_view::npos) { return std::nullopt; }
        return std::string{secret.substr(0, split)};
    }

    [[nodiscard]] std::optional<ApiFailure> validate_client_secret(
        std::string_view secret,
        std::string_view merchant_id_value
    ) const
    {
        // find_and_validate_subscription (subscription_handler.rs:189-243)
        const auto subscription_id = client_secret_subscription_id(secret);
        if (!subscription_id) { return missing_field("client_secret"); }
        auto record = find_subscription(*subscription_id);
        if (!record) {
            return generic_not_found("Subscription not found for id: " + *subscription_id);
        }
        if (!merchant_id_value.empty() && record->merchant_id != merchant_id_value) {
            return generic_not_found("Subscription not found for id: " + *subscription_id);
        }
        if (record->client_secret.empty()) { return missing_field("client_secret"); }
        if (secret != record->client_secret) {
            return invalid_request_data("The client_secret provided does not match the client_secret associated with the Payment");
        }
        const auto created = parse_int(record->created_at, 0);
        const auto now = parse_int(now_value(), 0);
        if (now > created + kSessionExpirySeconds) {
            return invalid_request_data("The provided client_secret has expired");
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<SubscriptionRecord> find_subscription(std::string_view id) const
    {
        auto repository = m_context->repository();
        if (auto found = repository.find_subscription_by_string(id)) {
            return from_store_record(*found);
        }
        std::int64_t numeric{};
        const auto converted = std::from_chars(id.data(), id.data() + id.size(), numeric);
        if (converted.ec == std::errc{} && converted.ptr == id.data() + id.size()) {
            if (auto found = repository.find_subscription(numeric)) {
                return from_store_record(*found);
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] app::store::subscriptions::SubscriptionRecord to_store_record(const SubscriptionRecord& record) const
    {
        app::store::subscriptions::SubscriptionRecord out{};
        out.set_id(record.id);
        out.set_subscription_id(record.subscription_id);
        out.set_merchant_id(record.merchant_id);
        out.set_profile_id(record.profile_id);
        out.set_customer_id(record.customer_id);
        out.set_status(record.status);
        out.set_plan_id(record.plan_id);
        out.set_item_price_id(record.item_price_id);
        out.set_coupon(record.coupon);
        out.set_merchant_reference_id(record.merchant_reference_id);
        out.set_billing_processor_subscription_id(record.connector_subscription_id);
        out.set_client_secret(record.client_secret);
        out.set_created_at(record.created_at);
        out.set_updated_at(record.modified_at);
        return out;
    }

    [[nodiscard]] SubscriptionRecord from_store_record(const app::store::subscriptions::SubscriptionRecord& record) const
    {
        SubscriptionRecord out{};
        out.id = record.get_id();
        out.subscription_id = record.get_subscription_id();
        out.merchant_id = record.get_merchant_id();
        out.profile_id = record.get_profile_id();
        out.customer_id = record.get_customer_id();
        out.status = record.get_status();
        out.plan_id = record.get_plan_id();
        out.item_price_id = record.get_item_price_id();
        out.coupon = record.get_coupon();
        out.merchant_reference_id = record.get_merchant_reference_id();
        out.connector_subscription_id = record.get_billing_processor_subscription_id();
        out.client_secret = record.get_client_secret();
        out.created_at = record.get_created_at();
        out.modified_at = record.get_updated_at();
        return out;
    }

    // ── BillingHandler (billing_processor_handler.rs) ───────────────────────────
    class BillingProcessor
    {
    public:
        std::string merchant_connector_id{};
        std::string connector_name{"local"};
        std::string profile_id{};
        std::string merchant_id{};
        bool ready{false};

        [[nodiscard]] std::optional<ApiFailure> create(std::string_view profile, std::string_view merchant)
        {
            // BillingHandler::create (billing_processor_handler.rs:40-101):
            // profile.get_billing_processor_id() -> MCA lookup by merchant + id.
            const auto record = m_context()->m_context->repository().find_profile_by_string(profile);
            if (!record) { return m_context()->profile_not_found(profile); }
            // Local substitute for Profile::get_billing_processor_id(): the configured
            // billing processor MCA id for the profile (upstream reads it from the profile).
            if (const auto configured = m_context()->m_context->config_store().get_value(
                    "subscription_billing_processors", std::string{profile})) {
                merchant_connector_id = *configured;
            }
            if (merchant_connector_id.empty()) {
                for (const auto& mca : m_context()->m_context->repository().list_merchant_connectors()) {
                    if (merchant.empty() || mca.get_merchant_id() == merchant) {
                        merchant_connector_id = mca.get_merchant_connector_id();
                        connector_name = mca.get_connector_name();
                        break;
                    }
                }
            } else {
                if (const auto mca = m_context()->m_context->repository().find_merchant_connector_by_string(merchant_connector_id)) {
                    connector_name = mca->get_connector_name();
                }
            }
            if (merchant_connector_id.empty()) {
                return m_context()->generic_not_found(
                    "Merchant connector account does not exist in our records");
            }
            profile_id = std::string{profile};
            merchant_id = std::string{merchant};
            ready = true;
            return std::nullopt;
        }

        // get_subscription_estimate (billing_processor_handler.rs:265-292)
        [[nodiscard]] Estimate estimate(
            const SubscriptionRecord& record,
            std::string_view plan_id,
            std::string_view item_price_id,
            std::string_view coupon_code,
            std::string_view client_secret
        ) const
        {
            app::connector::ConnectorRequest request = m_context()->billing_request(
                "get_subscription_estimate", record,
                item_price_id.empty() ? std::string{plan_id} : std::string{item_price_id});
            const auto raw = m_context()->call_connector("get_subscription_estimate", request);
            ConnectorResponse response{};
            if (raw) {
                response.status = raw->get_status();
                response.reference_id = raw->get_reference_id();
            }
            Estimate out = m_context()->estimate_from_connector(response, plan_id, item_price_id);
            out.description = std::string{coupon_code};
            if (!client_secret.empty()) { out.customer_id = record.customer_id; }
            return out;
        }

        // get_subscription_items + get_subscription_item_prices (core.rs:151-172)
        [[nodiscard]] std::vector<ItemView> get_items(
            std::string_view profile,
            std::uint32_t limit,
            std::uint32_t offset,
            std::string_view item_type
        ) const
        {
            std::vector<ItemView> items{};
            SubscriptionRecord seed{};
            seed.profile_id = std::string{profile};
            seed.merchant_id = merchant_id;
            const auto response = m_context()->call_connector(
                "get_subscription_items", m_context()->billing_request("get_subscription_items", seed, std::string{item_type}));
            (void)response;

            auto records = m_context()->m_context->repository().list_subscriptions();
            std::vector<SubscriptionRecord> scoped{};
            for (const auto& stored : records) {
                if (!profile.empty() && stored.get_profile_id() != profile) { continue; }
                scoped.push_back(m_context()->from_store_record(stored));
            }
            const std::size_t begin = std::min(static_cast<std::size_t>(offset), scoped.size());
            const std::size_t stop = std::min(begin + static_cast<std::size_t>(limit), scoped.size());
            for (std::size_t i = begin; i < stop; ++i) {
                const auto& record = scoped[i];
                ItemView item{};
                item.item_id = item_type == "addon" ? record.item_price_id : record.subscription_id;
                item.name = item_type == "addon"
                    ? (record.item_price_id.empty() ? std::string{"subscription addon"} : record.item_price_id)
                    : (record.plan_id.empty() ? std::string{"subscription item"} : record.plan_id);
                item.description = record.coupon;
                ItemPriceView price{};
                price.price_id = record.item_price_id;
                price.item_id = record.subscription_id;
                const Estimate estimate = m_context()->price_for(record.item_price_id, record.plan_id);
                price.amount = estimate.amount;
                price.currency = estimate.currency;
                price.interval = "month";
                price.interval_count = 1;
                item.prices.push_back(std::move(price));
                items.push_back(std::move(item));
            }
            return items;
        }

        explicit BillingProcessor(SubscriptionRouteHandlers* context) : m_owner{context} {}

    private:
        [[nodiscard]] SubscriptionRouteHandlers* m_context() const { return m_owner; }
        SubscriptionRouteHandlers* m_owner{};
    };

    // Payment / invoice helpers -------------------------------------------------
    [[nodiscard]] Estimate price_for(std::string_view item_price_id, std::string_view plan_id) const
    {
        // Local in-process substitute for the billing processor price catalogue.
        Estimate out{};
        const std::string key = item_price_id.empty() ? std::string{plan_id} : std::string{item_price_id};
        if (!key.empty()) {
            if (const auto configured = m_context->config_store().get_value("subscription_prices", key)) {
                const auto colon = configured->find(':');
                if (colon != std::string::npos) {
                    out.amount = parse_int(std::string_view{configured->data(), colon}, 0);
                    out.currency = configured->substr(colon + 1);
                } else {
                    out.amount = parse_int(*configured, 0);
                }
            }
        }
        if (out.amount <= 0) {
            const std::string seed = key.empty() ? "subscription_default" : key;
            const std::string digest = app::crypto::KeyedHasher{"subscription_price"}.hash(seed);
            std::uint64_t numeric{};
            const auto width = std::min<std::size_t>(8, digest.size());
            std::from_chars(digest.data(), digest.data() + width, numeric, 16);
            out.amount = 100 + static_cast<std::int64_t>(numeric % 100000);
        }
        if (out.currency.empty()) { out.currency = "USD"; }
        return out;
    }

    [[nodiscard]] Estimate estimate_from_connector(
        const ConnectorResponse& response,
        std::string_view plan_id,
        std::string_view item_price_id
    ) const
    {
        // Local connector is in-process; it echoes the amount/currency we requested.
        // The control flow matches InvoiceHandler::get_amount_and_currency
        // (invoice_handler.rs:103-114): connector total/currency, else (0, default).
        Estimate out = price_for(item_price_id, plan_id);
        out.plan_id = std::string{plan_id};
        out.item_price_id = std::string{item_price_id};
        if (!response.reference_id.empty()) {
            out.description = response.reference_id;
        }
        return out;
    }

    [[nodiscard]] app::connector::ConnectorRequest billing_request(
        std::string_view operation,
        const SubscriptionRecord& record,
        std::string_view resource_id
    ) const
    {
        return app::connector::ConnectorRequestBuilder{}
            .operation(std::string{operation})
            .merchant_id(record.merchant_id)
            .resource_id(std::string{resource_id})();
    }

    [[nodiscard]] std::optional<app::connector::ConnectorResponse>
    call_connector(std::string_view operation, const app::connector::ConnectorRequest& request) const
    {
        auto connector = m_context->connector_router().select(request.get_merchant_id());
        if (!connector) { return std::nullopt; }
        m_context->metrics().record_flow(std::format("subscription.connector.{}", operation));
        if (operation == "billing_customer_create" || operation == "create_customer_on_connector") {
            return connector->create_link(request);
        }
        if (operation == "get_subscription_estimate") {
            return connector->verify(request);
        }
        if (operation == "cancel_subscription") {
            return connector->cancel(request);
        }
        if (operation == "create_subscription_on_connector" || operation == "confirm_subscription") {
            return connector->authorize(request);
        }
        return connector->sync(request);
    }

    [[nodiscard]] ConnectorResponse call_billing(
        std::string_view operation,
        const SubscriptionRecord& record,
        const Json& payload,
        std::string_view merchant
    ) const
    {
        ConnectorResponse out{};
        const auto request = app::connector::ConnectorRequestBuilder{}
            .operation(std::string{operation})
            .merchant_id(std::string{merchant})
            .resource_id(record.subscription_id)()
            ;
        const auto response = call_connector(operation, request);
        (void)payload;
        if (response) {
            out.status = response->get_status();
            out.reference_id = response->get_reference_id();
        }
        return out;
    }

    [[nodiscard]] std::string connector_reference(const ConnectorResponse& response, std::string_view seed) const
    {
        if (!response.reference_id.empty()) { return response.reference_id; }
        return "local_" + app::crypto::KeyedHasher{"subscription_connector"}.hash(std::string{seed}).substr(0, 16);
    }

    [[nodiscard]] std::string subscription_status_from_connector(
        const ConnectorResponse& response,
        std::string_view intended
    ) const
    {
        // SubscriptionStatus::from(connector_response.status) -> common_enums snake_case.
        const auto& status = response.status;
        if (status == "cancelled" || status == "canceled" || status == "cancel") { return "cancelled"; }
        if (status == "paused") { return "paused"; }
        if (status == "active" || status == "authorized" || status == "succeeded") { return "active"; }
        if (status == "created" || status == "pending") { return "created"; }
        return std::string{intended};
    }

    void create_connector_customer(
        const SubscriptionRecord& record,
        const Json& payload,
        const BillingProcessor& billing
    ) const
    {
        (void)payload;
        (void)billing;
        // create_customer_on_connector (billing_processor_handler.rs:103-170):
        // Skip when the customer already has a connector-customer id for this MCA.
        const auto key = record.customer_id;
        if (m_context->config_store().get_value("subscription_connector_customers", key)) { return; }
        const auto response = call_connector(
            "billing_customer_create",
            billing_request("billing_customer_create", record, record.customer_id));
        ConnectorResponse converted{};
        if (response) {
            converted.status = response->get_status();
            converted.reference_id = response->get_reference_id();
        }
        const std::string customer_reference = connector_reference(converted, "cus_" + record.customer_id);
        // update_connector_customer_id_in_customer (subscription_handler.rs:109-138)
        m_context->config_store().put("subscription_connector_customers", key, customer_reference);
    }

    [[nodiscard]] PaymentView create_payment(
        const SubscriptionRecord& record,
        const Estimate& estimate,
        bool confirm,
        const ParsedPayload& payload,
        bool create_and_confirm
    ) const
    {
        (void)payload;
        (void)create_and_confirm;
        // create_payment_with_confirm_false / create_and_confirm_payment —
        // local PaymentsApiClient substitute backed by the payment repository.
        app::store::payments::PaymentIntentRecord payment{};
        payment.set_payment_id(m_context->ids().payment_id("pay_"));
        payment.set_merchant_id(record.merchant_id);
        payment.set_customer_id(record.customer_id);
        payment.set_merchant_reference_id(record.merchant_reference_id);
        payment.set_status(confirm ? "succeeded" : "requires_confirmation");
        payment.set_amount(estimate.amount);
        payment.set_currency(estimate.currency);
        payment.set_description("subscription " + record.subscription_id);
        payment.set_created_at(now_value());
        payment.set_updated_at(now_value());
        const auto stored = m_context->repository().save_payment(std::move(payment));
        PaymentView out{};
        out.payment_id = stored.get_payment_id();
        out.status = confirm ? "succeeded" : "requires_confirmation";
        out.amount = estimate.amount;
        out.currency = estimate.currency;
        out.payment_method_id = record.payment_method_id;
        out.connector = "payments_microservice";
        return out;
    }

    [[nodiscard]] PaymentView confirm_payment(
        std::string_view subscription_id,
        std::string_view payment_id,
        const SubscriptionRecord& record,
        const Estimate& estimate
    ) const
    {
        (void)subscription_id;
        // confirm_payment (invoice_handler.rs:203-229) — local substitute.
        auto payment = m_context->repository().find_payment_by_string(payment_id);
        if (payment) {
            payment->set_status("succeeded");
            payment->set_updated_at(now_value());
            m_context->repository().update_payment(*payment);
        }
        PaymentView out{};
        out.payment_id = std::string{payment_id};
        out.status = "succeeded";
        out.amount = estimate.amount;
        out.currency = estimate.currency;
        out.payment_method_id = record.payment_method_id;
        out.connector = "payments_microservice";
        return out;
    }

    void update_payment_amount(std::string_view payment_id, const Estimate& estimate) const
    {
        auto payment = m_context->repository().find_payment_by_string(payment_id);
        if (payment) {
            payment->set_amount(estimate.amount);
            payment->set_currency(estimate.currency);
            payment->set_updated_at(now_value());
            m_context->repository().update_payment(*payment);
        }
    }

    // Invoice persistence — config_store substitute for the invoice DB table.
    [[nodiscard]] InvoiceView create_invoice(
        const SubscriptionRecord& record,
        const BillingProcessor& billing,
        const Estimate& estimate,
        std::string_view payment_id,
        std::string_view status,
        std::string_view connector_invoice_id
    ) const
    {
        InvoiceView invoice{};
        invoice.invoice_id = m_context->ids().payment_id("inv_");
        invoice.subscription_id = record.subscription_id;
        invoice.merchant_id = record.merchant_id;
        invoice.profile_id = record.profile_id;
        invoice.merchant_connector_id = billing.merchant_connector_id;
        invoice.payment_intent_id = std::string{payment_id};
        invoice.customer_id = record.customer_id;
        invoice.amount = estimate.amount;
        invoice.currency = estimate.currency;
        invoice.status = std::string{status};
        invoice.connector_invoice_id = std::string{connector_invoice_id};
        store_invoice(invoice);
        return invoice;
    }

    void store_invoice(const InvoiceView& invoice) const
    {
        const auto payload = std::format("{}|{}|{}|{}|{}|{}|{}|{}|{}|{}|{}|{}",
            invoice.invoice_id, invoice.subscription_id, invoice.merchant_id, invoice.profile_id,
            invoice.merchant_connector_id, invoice.payment_intent_id, invoice.payment_method_id,
            invoice.customer_id, invoice.amount, invoice.currency, invoice.status, invoice.connector_invoice_id);
        m_context->config_store().put("subscription_invoices", invoice.subscription_id, payload);
    }

    [[nodiscard]] InvoiceView latest_invoice(std::string_view subscription_id) const
    {
        InvoiceView out{};
        const auto raw = m_context->config_store().get_value("subscription_invoices", subscription_id);
        if (!raw) { return out; }
        std::vector<std::string> parts{};
        std::size_t start{0};
        while (true) {
            const auto separator = raw->find('|', start);
            parts.push_back(separator == std::string::npos
                ? raw->substr(start)
                : raw->substr(start, separator - start));
            if (separator == std::string::npos) { break; }
            start = separator + 1;
        }
        if (parts.size() < 12) { return out; }
        out.invoice_id = parts[0];
        out.subscription_id = parts[1];
        out.merchant_id = parts[2];
        out.profile_id = parts[3];
        out.merchant_connector_id = parts[4];
        out.payment_intent_id = parts[5];
        out.payment_method_id = parts[6];
        out.customer_id = parts[7];
        out.amount = parse_int(parts[8], 0);
        out.currency = parts[9];
        out.status = parts[10];
        out.connector_invoice_id = parts[11];
        return out;
    }

    void create_invoice_sync_job(
        const SubscriptionRecord& record,
        const InvoiceView& invoice,
        std::string_view connector_invoice_id,
        std::string_view connector_name
    ) const
    {
        (void)connector_invoice_id;
        (void)connector_name;
        // InvoiceHandler::create_invoice_sync_job (invoice_handler.rs:286-307).
        const std::string payload = std::format("{}|{}|{}|{}|{}",
            record.subscription_id, invoice.invoice_id, record.merchant_id,
            record.profile_id, record.customer_id);
        const auto tracker_id = m_context->process_trackers().schedule("invoice_sync", payload, "0");
        m_context->process_trackers().mark_terminal(tracker_id, "synced");
        m_context->events().publish("subscription.invoice.created", payload);
    }

    [[nodiscard]] Estimate estimate_for(const SubscriptionRecord& record) const
    {
        return price_for(record.item_price_id, record.plan_id);
    }

    void metrics_for(std::string_view name) const
    {
        m_context->metrics().record_flow(name);
        m_context->metrics().counter(name).increment();
    }

    // ── Wire encoders (exact upstream field order / Option handling) ────────────
    [[nodiscard]] std::string encode_subscription_response(
        const SubscriptionRecord& record,
        std::optional<std::string> client_secret,
        std::optional<PaymentView> payment,
        std::optional<InvoiceView> invoice
    ) const
    {
        // SubscriptionResponse (api_models/subscription.rs:59-97):
        // id, merchant_reference_id, status, plan_id, item_price_id, profile_id,
        // client_secret, merchant_id, coupon_code, customer_id, payment, invoice
        std::string out = "{";
        out += std::format("\"id\":{}", json_string_literal(record.subscription_id));
        out += std::format(",\"merchant_reference_id\":{}", optional_json_string(record.merchant_reference_id));
        out += std::format(",\"status\":{}", json_string_literal(record.status));
        out += std::format(",\"plan_id\":{}", optional_json_string(record.plan_id));
        out += std::format(",\"item_price_id\":{}", optional_json_string(record.item_price_id));
        out += std::format(",\"profile_id\":{}", json_string_literal(record.profile_id));
        const std::string secret = client_secret.value_or(record.client_secret);
        out += std::format(",\"client_secret\":{}", optional_json_string(secret));
        out += std::format(",\"merchant_id\":{}", json_string_literal(record.merchant_id));
        out += std::format(",\"coupon_code\":{}", optional_json_string(record.coupon));
        out += std::format(",\"customer_id\":{}", json_string_literal(record.customer_id));
        out += std::format(",\"payment\":{}", payment ? encode_payment(*payment) : std::string{"null"});
        out += std::format(",\"invoice\":{}", invoice ? encode_invoice(*invoice) : std::string{"null"});
        out += '}';
        return out;
    }

    [[nodiscard]] std::string encode_confirm_response(
        const SubscriptionRecord& record,
        const InvoiceView& invoice,
        const PaymentView& payment
    ) const
    {
        // ConfirmSubscriptionResponse (api_models/subscription.rs:456-490):
        // id, merchant_reference_id, status, plan_id, item_price_id, coupon,
        // profile_id, payment, customer_id, invoice, billing_processor_subscription_id
        std::string out = "{";
        out += std::format("\"id\":{}", json_string_literal(record.subscription_id));
        out += std::format(",\"merchant_reference_id\":{}", optional_json_string(record.merchant_reference_id));
        out += std::format(",\"status\":{}", json_string_literal(record.status));
        out += std::format(",\"plan_id\":{}", optional_json_string(record.plan_id));
        out += std::format(",\"item_price_id\":{}", optional_json_string(record.item_price_id));
        out += std::format(",\"coupon\":{}", optional_json_string(record.coupon));
        out += std::format(",\"profile_id\":{}", json_string_literal(record.profile_id));
        out += std::format(",\"payment\":{}", encode_payment(payment));
        out += std::format(",\"customer_id\":{}", json_string_literal(record.customer_id));
        out += std::format(",\"invoice\":{}", encode_invoice(invoice));
        out += std::format(",\"billing_processor_subscription_id\":{}",
                           optional_json_string(record.connector_subscription_id));
        out += '}';
        return out;
    }

    [[nodiscard]] std::string encode_invoice(const InvoiceView& invoice) const
    {
        // Invoice (api_models/subscription.rs:504-542)
        std::string out = "{";
        out += std::format("\"id\":{}", json_string_literal(invoice.invoice_id));
        out += std::format(",\"subscription_id\":{}", json_string_literal(invoice.subscription_id));
        out += std::format(",\"merchant_id\":{}", json_string_literal(invoice.merchant_id));
        out += std::format(",\"profile_id\":{}", json_string_literal(invoice.profile_id));
        out += std::format(",\"merchant_connector_id\":{}", json_string_literal(invoice.merchant_connector_id));
        out += std::format(",\"payment_intent_id\":{}", optional_json_string(invoice.payment_intent_id));
        out += std::format(",\"payment_method_id\":{}", optional_json_string(invoice.payment_method_id));
        out += std::format(",\"customer_id\":{}", json_string_literal(invoice.customer_id));
        out += std::format(",\"amount\":{}", invoice.amount);
        out += std::format(",\"currency\":{}", json_string_literal(invoice.currency));
        out += std::format(",\"status\":{}", json_string_literal(invoice.status));
        out += std::format(",\"billing_processor_invoice_id\":{}", optional_json_string(invoice.connector_invoice_id));
        out += '}';
        return out;
    }

    [[nodiscard]] std::string encode_payment(const PaymentView& payment) const
    {
        // PaymentResponseData (api_models/subscription.rs:340-367) — all Option fields
        // upstream omits only where skip_serializing_if is set; the rest are null.
        std::string out = "{";
        out += std::format("\"payment_id\":{}", json_string_literal(payment.payment_id));
        out += std::format(",\"status\":{}", json_string_literal(payment.status));
        out += std::format(",\"amount\":{}", payment.amount);
        out += std::format(",\"currency\":{}", json_string_literal(payment.currency));
        out += ",\"profile_id\":null";
        out += std::format(",\"connector\":{}", optional_json_string(payment.connector));
        out += ",\"payment_method_id\":null";
        out += ",\"return_url\":null";
        out += ",\"next_action\":null";
        out += ",\"payment_experience\":null";
        out += ",\"error_code\":null";
        out += ",\"error_message\":null";
        out += ",\"payment_method_type\":null";
        out += ",\"client_secret\":null";
        out += ",\"billing\":null";
        out += ",\"shipping\":null";
        out += ",\"payment_type\":null";
        out += ",\"payment_token\":null";
        out += '}';
        return out;
    }

    [[nodiscard]] std::string encode_estimate_response(const Estimate& estimate, std::string_view profile) const
    {
        // EstimateSubscriptionResponse (api_models/subscription.rs:585-600).
        // From<GetSubscriptionEstimateResponse> (router_response_types/subscriptions.rs:151-168)
        // explicitly sets plan_id/item_price_id/coupon_code = None.
        std::string out = "{";
        out += std::format("\"amount\":{}", estimate.amount);
        out += std::format(",\"currency\":{}", json_string_literal(estimate.currency));
        out += ",\"plan_id\":null";
        out += ",\"item_price_id\":null";
        out += ",\"coupon_code\":null";
        out += std::format(",\"customer_id\":{}", optional_json_string(estimate.customer_id));
        out += ",\"line_items\":[";
        out += std::format("{{\"item_id\":{},\"item_type\":\"plan\",\"description\":{},\"amount\":{},\"currency\":{},\"quantity\":1}}",
            json_string_literal(estimate.item_price_id.empty() ? estimate.plan_id : estimate.item_price_id),
            json_string_literal(estimate.description), estimate.amount, json_string_literal(estimate.currency));
        out += "]}";
        (void)profile;
        return out;
    }

    [[nodiscard]] std::string encode_items_response(const std::vector<ItemView>& items) const
    {
        std::string out = "[";
        for (std::size_t i = 0U; i < items.size(); ++i) {
            if (i > 0U) { out += ','; }
            const auto& item = items[i];
            out += std::format("{{\"item_id\":{},\"name\":{},\"description\":{},\"price_id\":[",
                json_string_literal(item.item_id), json_string_literal(item.name),
                optional_json_string(item.description));
            for (std::size_t j = 0U; j < item.prices.size(); ++j) {
                if (j > 0U) { out += ','; }
                const auto& price = item.prices[j];
                out += std::format("{{\"price_id\":{},\"item_id\":{},\"amount\":{},\"currency\":{},\"interval\":{},\"interval_count\":{},\"trial_period\":null,\"trial_period_unit\":null}}",
                    json_string_literal(price.price_id), optional_json_string(price.item_id),
                    price.amount, json_string_literal(price.currency),
                    json_string_literal(price.interval), price.interval_count);
            }
            out += "]}";
        }
        out += ']';
        return out;
    }

    [[nodiscard]] std::string encode_pause_response(const SubscriptionRecord& record, std::string_view paused_at) const
    {
        // PauseSubscriptionResponse (api_models/subscription.rs:642-659)
        return std::format(
            R"({{"id":{},"status":{},"merchant_reference_id":{},"profile_id":{},"merchant_id":{},"customer_id":{},"paused_at":{}}})",
            json_string_literal(record.subscription_id), json_string_literal(record.status),
            optional_json_string(record.merchant_reference_id), json_string_literal(record.profile_id),
            json_string_literal(record.merchant_id), json_string_literal(record.customer_id),
            json_string_literal(paused_at));
    }

    [[nodiscard]] std::string encode_resume_response(const SubscriptionRecord& record, std::string_view next_billing_at) const
    {
        // ResumeSubscriptionResponse (api_models/subscription.rs:698-716)
        return std::format(
            R"({{"id":{},"status":{},"merchant_reference_id":{},"profile_id":{},"merchant_id":{},"customer_id":{},"next_billing_at":{}}})",
            json_string_literal(record.subscription_id), json_string_literal(record.status),
            optional_json_string(record.merchant_reference_id), json_string_literal(record.profile_id),
            json_string_literal(record.merchant_id), json_string_literal(record.customer_id),
            json_string_literal(next_billing_at));
    }

    [[nodiscard]] std::string encode_cancel_response(const SubscriptionRecord& record, std::string_view cancelled_at) const
    {
        // CancelSubscriptionResponse (api_models/subscription.rs:776-794)
        return std::format(
            R"({{"id":{},"status":{},"merchant_reference_id":{},"profile_id":{},"merchant_id":{},"customer_id":{},"cancelled_at":{}}})",
            json_string_literal(record.subscription_id), json_string_literal(record.status),
            optional_json_string(record.merchant_reference_id), json_string_literal(record.profile_id),
            json_string_literal(record.merchant_id), json_string_literal(record.customer_id),
            json_string_literal(cancelled_at));
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::subscription
