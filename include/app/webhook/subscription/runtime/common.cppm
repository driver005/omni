export module app_webhook:subscription_runtime_common;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;

import :subscription_input;

export namespace app::handlers::subscription {

// Json / JsonObject aliases come from :input (imported above).

// ── Local value objects (mirror the monolith's private structs) ────────────────
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

/**
 * @brief Shared business helpers for the subscription sub-runtimes.
 *
 * Errors are core::error::AppError: real text in message, upstream code in kind_name,
 * kind selects the HTTP status. Success responses are typed serde::Value documents
 * built here; the handler serializes them with Ser::serialize.
 */
class SubscriptionRuntimeSupport
{
public:
    explicit SubscriptionRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ── Error builders ──────────────────────────────────────────────────────────
    [[nodiscard]] core::error::AppError make_failure(
        core::error::AppErrorKind kind,
        std::string_view /*type*/,
        std::string_view code,
        std::string_view message
    ) const
    {
        auto error = core::error::AppErrorBuilder{}(kind, std::string{message});
        error.set_kind_name(std::string{code});
        return error;
    }

    [[nodiscard]] core::error::AppError missing_field(std::string_view name) const
    {
        return make_failure(
            core::error::AppErrorKind::INVALID_REQUEST,
            "invalid_request",
            "IR_04",
            std::format("Missing required param: {}", name)
        );
    }

    [[nodiscard]] core::error::AppError invalid_request_data(std::string_view message) const
    {
        return make_failure(
            core::error::AppErrorKind::VALIDATION_FAILED,
            "invalid_request",
            "IR_06",
            message
        );
    }

    [[nodiscard]] core::error::AppError generic_not_found(std::string_view message) const
    {
        return make_failure(
            core::error::AppErrorKind::NOT_FOUND,
            "invalid_request",
            "IR_37",
            message
        );
    }

    [[nodiscard]] core::error::AppError profile_not_found(std::string_view id) const
    {
        return make_failure(
            core::error::AppErrorKind::NOT_FOUND,
            "invalid_request",
            "HE_02",
            std::format("Business profile with the given id {} does not exist", id)
        );
    }

    [[nodiscard]] core::error::AppError customer_not_found() const
    {
        return make_failure(
            core::error::AppErrorKind::NOT_FOUND,
            "invalid_request",
            "HE_02",
            "Customer does not exist in our records"
        );
    }

    [[nodiscard]] core::error::AppError subscription_error(std::string_view operation) const
    {
        return make_failure(
            core::error::AppErrorKind::INVALID_REQUEST,
            "invalid_request",
            "CE_09",
            std::format("Subscription operation: {} failed with connector", operation)
        );
    }

    [[nodiscard]] core::error::AppError internal_error() const
    {
        return make_failure(
            core::error::AppErrorKind::INTERNAL,
            "api",
            "HE_00",
            "Something went wrong"
        );
    }

    // ── Value helpers ───────────────────────────────────────────────────────────
    [[nodiscard]] Json optional_document(const std::string& value) const
    {
        return value.empty() ? Json{} : Json{value};
    }

    [[nodiscard]] std::optional<std::string>
    json_string(const JsonObject* object, std::string_view key) const
    {
        if (object == nullptr) {
            return std::nullopt;
        }
        const auto field = object->get(std::string{key});
        if (!field) {
            return std::nullopt;
        }
        if (const auto* text = std::get_if<std::string>(&field.value().get())) {
            return *text;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::string>
    json_string(const Json& value, std::string_view key) const
    {
        return json_string(std::get_if<JsonObject>(&value.get()), key);
    }

    [[nodiscard]] std::string now_value() const
    {
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
                                 std::chrono::system_clock::now().time_since_epoch()
        )
                                 .count();
        return std::to_string(seconds);
    }

    [[nodiscard]] std::int64_t parse_int(std::string_view raw, std::int64_t fallback) const
    {
        std::int64_t value{};
        const auto converted = std::from_chars(raw.data(), raw.data() + raw.size(), value);
        return converted.ec == std::errc{} && converted.ptr == raw.data() + raw.size() ? value
                                                                                       : fallback;
    }

    // ── SubscriptionHandler helpers (subscription_handler.rs) ───────────────────
    [[nodiscard]] std::optional<core::error::AppError>
    find_business_profile(std::string_view profile_id, std::string_view merchant_id_value) const
    {
        (void)merchant_id_value;
        if (!m_context.repository().find_profile_by_string(profile_id)) {
            return profile_not_found(profile_id);
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    find_customer(std::string_view customer_id, std::string_view merchant_id_value) const
    {
        const auto customer = m_context.repository().find_customer_by_string(customer_id);
        if (!customer ||
            (!merchant_id_value.empty() && customer->get_merchant_id() != merchant_id_value)) {
            return customer_not_found();
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string generate_client_secret(std::string_view subscription_id) const
    {
        // Subscription::generate_and_set_client_secret (subscription.rs:110-115):
        // generate_id_with_default_len("{id}_secret") -> random suffix.
        const ::app::crypto::KeyedHasher hasher{"subscription_client_secret"};
        const std::string digest = hasher.hash(std::string{subscription_id} + "_secret");
        return std::string{subscription_id} + "_secret" + digest.substr(0, 16);
    }

    [[nodiscard]] std::optional<std::string>
    client_secret_subscription_id(std::string_view secret) const
    {
        // ClientSecret::get_subscription_id (subscription.rs:24-35)
        const auto split = secret.find("_secret");
        if (split == std::string_view::npos) {
            return std::nullopt;
        }
        return std::string{secret.substr(0, split)};
    }

    [[nodiscard]] std::optional<core::error::AppError>
    validate_client_secret(std::string_view secret, std::string_view merchant_id_value) const
    {
        // find_and_validate_subscription (subscription_handler.rs:189-243)
        const auto subscription_id = client_secret_subscription_id(secret);
        if (!subscription_id) {
            return missing_field("client_secret");
        }
        auto record = find_subscription(*subscription_id);
        if (!record) {
            return generic_not_found("Subscription not found for id: " + *subscription_id);
        }
        if (!merchant_id_value.empty() && record->merchant_id != merchant_id_value) {
            return generic_not_found("Subscription not found for id: " + *subscription_id);
        }
        if (record->client_secret.empty()) {
            return missing_field("client_secret");
        }
        if (secret != record->client_secret) {
            return invalid_request_data(
                "The client_secret provided does not match the client_secret associated with the "
                "Payment"
            );
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
        auto repository = m_context.repository();
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

    [[nodiscard]] ::app::store::subscriptions::SubscriptionRecord
    to_store_record(const SubscriptionRecord& record) const
    {
        ::app::store::subscriptions::SubscriptionRecord out{};
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

    [[nodiscard]] SubscriptionRecord
    from_store_record(const ::app::store::subscriptions::SubscriptionRecord& record) const
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

    // Payment / invoice helpers -------------------------------------------------
    [[nodiscard]] Estimate price_for(std::string_view item_price_id, std::string_view plan_id) const
    {
        // Local in-process substitute for the billing processor price catalogue.
        Estimate out{};
        const std::string key =
            item_price_id.empty() ? std::string{plan_id} : std::string{item_price_id};
        if (!key.empty()) {
            if (const auto configured =
                    m_context.config_store().get_value("subscription_prices", key)) {
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
            const std::string digest = ::app::crypto::KeyedHasher{"subscription_price"}.hash(seed);
            std::uint64_t numeric{};
            const auto width = std::min<std::size_t>(8, digest.size());
            std::from_chars(digest.data(), digest.data() + width, numeric, 16);
            out.amount = 100 + static_cast<std::int64_t>(numeric % 100'000);
        }
        if (out.currency.empty()) {
            out.currency = "USD";
        }
        return out;
    }

    [[nodiscard]] Estimate estimate_from_connector(
        const ConnectorResponse& response,
        std::string_view plan_id,
        std::string_view item_price_id
    ) const
    {
        // Local connector is in-process; it echoes the amount/currency we requested.
        Estimate out = price_for(item_price_id, plan_id);
        out.plan_id = std::string{plan_id};
        out.item_price_id = std::string{item_price_id};
        if (!response.reference_id.empty()) {
            out.description = response.reference_id;
        }
        return out;
    }

    [[nodiscard]] ::app::connector::ConnectorRequest billing_request(
        std::string_view operation,
        const SubscriptionRecord& record,
        std::string_view resource_id
    ) const
    {
        return ::app::connector::ConnectorRequestBuilder{}
            .operation(std::string{operation})
            .merchant_id(record.merchant_id)
            .resource_id(std::string{resource_id})();
    }

    [[nodiscard]] std::optional<::app::connector::ConnectorResponse> call_connector(
        std::string_view operation,
        const ::app::connector::ConnectorRequest& request
    ) const
    {
        auto connector = m_context.connector_router().select(request.get_merchant_id());
        if (!connector) {
            return std::nullopt;
        }
        m_context.metrics().record_flow(std::format("subscription.connector.{}", operation));
        if (operation == "billing_customer_create" || operation == "create_customer_on_connector") {
            return connector->create_link(request);
        }
        if (operation == "get_subscription_estimate") {
            return connector->verify(request);
        }
        if (operation == "cancel_subscription") {
            return connector->cancel(request);
        }
        if (operation == "create_subscription_on_connector" ||
            operation == "confirm_subscription") {
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
        const auto request = ::app::connector::ConnectorRequestBuilder{}
                                 .operation(std::string{operation})
                                 .merchant_id(std::string{merchant})
                                 .resource_id(record.subscription_id)();
        const auto response = call_connector(operation, request);
        (void)payload;
        if (response) {
            out.status = response->get_status();
            out.reference_id = response->get_reference_id();
        }
        return out;
    }

    [[nodiscard]] std::string
    connector_reference(const ConnectorResponse& response, std::string_view seed) const
    {
        if (!response.reference_id.empty()) {
            return response.reference_id;
        }
        return "local_" + ::app::crypto::KeyedHasher{"subscription_connector"}
                              .hash(std::string{seed})
                              .substr(0, 16);
    }

    [[nodiscard]] std::string subscription_status_from_connector(
        const ConnectorResponse& response,
        std::string_view intended
    ) const
    {
        // SubscriptionStatus::from(connector_response.status) -> common_enums snake_case.
        const auto& status = response.status;
        if (status == "cancelled" || status == "canceled" || status == "cancel") {
            return "cancelled";
        }
        if (status == "paused") {
            return "paused";
        }
        if (status == "active" || status == "authorized" || status == "succeeded") {
            return "active";
        }
        if (status == "created" || status == "pending") {
            return "created";
        }
        return std::string{intended};
    }

    void create_connector_customer(const SubscriptionRecord& record, const Json& payload) const
    {
        (void)payload;
        // create_customer_on_connector (billing_processor_handler.rs:103-170):
        // Skip when the customer already has a connector-customer id for this MCA.
        const auto key = record.customer_id;
        if (m_context.config_store().get_value("subscription_connector_customers", key)) {
            return;
        }
        const auto response = call_connector(
            "billing_customer_create",
            billing_request("billing_customer_create", record, record.customer_id)
        );
        ConnectorResponse converted{};
        if (response) {
            converted.status = response->get_status();
            converted.reference_id = response->get_reference_id();
        }
        const std::string customer_reference =
            connector_reference(converted, "cus_" + record.customer_id);
        // update_connector_customer_id_in_customer (subscription_handler.rs:109-138)
        m_context.config_store().put("subscription_connector_customers", key, customer_reference);
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
        ::app::store::payments::PaymentIntentRecord payment{};
        payment.set_payment_id(m_context.ids().payment_id("pay_"));
        payment.set_merchant_id(record.merchant_id);
        payment.set_customer_id(record.customer_id);
        payment.set_merchant_reference_id(record.merchant_reference_id);
        payment.set_status(confirm ? "succeeded" : "requires_confirmation");
        payment.set_amount(estimate.amount);
        payment.set_currency(estimate.currency);
        payment.set_description("subscription " + record.subscription_id);
        payment.set_created_at(now_value());
        payment.set_updated_at(now_value());
        const auto stored = m_context.repository().save_payment(std::move(payment));
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
        auto payment = m_context.repository().find_payment_by_string(payment_id);
        if (payment) {
            payment->set_status("succeeded");
            payment->set_updated_at(now_value());
            m_context.repository().update_payment(*payment);
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
        auto payment = m_context.repository().find_payment_by_string(payment_id);
        if (payment) {
            payment->set_amount(estimate.amount);
            payment->set_currency(estimate.currency);
            payment->set_updated_at(now_value());
            m_context.repository().update_payment(*payment);
        }
    }

    [[nodiscard]] InvoiceView create_invoice(
        const SubscriptionRecord& record,
        std::string_view merchant_connector_id,
        const Estimate& estimate,
        std::string_view payment_id,
        std::string_view status,
        std::string_view connector_invoice_id
    ) const
    {
        InvoiceView invoice{};
        invoice.invoice_id = m_context.ids().payment_id("inv_");
        invoice.subscription_id = record.subscription_id;
        invoice.merchant_id = record.merchant_id;
        invoice.profile_id = record.profile_id;
        invoice.merchant_connector_id = std::string{merchant_connector_id};
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
        const auto payload = std::format(
            "{}|{}|{}|{}|{}|{}|{}|{}|{}|{}|{}|{}",
            invoice.invoice_id,
            invoice.subscription_id,
            invoice.merchant_id,
            invoice.profile_id,
            invoice.merchant_connector_id,
            invoice.payment_intent_id,
            invoice.payment_method_id,
            invoice.customer_id,
            invoice.amount,
            invoice.currency,
            invoice.status,
            invoice.connector_invoice_id
        );
        m_context.config_store().put("subscription_invoices", invoice.subscription_id, payload);
    }

    [[nodiscard]] InvoiceView latest_invoice(std::string_view subscription_id) const
    {
        InvoiceView out{};
        const auto raw =
            m_context.config_store().get_value("subscription_invoices", subscription_id);
        if (!raw) {
            return out;
        }
        std::vector<std::string> parts{};
        std::size_t start{0};
        while (true) {
            const auto separator = raw->find('|', start);
            parts.push_back(
                separator == std::string::npos ? raw->substr(start)
                                               : raw->substr(start, separator - start)
            );
            if (separator == std::string::npos) {
                break;
            }
            start = separator + 1;
        }
        if (parts.size() < 12) {
            return out;
        }
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
        const std::string payload = std::format(
            "{}|{}|{}|{}|{}",
            record.subscription_id,
            invoice.invoice_id,
            record.merchant_id,
            record.profile_id,
            record.customer_id
        );
        const auto tracker_id = m_context.process_trackers().schedule("invoice_sync", payload, "0");
        m_context.process_trackers().mark_terminal(tracker_id, "synced");
        m_context.events().publish("subscription.invoice.created", payload);
    }

    [[nodiscard]] Estimate estimate_for(const SubscriptionRecord& record) const
    {
        return price_for(record.item_price_id, record.plan_id);
    }

    void metrics_for(std::string_view name) const
    {
        m_context.metrics().record_flow(name);
        m_context.metrics().counter(name).increment();
    }

    // ── Response document builders (exact upstream field order / Option handling) ──
    [[nodiscard]] Json build_subscription_response(
        const SubscriptionRecord& record,
        std::optional<std::string> client_secret,
        std::optional<PaymentView> payment,
        std::optional<InvoiceView> invoice
    ) const
    {
        // SubscriptionResponse (api_models/subscription.rs:59-97):
        // id, merchant_reference_id, status, plan_id, item_price_id, profile_id,
        // client_secret, merchant_id, coupon_code, customer_id, payment, invoice
        JsonObject out{};
        out["id"] = Json{record.subscription_id};
        out["merchant_reference_id"] = optional_document(record.merchant_reference_id);
        out["status"] = Json{record.status};
        out["plan_id"] = optional_document(record.plan_id);
        out["item_price_id"] = optional_document(record.item_price_id);
        out["profile_id"] = Json{record.profile_id};
        out["client_secret"] = optional_document(client_secret.value_or(record.client_secret));
        out["merchant_id"] = Json{record.merchant_id};
        out["coupon_code"] = optional_document(record.coupon);
        out["customer_id"] = Json{record.customer_id};
        out["payment"] = payment ? build_payment(*payment) : Json{};
        out["invoice"] = invoice ? build_invoice(*invoice) : Json{};
        return Json{std::move(out)};
    }

    [[nodiscard]] Json build_confirm_response(
        const SubscriptionRecord& record,
        const InvoiceView& invoice,
        const PaymentView& payment
    ) const
    {
        // ConfirmSubscriptionResponse (api_models/subscription.rs:456-490):
        // id, merchant_reference_id, status, plan_id, item_price_id, coupon,
        // profile_id, payment, customer_id, invoice, billing_processor_subscription_id
        JsonObject out{};
        out["id"] = Json{record.subscription_id};
        out["merchant_reference_id"] = optional_document(record.merchant_reference_id);
        out["status"] = Json{record.status};
        out["plan_id"] = optional_document(record.plan_id);
        out["item_price_id"] = optional_document(record.item_price_id);
        out["coupon"] = optional_document(record.coupon);
        out["profile_id"] = Json{record.profile_id};
        out["payment"] = build_payment(payment);
        out["customer_id"] = Json{record.customer_id};
        out["invoice"] = build_invoice(invoice);
        out["billing_processor_subscription_id"] =
            optional_document(record.connector_subscription_id);
        return Json{std::move(out)};
    }

    [[nodiscard]] Json build_invoice(const InvoiceView& invoice) const
    {
        // Invoice (api_models/subscription.rs:504-542)
        JsonObject out{};
        out["id"] = Json{invoice.invoice_id};
        out["subscription_id"] = Json{invoice.subscription_id};
        out["merchant_id"] = Json{invoice.merchant_id};
        out["profile_id"] = Json{invoice.profile_id};
        out["merchant_connector_id"] = Json{invoice.merchant_connector_id};
        out["payment_intent_id"] = optional_document(invoice.payment_intent_id);
        out["payment_method_id"] = optional_document(invoice.payment_method_id);
        out["customer_id"] = Json{invoice.customer_id};
        out["amount"] = Json{invoice.amount};
        out["currency"] = Json{invoice.currency};
        out["status"] = Json{invoice.status};
        out["billing_processor_invoice_id"] = optional_document(invoice.connector_invoice_id);
        return Json{std::move(out)};
    }

    [[nodiscard]] Json build_payment(const PaymentView& payment) const
    {
        // PaymentResponseData (api_models/subscription.rs:340-367) — Option fields
        // upstream emits as null unless explicitly skipped.
        JsonObject out{};
        out["payment_id"] = Json{payment.payment_id};
        out["status"] = Json{payment.status};
        out["amount"] = Json{payment.amount};
        out["currency"] = Json{payment.currency};
        out["profile_id"] = Json{};
        out["connector"] = optional_document(payment.connector);
        out["payment_method_id"] = Json{};
        out["return_url"] = Json{};
        out["next_action"] = Json{};
        out["payment_experience"] = Json{};
        out["error_code"] = Json{};
        out["error_message"] = Json{};
        out["payment_method_type"] = Json{};
        out["client_secret"] = Json{};
        out["billing"] = Json{};
        out["shipping"] = Json{};
        out["payment_type"] = Json{};
        out["payment_token"] = Json{};
        return Json{std::move(out)};
    }

    [[nodiscard]] Json
    build_estimate_response(const Estimate& estimate, std::string_view /*profile*/) const
    {
        // EstimateSubscriptionResponse (api_models/subscription.rs:585-600).
        // From<GetSubscriptionEstimateResponse> (router_response_types/subscriptions.rs:151-168)
        // explicitly sets plan_id/item_price_id/coupon_code = None.
        JsonObject out{};
        out["amount"] = Json{estimate.amount};
        out["currency"] = Json{estimate.currency};
        out["plan_id"] = Json{};
        out["item_price_id"] = Json{};
        out["coupon_code"] = Json{};
        out["customer_id"] = optional_document(estimate.customer_id);
        JsonObject line{};
        line["item_id"] =
            Json{estimate.item_price_id.empty() ? estimate.plan_id : estimate.item_price_id};
        line["item_type"] = Json{std::string{"plan"}};
        line["description"] = Json{estimate.description};
        line["amount"] = Json{estimate.amount};
        line["currency"] = Json{estimate.currency};
        line["quantity"] = Json{std::int64_t{1}};
        Json::Array line_items{};
        line_items.push_back(Json{std::move(line)});
        out["line_items"] = Json{std::move(line_items)};
        return Json{std::move(out)};
    }

    [[nodiscard]] Json build_items_response(const std::vector<ItemView>& items) const
    {
        Json::Array out{};
        for (const auto& item: items) {
            JsonObject entry{};
            entry["item_id"] = Json{item.item_id};
            entry["name"] = Json{item.name};
            entry["description"] = optional_document(item.description);
            Json::Array prices{};
            for (const auto& price: item.prices) {
                JsonObject p{};
                p["price_id"] = Json{price.price_id};
                p["item_id"] = optional_document(price.item_id);
                p["amount"] = Json{price.amount};
                p["currency"] = Json{price.currency};
                p["interval"] = Json{price.interval};
                p["interval_count"] = Json{price.interval_count};
                p["trial_period"] = Json{};
                p["trial_period_unit"] = Json{};
                prices.push_back(Json{std::move(p)});
            }
            entry["price_id"] = Json{std::move(prices)};
            out.push_back(Json{std::move(entry)});
        }
        return Json{std::move(out)};
    }

    [[nodiscard]] Json
    build_pause_response(const SubscriptionRecord& record, std::string_view paused_at) const
    {
        // PauseSubscriptionResponse (api_models/subscription.rs:642-659)
        JsonObject out{};
        out["id"] = Json{record.subscription_id};
        out["status"] = Json{record.status};
        out["merchant_reference_id"] = optional_document(record.merchant_reference_id);
        out["profile_id"] = Json{record.profile_id};
        out["merchant_id"] = Json{record.merchant_id};
        out["customer_id"] = Json{record.customer_id};
        out["paused_at"] = Json{std::string{paused_at}};
        return Json{std::move(out)};
    }

    [[nodiscard]] Json
    build_resume_response(const SubscriptionRecord& record, std::string_view next_billing_at) const
    {
        // ResumeSubscriptionResponse (api_models/subscription.rs:698-716)
        JsonObject out{};
        out["id"] = Json{record.subscription_id};
        out["status"] = Json{record.status};
        out["merchant_reference_id"] = optional_document(record.merchant_reference_id);
        out["profile_id"] = Json{record.profile_id};
        out["merchant_id"] = Json{record.merchant_id};
        out["customer_id"] = Json{record.customer_id};
        out["next_billing_at"] = Json{std::string{next_billing_at}};
        return Json{std::move(out)};
    }

    [[nodiscard]] Json
    build_cancel_response(const SubscriptionRecord& record, std::string_view cancelled_at) const
    {
        // CancelSubscriptionResponse (api_models/subscription.rs:776-794)
        JsonObject out{};
        out["id"] = Json{record.subscription_id};
        out["status"] = Json{record.status};
        out["merchant_reference_id"] = optional_document(record.merchant_reference_id);
        out["profile_id"] = Json{record.profile_id};
        out["merchant_id"] = Json{record.merchant_id};
        out["customer_id"] = Json{record.customer_id};
        out["cancelled_at"] = Json{std::string{cancelled_at}};
        return Json{std::move(out)};
    }

private:
    ::app::context::AppContext& m_context;
};

// ── BillingHandler (billing_processor_handler.rs) ──────────────────────────────
class BillingProcessor
{
public:
    std::string merchant_connector_id{};
    std::string connector_name{"local"};
    std::string profile_id{};
    std::string merchant_id{};
    bool ready{false};

    [[nodiscard]] std::optional<core::error::AppError>
    create(std::string_view profile, std::string_view merchant)
    {
        // BillingHandler::create (billing_processor_handler.rs:40-101):
        // profile.get_billing_processor_id() -> MCA lookup by merchant + id.
        const auto record = m_context.repository().find_profile_by_string(profile);
        if (!record) {
            return m_support.profile_not_found(profile);
        }
        // Local substitute for Profile::get_billing_processor_id(): the configured
        // billing processor MCA id for the profile (upstream reads it from the profile).
        if (const auto configured = m_context.config_store().get_value(
                "subscription_billing_processors",
                std::string{profile}
            )) {
            merchant_connector_id = *configured;
        }
        if (merchant_connector_id.empty()) {
            for (const auto& mca: m_context.repository().list_merchant_connectors()) {
                if (merchant.empty() || mca.get_merchant_id() == merchant) {
                    merchant_connector_id = mca.get_merchant_connector_id();
                    connector_name = mca.get_connector_name();
                    break;
                }
            }
        } else {
            if (const auto mca = m_context.repository().find_merchant_connector_by_string(
                    merchant_connector_id
                )) {
                connector_name = mca->get_connector_name();
            }
        }
        if (merchant_connector_id.empty()) {
            return m_support.generic_not_found(
                "Merchant connector account does not exist in our records"
            );
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
        ::app::connector::ConnectorRequest request = m_support.billing_request(
            "get_subscription_estimate",
            record,
            item_price_id.empty() ? std::string{plan_id} : std::string{item_price_id}
        );
        const auto raw = m_support.call_connector("get_subscription_estimate", request);
        ConnectorResponse response{};
        if (raw) {
            response.status = raw->get_status();
            response.reference_id = raw->get_reference_id();
        }
        Estimate out = m_support.estimate_from_connector(response, plan_id, item_price_id);
        out.description = std::string{coupon_code};
        if (!client_secret.empty()) {
            out.customer_id = record.customer_id;
        }
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
        const auto response = m_support.call_connector(
            "get_subscription_items",
            m_support.billing_request("get_subscription_items", seed, std::string{item_type})
        );
        (void)response;

        auto records = m_context.repository().list_subscriptions();
        std::vector<SubscriptionRecord> scoped{};
        for (const auto& stored: records) {
            if (!profile.empty() && stored.get_profile_id() != profile) {
                continue;
            }
            scoped.push_back(m_support.from_store_record(stored));
        }
        const std::size_t begin = std::min(static_cast<std::size_t>(offset), scoped.size());
        const std::size_t stop = std::min(begin + static_cast<std::size_t>(limit), scoped.size());
        for (std::size_t i = begin; i < stop; ++i) {
            const auto& record = scoped[i];
            ItemView item{};
            item.item_id = item_type == "addon" ? record.item_price_id : record.subscription_id;
            item.name =
                item_type == "addon"
                    ? (record.item_price_id.empty() ? std::string{"subscription addon"}
                                                    : record.item_price_id)
                    : (record.plan_id.empty() ? std::string{"subscription item"} : record.plan_id);
            item.description = record.coupon;
            ItemPriceView price{};
            price.price_id = record.item_price_id;
            price.item_id = record.subscription_id;
            const Estimate estimate = m_support.price_for(record.item_price_id, record.plan_id);
            price.amount = estimate.amount;
            price.currency = estimate.currency;
            price.interval = "month";
            price.interval_count = 1;
            item.prices.push_back(std::move(price));
            items.push_back(std::move(item));
        }
        return items;
    }

    explicit BillingProcessor(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

private:
    ::app::context::AppContext& m_context;
    SubscriptionRuntimeSupport m_support;
};

} // namespace app::handlers::subscription
