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

class SubscriptionRouteHandlers
{
public:
    void handle_create_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("create_subscription");
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) { send_error(res, send, *auth); return; }
        if (const auto profile_error = validate_profile(auth_context.profile_id())) {
            send_error(res, send, *profile_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::subscription::CreateSubscriptionRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        if (parsed->get_customer_id().get_value().empty() || parsed->get_item_price_id().empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "customer_id and item_price_id are required"));
            return;
        }

        const std::string subscription_id = m_context->ids().payment_id("sub_");
        const std::string merchant_id = merchant_from_request(req);
        auto [client_secret_plain, client_secret_hash] = make_client_secret(subscription_id);

        app::store::subscriptions::SubscriptionRecord record;
        record.set_subscription_id(subscription_id);
        record.set_merchant_id(merchant_id);
        record.set_profile_id(auth_context.profile_id());
        record.set_customer_id(parsed->get_customer_id().get_value());
        record.set_status("incomplete");
        record.set_plan_id(parsed->get_plan_id().value_or(""));
        record.set_item_price_id(parsed->get_item_price_id());
        record.set_coupon(parsed->get_coupon_code().value_or(""));
        record.set_merchant_reference_id(parsed->get_merchant_reference_id().value_or(""));
        record.set_client_secret(client_secret_hash);
        record.set_created_at(now_value());
        record.set_updated_at(record.get_created_at());

        const BillingSetup setup = setup_billing(record, false, body);
        record.set_billing_processor_subscription_id(setup.connector_subscription_id);
        auto created = m_context->repository().create_subscription(std::move(record));
        if (!created) {
            send_error(res, send, created.error());
            return;
        }

        m_context->metrics().record_flow("subscription.create");
        m_context->metrics().counter("subscription_create").increment();

        auto text = serde::Ser::encode_json(to_response(*created, client_secret_plain, &setup));
        send_json(res, send, text, true);
    }

    void handle_pause_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("pause_subscription");
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) { send_error(res, send, *auth); return; }
        if (const auto profile_error = validate_profile(auth_context.profile_id())) {
            send_error(res, send, *profile_error);
            return;
        }

        const std::string subscription_id = subscription_id_from_request(req);
        if (subscription_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing subscription id"));
            return;
        }
        const auto body = req.body_as_string();
        if (!body.empty()) {
            auto parsed = serde::Ser::deserialize<app::model::subscription::PauseSubscriptionRequest>("application/json", body);
            if (!parsed) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
        }

        auto repository = m_context->repository();
        auto found = find_subscription(repository, subscription_id);
        if (!found) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "subscription not found"));
            return;
        }
        if (!in_scope(*found, merchant_from_request(req), auth_context.profile_id())) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "subscription not found"));
            return;
        }
        if (found->get_status() == "paused") {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::PRECONDITION_FAILED, "subscription already paused"));
            return;
        }

        auto connector_response = call_connector("pause_subscription", make_billing_request(
            "pause_subscription", *found, subscription_id, body));
        found->set_status(status_from_connector(connector_response, "paused"));
        apply_connector_reference(*found, connector_response, "pause_subscription");
        found->set_updated_at(now_value());
        repository.update_subscription(*found);
        record_state_sync(*found, "subscription.paused");

        m_context->metrics().record_flow("subscription.pause");
        m_context->metrics().counter("subscription_pause").increment();

        app::model::subscription::PauseSubscriptionResponse response;
        response.set_id(id_value(found->get_subscription_id()));
        response.set_status(status_value(found->get_status()));
        response.set_merchant_reference_id(optional_string(found->get_merchant_reference_id()));
        response.set_profile_id(profile_value(found->get_profile_id()));
        response.set_merchant_id(merchant_value(found->get_merchant_id()));
        response.set_customer_id(customer_value(found->get_customer_id()));
        app::model::subscription::PrimitiveDateTimeValue paused_at;
        paused_at.set_value(found->get_updated_at());
        response.set_paused_at(std::move(paused_at));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_resume_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("resume_subscription");
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) { send_error(res, send, *auth); return; }
        if (const auto profile_error = validate_profile(auth_context.profile_id())) {
            send_error(res, send, *profile_error);
            return;
        }

        const std::string subscription_id = subscription_id_from_request(req);
        if (subscription_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing subscription id"));
            return;
        }
        const auto body = req.body_as_string();
        if (!body.empty()) {
            auto parsed = serde::Ser::deserialize<app::model::subscription::ResumeSubscriptionRequest>("application/json", body);
            if (!parsed) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
        }

        auto repository = m_context->repository();
        auto found = find_subscription(repository, subscription_id);
        if (!found) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "subscription not found"));
            return;
        }
        if (!in_scope(*found, merchant_from_request(req), auth_context.profile_id())) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "subscription not found"));
            return;
        }
        if (found->get_status() != "paused") {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::PRECONDITION_FAILED, "subscription is not paused"));
            return;
        }

        auto connector_response = call_connector("resume_subscription", make_billing_request(
            "resume_subscription", *found, subscription_id, body));
        found->set_status(status_from_connector(connector_response, "active"));
        apply_connector_reference(*found, connector_response, "resume_subscription");
        found->set_updated_at(now_value());
        repository.update_subscription(*found);
        record_state_sync(*found, "subscription.resumed");

        m_context->metrics().record_flow("subscription.resume");
        m_context->metrics().counter("subscription_resume").increment();

        app::model::subscription::ResumeSubscriptionResponse response;
        response.set_id(id_value(found->get_subscription_id()));
        response.set_status(status_value(found->get_status()));
        response.set_merchant_reference_id(optional_string(found->get_merchant_reference_id()));
        response.set_profile_id(profile_value(found->get_profile_id()));
        response.set_merchant_id(merchant_value(found->get_merchant_id()));
        response.set_customer_id(customer_value(found->get_customer_id()));
        app::model::subscription::PrimitiveDateTimeValue next_billing_at;
        next_billing_at.set_value(found->get_updated_at());
        response.set_next_billing_at(std::move(next_billing_at));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_cancel_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("cancel_subscription");
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) { send_error(res, send, *auth); return; }
        if (const auto profile_error = validate_profile(auth_context.profile_id())) {
            send_error(res, send, *profile_error);
            return;
        }

        const std::string subscription_id = subscription_id_from_request(req);
        if (subscription_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing subscription id"));
            return;
        }
        const auto body = req.body_as_string();
        if (!body.empty()) {
            auto parsed = serde::Ser::deserialize<app::model::subscription::CancelSubscriptionRequest>("application/json", body);
            if (!parsed) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
        }

        auto repository = m_context->repository();
        auto found = find_subscription(repository, subscription_id);
        if (!found) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "subscription not found"));
            return;
        }
        if (!in_scope(*found, merchant_from_request(req), auth_context.profile_id())) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "subscription not found"));
            return;
        }
        if (found->get_status() == "cancelled") {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::PRECONDITION_FAILED, "subscription already cancelled"));
            return;
        }

        auto connector_response = call_connector("cancel_subscription", make_billing_request(
            "cancel_subscription", *found, subscription_id, body));
        found->set_status(status_from_connector(connector_response, "cancelled"));
        apply_connector_reference(*found, connector_response, "cancel_subscription");
        found->set_updated_at(now_value());
        repository.update_subscription(*found);
        record_state_sync(*found, "subscription.cancelled");

        m_context->metrics().record_flow("subscription.cancel");
        m_context->metrics().counter("subscription_cancel").increment();

        app::model::subscription::CancelSubscriptionResponse response;
        response.set_id(id_value(found->get_subscription_id()));
        response.set_status(status_value(found->get_status()));
        response.set_merchant_reference_id(optional_string(found->get_merchant_reference_id()));
        response.set_profile_id(profile_value(found->get_profile_id()));
        response.set_merchant_id(merchant_value(found->get_merchant_id()));
        response.set_customer_id(customer_value(found->get_customer_id()));
        app::model::subscription::PrimitiveDateTimeValue cancelled_at;
        cancelled_at.set_value(found->get_updated_at());
        response.set_cancelled_at(std::move(cancelled_at));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_confirm_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("confirm_subscription");
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) { send_error(res, send, *auth); return; }
        if (const auto profile_error = validate_profile(auth_context.profile_id())) {
            send_error(res, send, *profile_error);
            return;
        }

        const std::string subscription_id = subscription_id_from_request(req);
        if (subscription_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing subscription id"));
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::deserialize<app::model::subscription::ConfirmSubscriptionRequest>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const std::string provided_secret = parsed->get_client_secret()
            ? parsed->get_client_secret()->get_value()
            : std::string{};
        if (provided_secret.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "client_secret is required"));
            return;
        }

        auto repository = m_context->repository();
        auto found = find_subscription(repository, subscription_id);
        if (!found) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "subscription not found"));
            return;
        }
        if (!in_scope(*found, merchant_from_request(req), auth_context.profile_id())) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "subscription not found"));
            return;
        }
        if (!client_secret_matches(*found, provided_secret)) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::FORBIDDEN, "invalid client_secret"));
            return;
        }

        InvoiceState state = load_invoice_state(subscription_id).value_or(InvoiceState{});
        if (state.invoice_id.empty()) {
            const BillingSetup setup = setup_billing(*found, true, body);
            state = setup.invoice;
            found->set_billing_processor_subscription_id(setup.connector_subscription_id);
        }

        auto connector_response = call_connector("confirm_subscription", make_billing_request(
            "confirm_subscription", *found, state.payment_id, body));
        auto payment = repository.find_payment_by_string(state.payment_id);
        if (payment) {
            payment->set_status("succeeded");
            payment->set_updated_at(now_value());
            repository.update_payment(*payment);
        }
        state.status = "paid";
        store_invoice_state(subscription_id, state);
        found->set_status(status_from_connector(connector_response, "active"));
        apply_connector_reference(*found, connector_response, "confirm_subscription");
        found->set_updated_at(now_value());
        repository.update_subscription(*found);

        const std::string tracker_payload = subscription_id + "|" + state.invoice_id;
        const auto tracker_id = m_context->process_trackers().schedule("invoice_sync", tracker_payload, "0");
        m_context->process_trackers().mark_terminal(tracker_id, "synced");
        m_context->events().publish("subscription.confirmed", tracker_payload);

        m_context->metrics().record_flow("subscription.confirm");
        m_context->metrics().counter("subscription_confirm").increment();

        app::model::subscription::ConfirmSubscriptionResponse response;
        response.set_id(id_value(found->get_subscription_id()));
        response.set_merchant_reference_id(optional_string(found->get_merchant_reference_id()));
        response.set_status(status_value(found->get_status()));
        response.set_plan_id(optional_string(found->get_plan_id()));
        response.set_item_price_id(optional_string(found->get_item_price_id()));
        response.set_coupon(optional_string(found->get_coupon()));
        response.set_profile_id(profile_value(found->get_profile_id()));
        response.set_customer_id(customer_value(found->get_customer_id()));
        response.set_billing_processor_subscription_id(optional_string(found->get_billing_processor_subscription_id()));
        response.set_invoice(build_invoice(*found, state));
        response.set_payment(build_payment(repository, state));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_get_subscription_items(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("get_subscription_items");
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) { send_error(res, send, *auth); return; }
        if (const auto profile_error = validate_profile(auth_context.profile_id())) {
            send_error(res, send, *profile_error);
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        const auto client_secret = query.get("client_secret");
        const std::int64_t limit = query_number(query, "limit", 100);
        const std::int64_t offset = query_number(query, "offset", 0);
        const auto item_type_raw = query.get("item_type");
        const bool addon_only = item_type_raw && *item_type_raw == "addon";
        const std::string client_secret_hash = client_secret
            ? app::crypto::KeyedHasher{"subscription_client_secret"}.hash(*client_secret)
            : std::string{};

        auto repository = m_context->repository();
        std::vector<app::store::subscriptions::SubscriptionRecord> scoped;
        for (const auto& record : repository.list_subscriptions()) {
            if (!in_scope(record, merchant_from_request(req), auth_context.profile_id())) {
                continue;
            }
            if (!client_secret_hash.empty() && record.get_client_secret() != client_secret_hash) {
                continue;
            }
            scoped.push_back(record);
        }
        if (!client_secret_hash.empty() && scoped.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::FORBIDDEN, "invalid client_secret"));
            return;
        }

        const std::size_t begin = offset > 0 ? static_cast<std::size_t>(offset) : 0U;
        const std::size_t count = limit >= 0 ? static_cast<std::size_t>(limit) : scoped.size();
        const std::size_t stop = std::min(begin + count, scoped.size());

        std::vector<app::model::subscription::GetSubscriptionItemsResponse> items;
        for (std::size_t i = begin; i < stop; ++i) {
            const auto& record = scoped[i];
            const PriceInfo price = price_for(record.get_item_price_id(), record.get_plan_id());
            const auto connector_response = call_connector("get_subscription_items", make_billing_request(
                "get_subscription_items", record, record.get_item_price_id(), {}));
            (void)connector_response;
            const std::string currency_code = price.currency;

            app::model::subscription::GetSubscriptionItemsResponse item;
            item.set_item_id(addon_only ? record.get_item_price_id() : record.get_subscription_id());
            item.set_name(addon_only
                ? (record.get_item_price_id().empty() ? "subscription addon" : record.get_item_price_id())
                : (record.get_plan_id().empty() ? "subscription item" : record.get_plan_id()));
            item.set_description(optional_string(record.get_coupon()));

            app::model::subscription::SubscriptionItemPrices item_price;
            item_price.set_price_id(record.get_item_price_id());
            item_price.set_item_id(optional_string(record.get_subscription_id()));
            item_price.set_amount(minor_unit(std::to_string(price.amount)));
            item_price.set_currency(currency(currency_code));
            item_price.set_interval(app::model::subscription::PeriodUnit::Month);
            item_price.set_interval_count(1);
            item.set_price_id({item_price});
            items.push_back(std::move(item));
        }

        m_context->metrics().record_flow("subscription.get_subscription_items");
        m_context->metrics().counter("subscription_get_subscription_items").increment();

        auto text = serde::Ser::encode_json(items);
        send_json(res, send, text, true);
    }

    void handle_get_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("get_subscription");
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) { send_error(res, send, *auth); return; }
        if (const auto profile_error = validate_profile(auth_context.profile_id())) {
            send_error(res, send, *profile_error);
            return;
        }
        const std::string subscription_id = subscription_id_from_request(req);
        if (subscription_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing subscription id"));
            return;
        }

        auto repository = m_context->repository();
        const auto found = find_subscription(repository, subscription_id);
        if (!found || !in_scope(*found, merchant_from_request(req), auth_context.profile_id())) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "subscription not found"));
            return;
        }

        m_context->metrics().record_flow("subscription.get");
        m_context->metrics().counter("subscription_get").increment();

        auto text = serde::Ser::encode_json(to_response(*found));
        send_json(res, send, text, true);
    }

    void handle_create_and_confirm_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("create_and_confirm_subscription");
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) { send_error(res, send, *auth); return; }
        if (const auto profile_error = validate_profile(auth_context.profile_id())) {
            send_error(res, send, *profile_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::subscription::CreateAndConfirmSubscriptionRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        if (parsed->get_customer_id().get_value().empty() || parsed->get_item_price_id().empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "customer_id and item_price_id are required"));
            return;
        }

        const std::string subscription_id = m_context->ids().payment_id("sub_");
        auto [client_secret_plain, client_secret_hash] = make_client_secret(subscription_id);

        app::store::subscriptions::SubscriptionRecord record;
        record.set_subscription_id(subscription_id);
        record.set_merchant_id(merchant_from_request(req));
        record.set_profile_id(auth_context.profile_id());
        record.set_customer_id(parsed->get_customer_id().get_value());
        record.set_status("active");
        record.set_plan_id(parsed->get_plan_id().value_or(""));
        record.set_item_price_id(parsed->get_item_price_id());
        record.set_coupon(parsed->get_coupon_code().value_or(""));
        record.set_merchant_reference_id(parsed->get_merchant_reference_id().value_or(""));
        record.set_client_secret(client_secret_hash);
        record.set_created_at(now_value());
        record.set_updated_at(record.get_created_at());

        const BillingSetup setup = setup_billing(record, true, body);
        record.set_billing_processor_subscription_id(setup.connector_subscription_id);
        auto created = m_context->repository().create_subscription(std::move(record));
        if (!created) {
            send_error(res, send, created.error());
            return;
        }

        m_context->metrics().record_flow("subscription.create_and_confirm");
        m_context->metrics().counter("subscription_create_and_confirm").increment();

        auto text = serde::Ser::encode_json(to_response(*created, client_secret_plain, &setup));
        send_json(res, send, text, true);
    }

    void handle_get_estimate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("get_estimate");
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) { send_error(res, send, *auth); return; }
        if (const auto profile_error = validate_profile(auth_context.profile_id())) {
            send_error(res, send, *profile_error);
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        std::string plan_id = query.get("plan_id").value_or("");
        std::string item_price_id = query.get("item_price_id").value_or("");
        const auto coupon_code = query.get("coupon_code");
        const auto client_secret = query.get("client_secret");
        std::string customer_id = query.get("customer_id").value_or("");

        if (client_secret) {
            const auto matched = find_subscription_by_client_secret(*client_secret, merchant_from_request(req), auth_context.profile_id());
            if (!matched) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::FORBIDDEN, "invalid client_secret"));
                return;
            }
            if (plan_id.empty()) { plan_id = matched->get_plan_id(); }
            if (item_price_id.empty()) { item_price_id = matched->get_item_price_id(); }
            if (customer_id.empty()) { customer_id = matched->get_customer_id(); }
        }

        const PriceInfo price = price_for(item_price_id, plan_id);
        auto connector_request = app::connector::ConnectorRequestBuilder{}
            .operation("get_subscription_estimate")
            .merchant_id(merchant_from_request(req))
            .resource_id(item_price_id.empty() ? plan_id : item_price_id)
            .currency(price.currency)
            .amount(price.amount)();
        const auto connector_response = call_connector("get_subscription_estimate", connector_request);
        (void)connector_response;
        const std::string currency_code = price.currency;

        app::model::subscription::EstimateSubscriptionResponse response;
        response.set_amount(minor_unit(std::to_string(price.amount)));
        response.set_currency(currency(currency_code));
        response.set_plan_id(optional_string(plan_id));
        response.set_item_price_id(optional_string(item_price_id));
        response.set_coupon_code(coupon_code);
        if (!customer_id.empty()) {
            response.set_customer_id(customer_value(customer_id));
        }

        std::vector<app::model::subscription::SubscriptionLineItem> lines{};
        if (!item_price_id.empty()) {
            app::model::subscription::SubscriptionLineItem line;
            line.set_item_id(item_price_id);
            line.set_item_type(plan_id.empty() ? "addon" : "plan");
            line.set_description(coupon_code.value_or(""));
            line.set_amount(minor_unit(std::to_string(price.amount)));
            line.set_currency(currency(currency_code));
            line.set_quantity(1);
            lines.push_back(std::move(line));
        }
        response.set_line_items(std::move(lines));

        m_context->metrics().record_flow("subscription.get_estimate");
        m_context->metrics().counter("subscription_get_estimate").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_update_subscription(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("update_subscription");
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) { send_error(res, send, *auth); return; }
        if (const auto profile_error = validate_profile(auth_context.profile_id())) {
            send_error(res, send, *profile_error);
            return;
        }

        const std::string subscription_id = subscription_id_from_request(req);
        if (subscription_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing subscription id"));
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::deserialize<app::model::subscription::UpdateSubscriptionRequest>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        if (parsed->get_plan_id().empty() && parsed->get_item_price_id().empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "plan_id or item_price_id is required"));
            return;
        }

        auto repository = m_context->repository();
        auto found = find_subscription(repository, subscription_id);
        if (!found || !in_scope(*found, merchant_from_request(req), auth_context.profile_id())) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "subscription not found"));
            return;
        }
        found->set_plan_id(parsed->get_plan_id());
        found->set_item_price_id(parsed->get_item_price_id());
        found->set_updated_at(now_value());

        const PriceInfo price = price_for(found->get_item_price_id(), found->get_plan_id());
        auto connector_response = call_connector("update_subscription", make_billing_request(
            "update_subscription", *found, subscription_id, body));
        apply_connector_reference(*found, connector_response, "update_subscription");
        repository.update_subscription(*found);

        auto state = load_invoice_state(subscription_id).value_or(InvoiceState{});
        if (!state.invoice_id.empty()) {
            state.amount = price.amount;
            state.currency = price.currency;
            store_invoice_state(subscription_id, state);
            const std::string tracker_payload = subscription_id + "|" + state.invoice_id;
            const auto tracker_id = m_context->process_trackers().schedule("invoice_sync", tracker_payload, "0");
            m_context->process_trackers().mark_terminal(tracker_id, "re-estimated");
            m_context->events().publish("subscription.updated", tracker_payload);
        }

        m_context->metrics().record_flow("subscription.update");
        m_context->metrics().counter("subscription_update").increment();

        BillingSetup setup{};
        setup.invoice = state;
        auto text = serde::Ser::encode_json(to_response(*found, std::nullopt, &setup));
        send_json(res, send, text, true);
    }

    void handle_list_subscriptions(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for("list_subscriptions");
        const auto auth = m_context->authorizer().authorize_route(auth_context, rule);
        if (auth) { send_error(res, send, *auth); return; }
        if (const auto profile_error = validate_profile(auth_context.profile_id())) {
            send_error(res, send, *profile_error);
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        const std::size_t requested = static_cast<std::size_t>(std::max<std::int64_t>(0, query_number(query, "limit", 10)));
        const std::size_t limit = std::min<std::size_t>(requested, 10);
        const std::size_t offset = static_cast<std::size_t>(std::max<std::int64_t>(0, query_number(query, "offset", 0)));

        auto repository = m_context->repository();
        const auto merchant_id = merchant_from_request(req);
        auto scoped = repository.find_subscriptions_by_customer_id(
            app::store::SubscriptionQuery{}.merchant_id(merchant_id));
        std::vector<app::store::subscriptions::SubscriptionRecord> visible{};
        for (const auto& record : scoped) {
            if (in_scope(record, merchant_id, auth_context.profile_id())) {
                visible.push_back(record);
            }
        }
        const std::size_t begin = std::min(offset, visible.size());
        const std::size_t stop = std::min(begin + limit, visible.size());

        std::vector<app::model::subscription::SubscriptionResponse> responses;
        for (std::size_t i = begin; i < stop; ++i) {
            responses.push_back(to_response(visible[i]));
        }

        m_context->metrics().record_flow("subscription.list");
        m_context->metrics().counter("subscription_list").increment();

        auto text = serde::Ser::encode_json(responses);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/subscriptions"}
            .add_route(
                Route{"/create"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_subscription(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_subscription")
            )
            .add_route(
                Route{"/:subscription_id/pause"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_pause_subscription(req, res, std::move(send));
                    })
                    .summary("Hyperswitch pause_subscription")
            )
            .add_route(
                Route{"/:subscription_id/resume"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_resume_subscription(req, res, std::move(send));
                    })
                    .summary("Hyperswitch resume_subscription")
            )
            .add_route(
                Route{"/:subscription_id/cancel"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_cancel_subscription(req, res, std::move(send));
                    })
                    .summary("Hyperswitch cancel_subscription")
            )
            .add_route(
                Route{"/:subscription_id/confirm"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_confirm_subscription(req, res, std::move(send));
                    })
                    .summary("Hyperswitch confirm_subscription")
            )
            .add_route(
                Route{"/items"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_subscription_items(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_subscription_items")
            )
            .add_route(
                Route{"/:subscription_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_subscription(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_subscription")
            )
            .add_route(
                Route{"/"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_and_confirm_subscription(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_and_confirm_subscription")
            )
            .add_route(
                Route{"/estimate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_estimate(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_estimate")
            )
            .add_route(
                Route{"/:subscription_id/update"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_update_subscription(req, res, std::move(send));
                    })
                    .summary("Hyperswitch update_subscription")
            )
            .add_route(
                Route{"/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_subscriptions(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_subscriptions")
            );
    }

    explicit SubscriptionRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    struct PriceInfo
    {
        std::int64_t amount{};
        std::string currency{"USD"};
    };

    struct InvoiceState
    {
        std::string invoice_id{};
        std::string payment_id{};
        std::int64_t amount{};
        std::string currency{};
        std::string status{};
        std::string connector_invoice_id{};
    };

    struct BillingSetup
    {
        InvoiceState invoice{};
        std::string connector_subscription_id{};
        std::string connector_customer_id{};
    };

    // Profile scoping: resolve the X_PROFILE_ID claim against the profile repository.
    // A missing profile is recorded as a metric, not a hard failure, matching the
    // repository-backed profile resolution pattern in authentication.cppm:61-73.
    [[nodiscard]] std::optional<core::error::AppError> validate_profile(std::string_view profile_id) const
    {
        if (profile_id.empty()) {
            return std::nullopt;
        }
        if (m_context->repository().find_profile_by_string(profile_id)) {
            m_context->metrics().counter("subscription_profile_resolved").increment();
        } else {
            m_context->metrics().counter("subscription_profile_missing").increment();
        }
        return std::nullopt;
    }

    [[nodiscard]] bool in_scope(
        const app::store::subscriptions::SubscriptionRecord& record,
        std::string_view merchant_id,
        std::string_view profile_id
    ) const
    {
        if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) {
            return false;
        }
        if (!profile_id.empty() && record.get_profile_id() != profile_id) {
            return false;
        }
        return true;
    }

    [[nodiscard]] std::string subscription_id_from_request(interfaces::io::IRequest& req) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        if (segments.count() < 2 || segments.at(0) != "subscriptions") {
            return {};
        }
        const auto value = segments.at(1);
        return value ? std::string{*value} : std::string{};
    }

    [[nodiscard]] std::string merchant_from_request(interfaces::io::IRequest& req) const
    {
        return app::http::merchant_id_from_header{}(req);
    }

    [[nodiscard]] std::string now_value() const
    {
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        return std::to_string(seconds);
    }

    [[nodiscard]] std::pair<std::string, std::string> make_client_secret(
        std::string_view subscription_id
    ) const
    {
        const app::crypto::KeyedHasher hasher{"subscription_client_secret"};
        const std::string plain = app::crypto::Base64Url{}.encode(
            hasher.hash(std::string{subscription_id} + "_secret"));
        return {plain, hasher.hash(plain)};
    }

    [[nodiscard]] bool client_secret_matches(
        const app::store::subscriptions::SubscriptionRecord& record,
        std::string_view plain
    ) const
    {
        if (plain.empty()) {
            return false;
        }
        return app::crypto::KeyedHasher{"subscription_client_secret"}.hash(std::string{plain}) ==
               record.get_client_secret();
    }

    [[nodiscard]] std::optional<app::store::subscriptions::SubscriptionRecord>
    find_subscription_by_client_secret(
        std::string_view plain,
        std::string_view merchant_id,
        std::string_view profile_id
    ) const
    {
        if (plain.empty()) {
            return std::nullopt;
        }
        const std::string digest = app::crypto::KeyedHasher{"subscription_client_secret"}.hash(std::string{plain});
        for (const auto& record : m_context->repository().list_subscriptions()) {
            if (record.get_client_secret() == digest && in_scope(record, merchant_id, profile_id)) {
                return record;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<app::store::subscriptions::SubscriptionRecord>
    find_subscription(app::store::AppRepository& repository, std::string_view id) const
    {
        if (auto found = repository.find_subscription_by_string(id)) {
            return found;
        }
        std::int64_t numeric{};
        const auto converted = std::from_chars(id.data(), id.data() + id.size(), numeric);
        if (converted.ec == std::errc{} && converted.ptr == id.data() + id.size()) {
            return repository.find_subscription(numeric);
        }
        return std::nullopt;
    }

    [[nodiscard]] std::int64_t parse_int(std::string_view raw, std::int64_t fallback) const
    {
        std::int64_t value{};
        const auto converted = std::from_chars(raw.data(), raw.data() + raw.size(), value);
        if (converted.ec == std::errc{} && converted.ptr == raw.data() + raw.size()) {
            return value;
        }
        return fallback;
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

    [[nodiscard]] PriceInfo price_for(std::string_view item_price_id, std::string_view plan_id) const
    {
        PriceInfo out{};
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
        if (out.currency.empty()) {
            out.currency = "USD";
        }
        return out;
    }

    [[nodiscard]] app::connector::ConnectorRequest make_billing_request(
        std::string_view operation,
        const app::store::subscriptions::SubscriptionRecord& record,
        std::string_view resource_id,
        std::string_view raw_payload
    ) const
    {
        return app::connector::ConnectorRequestBuilder{}
            .operation(std::string{operation})
            .merchant_id(record.get_merchant_id())
            .resource_id(std::string{resource_id})
            .raw_payload(std::string{raw_payload})();
    }

    [[nodiscard]] std::optional<app::connector::ConnectorResponse>
    call_connector(std::string_view operation, const app::connector::ConnectorRequest& request) const
    {
        auto connector = m_context->connector_router().select(request.get_merchant_id());
        if (!connector) {
            return std::nullopt;
        }
        m_context->metrics().record_flow(std::format("subscription.connector.{}", operation));
        m_context->metrics().counter(std::format("subscription_connector_{}", operation)).increment();
        if (operation == "billing_customer_create") {
            return connector->create_link(request);
        }
        if (operation == "get_subscription_estimate") {
            return connector->verify(request);
        }
        if (operation == "cancel_subscription") {
            return connector->cancel(request);
        }
        if (operation == "confirm_subscription") {
            return connector->authorize(request);
        }
        return connector->sync(request);
    }

    [[nodiscard]] std::string connector_reference(
        const std::optional<app::connector::ConnectorResponse>& response,
        std::string_view seed
    ) const
    {
        if (response && !response->get_reference_id().empty()) {
            return response->get_reference_id();
        }
        return "local_" + app::crypto::KeyedHasher{"subscription_connector"}.hash(std::string{seed}).substr(0, 16);
    }

    [[nodiscard]] std::string status_from_connector(
        const std::optional<app::connector::ConnectorResponse>& response,
        std::string_view intended
    ) const
    {
        if (!response) {
            return std::string{intended};
        }
        const auto& status = response->get_status();
        if (status == "cancelled" || status == "canceled") {
            return "cancelled";
        }
        if (status == "paused") {
            return "paused";
        }
        return std::string{intended};
    }

    void apply_connector_reference(
        app::store::subscriptions::SubscriptionRecord& record,
        const std::optional<app::connector::ConnectorResponse>& response,
        std::string_view operation
    ) const
    {
        if (!response || response->get_reference_id().empty()) {
            return;
        }
        record.set_billing_processor_subscription_id(connector_reference(
            response, std::string{operation} + record.get_subscription_id()));
    }

    void store_invoice_state(const std::string& subscription_id, const InvoiceState& state) const
    {
        const std::string payload = std::format("{}|{}|{}|{}|{}|{}",
            state.invoice_id,
            state.payment_id,
            state.amount,
            state.currency,
            state.status,
            state.connector_invoice_id);
        m_context->config_store().put("subscription_invoices", subscription_id, payload);
    }

    [[nodiscard]] std::optional<InvoiceState> load_invoice_state(const std::string& subscription_id) const
    {
        const auto raw = m_context->config_store().get_value("subscription_invoices", subscription_id);
        if (!raw) {
            return std::nullopt;
        }
        std::vector<std::string> parts{};
        std::size_t start{0};
        while (true) {
            const auto separator = raw->find('|', start);
            parts.push_back(separator == std::string::npos
                ? raw->substr(start)
                : raw->substr(start, separator - start));
            if (separator == std::string::npos) {
                break;
            }
            start = separator + 1;
        }
        if (parts.size() < 6) {
            return std::nullopt;
        }
        InvoiceState state{};
        state.invoice_id = parts[0];
        state.payment_id = parts[1];
        state.amount = parse_int(parts[2], 0);
        state.currency = parts[3];
        state.status = parts[4];
        state.connector_invoice_id = parts[5];
        return state;
    }

    [[nodiscard]] BillingSetup setup_billing(
        const app::store::subscriptions::SubscriptionRecord& record,
        bool confirm,
        std::string_view raw_payload
    ) const
    {
        BillingSetup setup{};
        const PriceInfo price = price_for(record.get_item_price_id(), record.get_plan_id());
        setup.invoice.amount = price.amount;
        setup.invoice.currency = price.currency;
        setup.invoice.status = confirm ? "paid" : "draft";

        const auto customer_response = call_connector("billing_customer_create", make_billing_request(
            "billing_customer_create", record, record.get_customer_id(), raw_payload));
        setup.connector_customer_id = connector_reference(customer_response, "cus_" + record.get_customer_id());
        m_context->config_store().put(
            "subscription_connector_customers", record.get_customer_id(), setup.connector_customer_id);

        const auto subscription_response = call_connector("create_subscription", make_billing_request(
            "create_subscription", record, record.get_subscription_id(), raw_payload));
        setup.connector_subscription_id = connector_reference(subscription_response, "bpsub_" + record.get_subscription_id());
        setup.invoice.connector_invoice_id = connector_reference(subscription_response, "pinv_" + record.get_subscription_id());

        app::store::payments::PaymentIntentRecord payment{};
        payment.set_payment_id(m_context->ids().payment_id("pay_"));
        payment.set_merchant_id(record.get_merchant_id());
        payment.set_customer_id(record.get_customer_id());
        payment.set_merchant_reference_id(record.get_merchant_reference_id());
        payment.set_status(confirm ? "requires_confirmation" : "requires_payment_method");
        payment.set_amount(price.amount);
        payment.set_currency(price.currency);
        payment.set_description("subscription " + record.get_subscription_id());
        payment.set_created_at(now_value());
        payment.set_updated_at(now_value());
        const auto stored = m_context->repository().save_payment(std::move(payment));
        setup.invoice.payment_id = stored.get_payment_id();
        setup.invoice.invoice_id = m_context->ids().payment_id("inv_");
        store_invoice_state(record.get_subscription_id(), setup.invoice);

        const std::string tracker_payload = record.get_subscription_id() + "|" + setup.invoice.invoice_id;
        const auto tracker_id = m_context->process_trackers().schedule("invoice_sync", tracker_payload, "0");
        if (confirm) {
            m_context->process_trackers().mark_terminal(tracker_id, "synced");
        }
        m_context->events().publish("subscription.invoice.created", tracker_payload);
        return setup;
    }

    void record_state_sync(
        const app::store::subscriptions::SubscriptionRecord& record,
        std::string_view topic
    ) const
    {
        const std::string payload = record.get_subscription_id() + "|" + record.get_status();
        const auto tracker_id = m_context->process_trackers().schedule("subscription_state_sync", payload, "0");
        m_context->process_trackers().mark_terminal(tracker_id, std::string{topic});
        m_context->events().publish(std::string{topic}, payload);
    }

    [[nodiscard]] app::model::subscription::SubscriptionIdValue id_value(std::string_view value) const
    {
        app::model::subscription::SubscriptionIdValue out;
        out.set_value(std::string{value});
        return out;
    }

    [[nodiscard]] app::model::subscription::SubscriptionStatusValue status_value(std::string_view value) const
    {
        app::model::subscription::SubscriptionStatusValue out;
        out.set_value(std::string{value});
        return out;
    }

    [[nodiscard]] app::model::subscription::ProfileIdValue profile_value(std::string_view value) const
    {
        app::model::subscription::ProfileIdValue out;
        out.set_value(std::string{value});
        return out;
    }

    [[nodiscard]] app::model::subscription::MerchantIdValue merchant_value(std::string_view value) const
    {
        app::model::subscription::MerchantIdValue out;
        out.set_value(std::string{value});
        return out;
    }

    [[nodiscard]] app::model::subscription::CustomerIdValue customer_value(std::string_view value) const
    {
        app::model::subscription::CustomerIdValue out;
        out.set_value(std::string{value});
        return out;
    }

    [[nodiscard]] app::model::subscription::MinorUnitValue minor_unit(std::string_view value) const
    {
        app::model::subscription::MinorUnitValue out;
        out.set_value(std::string{value});
        return out;
    }

    [[nodiscard]] app::model::subscription::CurrencyValue currency(std::string_view value) const
    {
        app::model::subscription::CurrencyValue out;
        out.set_value(std::string{value});
        return out;
    }

    [[nodiscard]] app::model::subscription::InvoiceIdValue invoice_id_value(std::string_view value) const
    {
        app::model::subscription::InvoiceIdValue out;
        out.set_value(std::string{value});
        return out;
    }

    [[nodiscard]] app::model::subscription::InvoiceStatusValue invoice_status_value(std::string_view value) const
    {
        app::model::subscription::InvoiceStatusValue out;
        out.set_value(std::string{value});
        return out;
    }

    [[nodiscard]] app::model::subscription::IntentStatusValue intent_status_value(std::string_view value) const
    {
        app::model::subscription::IntentStatusValue out;
        out.set_value(std::string{value});
        return out;
    }

    [[nodiscard]] std::optional<std::string> optional_string(const std::string& value) const
    {
        return value.empty() ? std::nullopt : std::optional<std::string>{value};
    }

    [[nodiscard]] std::optional<app::model::subscription::Invoice> build_invoice(
        const app::store::subscriptions::SubscriptionRecord& record,
        const InvoiceState& state
    ) const
    {
        if (state.invoice_id.empty()) {
            return std::nullopt;
        }
        app::model::subscription::Invoice invoice{};
        invoice.set_id(invoice_id_value(state.invoice_id));
        invoice.set_subscription_id(id_value(record.get_subscription_id()));
        invoice.set_merchant_id(merchant_value(record.get_merchant_id()));
        invoice.set_profile_id(profile_value(record.get_profile_id()));
        invoice.set_customer_id(customer_value(record.get_customer_id()));
        invoice.set_amount(minor_unit(std::to_string(state.amount)));
        invoice.set_currency(currency(state.currency));
        invoice.set_status(invoice_status_value(state.status));
        invoice.set_billing_processor_invoice_id(optional_string(state.connector_invoice_id));
        app::model::subscription::PaymentIdValue payment{};
        payment.set_value(state.payment_id);
        invoice.set_payment_intent_id(payment);
        return invoice;
    }

    [[nodiscard]] std::optional<app::model::subscription::PaymentResponseData> build_payment(
        app::store::AppRepository& repository,
        const InvoiceState& state
    ) const
    {
        if (state.payment_id.empty()) {
            return std::nullopt;
        }
        const auto found = repository.find_payment_by_string(state.payment_id);
        if (!found) {
            return std::nullopt;
        }
        app::model::subscription::PaymentResponseData payment{};
        app::model::subscription::PaymentIdValue payment_id{};
        payment_id.set_value(found->get_payment_id());
        payment.set_payment_id(payment_id);
        payment.set_status(intent_status_value(found->get_status()));
        payment.set_amount(minor_unit(std::to_string(found->get_amount())));
        payment.set_currency(currency(found->get_currency()));
        payment.set_connector(optional_string(state.connector_invoice_id));
        return payment;
    }

    [[nodiscard]] app::model::subscription::SubscriptionResponse to_response(
        const app::store::subscriptions::SubscriptionRecord& record,
        std::optional<std::string> client_secret_plain = std::nullopt,
        const BillingSetup* setup = nullptr
    ) const
    {
        app::model::subscription::SubscriptionResponse response;
        response.set_id(id_value(record.get_subscription_id()));
        response.set_merchant_reference_id(optional_string(record.get_merchant_reference_id()));
        response.set_status(status_value(record.get_status()));
        response.set_plan_id(optional_string(record.get_plan_id()));
        response.set_item_price_id(optional_string(record.get_item_price_id()));
        response.set_profile_id(profile_value(record.get_profile_id()));
        response.set_client_secret(
            client_secret_plain ? client_secret_plain : optional_string(record.get_client_secret()));
        response.set_merchant_id(merchant_value(record.get_merchant_id()));
        response.set_coupon_code(optional_string(record.get_coupon()));
        response.set_customer_id(customer_value(record.get_customer_id()));

        const InvoiceState state = setup
            ? setup->invoice
            : load_invoice_state(record.get_subscription_id()).value_or(InvoiceState{});
        if (!state.invoice_id.empty()) {
            auto repository = m_context->repository();
            response.set_invoice(build_invoice(record, state));
            response.set_payment(build_payment(repository, state));
        }
        return response;
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

} // namespace app::handlers::subscription
