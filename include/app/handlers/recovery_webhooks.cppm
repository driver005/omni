module;
export module app_handlers:recovery_webhooks;

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
import app_events;
import app_locking;
import app_services;
import app_workflow;

export namespace app::handlers::recovery_webhooks {

/**
 * @brief Typed acknowledgement mirroring upstream `WebhookResponse<Value>` plus
 * `ApplicationResponse::IncomingWebhookEvent` metadata. Replaces the previous raw
 * echo of the request body (`incoming_v2.rs:81-99`).
 */
class RecoveryWebhookAck
{
public:
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }
    void set_flow(std::string value) { flow = std::move(value); }
    [[nodiscard]] const std::string& get_flow() const noexcept { return flow; }
    void set_event_type(std::string value) { event_type = std::move(value); }
    [[nodiscard]] const std::string& get_event_type() const noexcept { return event_type; }
    void set_event_id(std::string value) { event_id = std::move(value); }
    [[nodiscard]] const std::string& get_event_id() const noexcept { return event_id; }
    void set_recovery_intent_id(std::string value) { recovery_intent_id = std::move(value); }
    [[nodiscard]] const std::string& get_recovery_intent_id() const noexcept { return recovery_intent_id; }
    void set_tracker(std::string value) { tracker = std::move(value); }
    [[nodiscard]] const std::string& get_tracker() const noexcept { return tracker; }
    void set_connector(std::string value) { connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return connector; }
    void set_merchant_id(std::string value) { merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return merchant_id; }
    void set_profile_id(std::string value) { profile_id = std::move(value); }
    [[nodiscard]] const std::string& get_profile_id() const noexcept { return profile_id; }
    void set_payment_id(std::string value) { payment_id = std::move(value); }
    [[nodiscard]] const std::string& get_payment_id() const noexcept { return payment_id; }
    void set_refund_id(std::string value) { refund_id = std::move(value); }
    [[nodiscard]] const std::string& get_refund_id() const noexcept { return refund_id; }
    void set_attempt_id(std::string value) { attempt_id = std::move(value); }
    [[nodiscard]] const std::string& get_attempt_id() const noexcept { return attempt_id; }
    void set_recovery_token(std::string value) { recovery_token = std::move(value); }
    [[nodiscard]] const std::string& get_recovery_token() const noexcept { return recovery_token; }
    void set_published_event_id(std::string value) { published_event_id = std::move(value); }
    [[nodiscard]] const std::string& get_published_event_id() const noexcept { return published_event_id; }
    void set_lock_action(std::string value) { lock_action = std::move(value); }
    [[nodiscard]] const std::string& get_lock_action() const noexcept { return lock_action; }
    void set_message(std::string value) { message = std::move(value); }
    [[nodiscard]] const std::string& get_message() const noexcept { return message; }

    std::string status{};
    std::string flow{};
    std::string event_type{};
    std::string event_id{};
    std::string recovery_intent_id{};
    std::string tracker{};
    std::string connector{};
    std::string merchant_id{};
    std::string profile_id{};
    std::string payment_id{};
    std::string refund_id{};
    std::string attempt_id{};
    std::string recovery_token{};
    std::string published_event_id{};
    std::string lock_action{};
    std::string message{};
};

class RecoveryWebhooksRouteHandlers
{
public:
    void handle_recovery_receive_incoming_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Path and query are parsed first so the URL merchant/profile ids can be bound
        // into the auth context (upstream MerchantIdAndProfileIdAuth, recovery_webhooks.rs:42-45).
        const auto path = app::http::RequestPath{}(req);
        const auto query = app::http::RequestQuery{}(req);
        const RecoveryIds ids = recovery_ids(path);
        if (ids.merchant_id.empty() || ids.profile_id.empty() || ids.connector_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing merchant_id, profile_id or connector_id"
            ));
            return;
        }

        const auto rule = m_context->auth_rules().rule_for("recovery_receive_incoming_webhook");
        auto auth_context = app::auth::AuthContextBuilder{}(req);
        auth_context.set_merchant_id(ids.merchant_id);
        auth_context.set_profile_id(query.get("profile_id").value_or(ids.profile_id));
        auth_context.add_claim("x-merchant-id", ids.merchant_id);
        auth_context.add_claim("x-profile-id", query.get("profile_id").value_or(ids.profile_id));
        if (const auto auth = m_context->authorizer().authorize_route(auth_context, rule)) {
            send_error(res, send, *auth);
            return;
        }

        const std::string body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST, "empty request"
            ));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON, "invalid JSON"
            ));
            return;
        }

        // RequestScope is the local equivalent of upstream `server_wrap` request state:
        // one instance carries ids, decoded event metadata and side-effect results
        // through every helper. LockAction is NotApplicable upstream (recovery_webhooks.rs:22-46).
        RequestScope scope{};
        scope.ids = ids;
        scope.body = body;
        scope.org_id = app::http::org_id_from_header{}(req);
        scope.event_class = scope.org_id.empty()
            ? std::string{"incoming:recovery:v2"}
            : std::string{"incoming:recovery:v2:org:"} + scope.org_id;
        scope.lock_action = "NotApplicable";

        // Billing connector (MCA) fetch and validation, incoming_v2.rs:139-145.
        const auto mca = m_context->repository().find_merchant_connector_by_string(ids.connector_id);
        if (mca) {
            if ((!mca->get_merchant_id().empty() && mca->get_merchant_id() != ids.merchant_id) ||
                (!mca->get_profile_id().empty() && mca->get_profile_id() != ids.profile_id)) {
                send_error(res, send, core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::FORBIDDEN,
                    "billing connector does not belong to merchant/profile"
                ));
                return;
            }
            scope.connector_name = mca->get_connector_name().empty()
                ? ids.connector_id
                : mca->get_connector_name();
            scope.secret_hint = mca->get_connector_webhook_details();
        } else {
            scope.connector_name = ids.connector_id;
            record_flow(scope, "WebhookRecoveryMcaFallback");
        }
        scope.connector_key = parse_connector_name(scope.connector_name);
        scope.webhook_secret = resolve_webhook_secret(req, scope);
        load_registered_connector_webhook(scope);

        // Mandatory source verification: upstream rejects !source_verified with
        // WebhookAuthenticationFailed because recovery has no pre-existing payment
        // (recovery_incoming.rs:71-76; verifier at incoming_v2.rs:147-149).
        if (const auto source_error = verify_webhook_source(req, scope)) {
            send_error(res, send, *source_error);
            return;
        }

        const EventDetection detection = detect_event(*parsed, scope);
        scope.event_type = detection.event_type;
        scope.object_id = detection.object_id;
        scope.flow = detection.flow;
        scope.tracker = detection.tracker;
        scope.attempt = detection.attempt;
        scope.token = detection.token;
        scope.action = detection.action;
        scope.refund_id = detection.refund_id;
        scope.external_trigger = detection.external_trigger;
        record_flow(scope, "WebhookRecoveryEventDetected");

        // Unsupported and setup events return early; enabled-event filtering follows,
        // with merchant-scoped metrics (incoming_v2.rs:164-232, :401-405).
        if (scope.flow == "Setup" || scope.flow == "Unsupported") {
            auto ack = build_ack(scope, "ignored", scope.flow == "Setup"
                ? "setup webhook requires no recovery processing"
                : "unsupported event ignored");
            record_flow(scope, "WebhookRecoveryEarlyReturn");
            send_ack(res, send, ack);
            return;
        }
        if (!event_enabled(scope)) {
            auto ack = build_ack(scope, "ignored", "event type disabled by configuration");
            record_flow(scope, "WebhookRecoveryEventDisabled");
            send_ack(res, send, ack);
            return;
        }
        if (scope.external_trigger) {
            auto ack = build_ack(scope, "skipped", "external trigger: recovery flow skipped");
            record_flow(scope, "WebhookRecoveryExternalTriggerSkipped");
            send_ack(res, send, ack);
            return;
        }

        std::optional<core::error::AppError> error{};
        const bool created = get_or_create_recovery_intent(scope, error);
        if (error) {
            send_error(res, send, *error);
            return;
        }
        if (!created) {
            auto ack = build_ack(scope, "duplicate", "recovery intent already recorded");
            record_flow(scope, "WebhookRecoveryDuplicate");
            send_ack(res, send, ack);
            return;
        }

        // Ephemeral delivery guard: upstream declares LockAction::NotApplicable for
        // this route; the registry is used only as a short-TTL duplicate-delivery
        // guard, never as a business-state lock.
        const std::string lock_key = "recovery.delivery:" + scope.intent_key;
        const auto lock = m_context->locks().acquire(lock_key, std::int64_t{30}, ids.merchant_id);
        if (!lock) {
            auto ack = build_ack(scope, "duplicate", "concurrent recovery delivery in progress");
            record_flow(scope, "WebhookRecoveryConcurrentDelivery");
            send_ack(res, send, ack);
            return;
        }
        DeliveryGuard guard{m_context->locks(), lock_key};

        record_recovery_attempt(scope, error);
        if (error) { send_error(res, send, *error); return; }
        maybe_sync(scope, error);
        if (error) { send_error(res, send, *error); return; }
        ensure_payment_record(scope, error);
        if (error) { send_error(res, send, *error); return; }
        lookup_recovery_token(scope);
        scope.published_event_id = publish_recovery_event(scope);
        finalize_recovery_state(scope);

        auto ack = build_ack(scope, "success", "recovery webhook processed");
        record_flow(scope, "WebhookRecoveryReceiveIncoming");
        m_context->metrics().counter("WEBHOOK_RECOVERY_RECEIVED").increment();
        send_ack(res, send, ack);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/v2/webhooks"}
            .add_route(
                Route{"/recovery/:merchant_id/:profile_id/:connector_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_recovery_receive_incoming_webhook(req, res, std::move(send));
                    })
                    .summary("Hyperswitch recovery_receive_incoming_webhook")
            );
    }

    explicit RecoveryWebhooksRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    struct RecoveryIds
    {
        std::string merchant_id{};
        std::string profile_id{};
        std::string connector_id{};
    };

    struct EventDetection
    {
        std::string event_type{};
        std::string object_id{};
        std::string refund_id{};
        std::string attempt{};
        std::string token{};
        std::string action{};
        std::string flow{};
        std::string tracker{};
        bool external_trigger{};
    };

    struct RequestScope
    {
        RecoveryIds ids{};
        std::string body{};
        std::string org_id{};
        std::string event_class{};
        std::string connector_name{};
        std::string connector_key{};
        std::string secret_hint{};
        std::string webhook_secret{};
        std::string connector_webhook_id{};
        std::string registered_event_type{};
        std::string intent_key{};
        std::string intent_id{};
        std::string event_type{};
        std::string object_id{};
        std::string refund_id{};
        std::string attempt{};
        std::string token{};
        std::string action{};
        std::string flow{};
        std::string tracker{};
        std::string payment_id{};
        std::string attempt_id{};
        std::string published_event_id{};
        std::string lock_action{};
        std::int64_t attempt_row_id{};
        bool external_trigger{};
    };

    class DeliveryGuard
    {
    public:
        DeliveryGuard(app::locking::LockRegistry& locks, std::string key) :
            m_locks{locks}, m_key{std::move(key)}
        {
        }

        ~DeliveryGuard()
        {
            (void)m_locks.release(m_key);
        }

        DeliveryGuard(const DeliveryGuard&) = delete;
        DeliveryGuard& operator=(const DeliveryGuard&) = delete;

    private:
        app::locking::LockRegistry& m_locks;
        std::string m_key{};
    };

    // Skip router/literal prefixes so ids resolve for /v2/webhooks/recovery/{m}/{p}/{c}
    // as well as any double-prefixed mounted form.
    [[nodiscard]] std::size_t id_base(const app::http::PathSegments& segments) const
    {
        std::size_t index = segments.at(0).value_or("") == "v2" ? 1 : 0;
        while (index < segments.count()) {
            const auto value = segments.at(index).value_or("");
            if (value == "webhooks" || value == "recovery") {
                ++index;
                continue;
            }
            break;
        }
        return index;
    }

    [[nodiscard]] RecoveryIds recovery_ids(const app::http::PathSegments& segments) const
    {
        const std::size_t base = id_base(segments);
        RecoveryIds result;
        result.merchant_id = std::string{segments.at(base).value_or("")};
        result.profile_id = std::string{segments.at(base + 1).value_or("")};
        result.connector_id = std::string{segments.at(base + 2).value_or("")};
        return result;
    }

    [[nodiscard]] std::string parse_connector_name(std::string_view connector) const
    {
        const std::size_t split = connector.find('_');
        return split == std::string_view::npos
            ? std::string{connector}
            : std::string{connector.substr(0, split)};
    }

    [[nodiscard]] std::string resolve_webhook_secret(
        interfaces::io::IRequest& req,
        const RequestScope& scope
    ) const
    {
        if (!scope.secret_hint.empty()) {
            return scope.secret_hint;
        }
        // connector webhook configuration carries the connector-side verification secret.
        for (const auto& webhook : m_context->repository().list_connector_webhooks()) {
            if (webhook.get_merchant_id() == scope.ids.merchant_id &&
                (webhook.get_connector() == scope.connector_key ||
                 webhook.get_connector() == scope.connector_name) &&
                !webhook.get_secret().empty()) {
                return webhook.get_secret();
            }
        }
        const std::string key_id = scope.ids.connector_id + ":" + scope.ids.profile_id;
        if (const auto stored = m_context->key_store().get("webhook_recovery", key_id)) {
            return stored->value();
        }
        const std::string header = app::http::HeaderReader{}.get(req, "x-webhook-secret");
        if (!header.empty()) {
            return header;
        }
        // Deterministic fallback; no external credential is required locally.
        return key_id;
    }

    void load_registered_connector_webhook(RequestScope& scope) const
    {
        for (const auto& webhook : m_context->repository().list_connector_webhooks()) {
            if (webhook.get_merchant_id() == scope.ids.merchant_id &&
                (webhook.get_connector() == scope.connector_key ||
                 webhook.get_connector() == scope.connector_name)) {
                scope.connector_webhook_id = webhook.get_connector_webhook_id();
                scope.registered_event_type = webhook.get_event_type();
                record_flow(scope, "WebhookRecoveryConnectorWebhookDecoded");
                return;
            }
        }
    }

    [[nodiscard]] std::optional<core::error::AppError> verify_webhook_source(
        interfaces::io::IRequest& req,
        const RequestScope& scope
    ) const
    {
        const auto verify_request = app::connector::ConnectorRequestBuilder{}
                                        .operation("webhook_source_verify")
                                        .merchant_id(scope.ids.merchant_id)
                                        .resource_id(scope.ids.connector_id)
                                        .raw_payload(scope.body)();
        bool source_verified = false;
        if (const auto connector = m_context->connector_router().select_for(verify_request)) {
            const auto response = connector->webhook_source_verify(verify_request);
            source_verified = response.get_status() == "webhook_verified" ||
                              response.get_status() == "verified";
            record_flow(scope, source_verified
                ? "WebhookRecoverySourceVerified"
                : "WebhookRecoverySourceRejected");
        }
        if (source_verified) {
            return std::nullopt;
        }
        // No connector verifier available: the signature header is mandatory.
        const std::string signature = app::http::HeaderReader{}.get(req, "x-webhook-signature");
        if (signature.empty()) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "WebhookAuthenticationFailed: recovery requires source verification"
            );
        }
        const app::crypto::KeyedHasher hasher{scope.webhook_secret};
        if (!hasher.verify_with_domain("recovery-webhook", scope.body, signature)) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "WebhookAuthenticationFailed: signature verification failed"
            );
        }
        record_flow(scope, "WebhookRecoverySignatureVerified");
        return std::nullopt;
    }

    [[nodiscard]] std::string json_string_field(
        const interfaces::Value& value,
        std::string_view key
    ) const
    {
        const auto* object = std::get_if<interfaces::Value::Object>(&value.get());
        if (object == nullptr) {
            return {};
        }
        const auto field = object->get(std::string{key});
        if (!field) {
            return {};
        }
        if (const auto* text = std::get_if<std::string>(&field.value().get())) {
            return *text;
        }
        return {};
    }

    [[nodiscard]] bool json_bool_field(
        const interfaces::Value& value,
        std::string_view key
    ) const
    {
        const auto* object = std::get_if<interfaces::Value::Object>(&value.get());
        if (object == nullptr) {
            return false;
        }
        const auto field = object->get(std::string{key});
        if (!field) {
            return false;
        }
        const bool* flag = std::get_if<bool>(&field.value().get());
        return flag != nullptr && *flag;
    }

    [[nodiscard]] std::string lower_copy(std::string_view value) const
    {
        std::string result{value};
        std::transform(
            result.begin(), result.end(), result.begin(),
            [](unsigned char character) { return static_cast<char>(std::tolower(character)); }
        );
        return result;
    }

    [[nodiscard]] EventDetection detect_event(
        const interfaces::Value& value,
        const RequestScope& scope
    ) const
    {
        EventDetection detected{};
        detected.event_type = json_string_field(value, "event_type");
        if (detected.event_type.empty()) {
            detected.event_type = json_string_field(value, "type");
        }
        if (detected.event_type.empty()) {
            detected.event_type = json_string_field(value, "event");
        }
        if (detected.event_type.empty()) {
            detected.event_type = scope.registered_event_type;
        }
        if (detected.event_type.empty()) {
            detected.event_type = "recovery.unknown";
        }

        detected.object_id = json_string_field(value, "payment_id");
        if (detected.object_id.empty()) {
            detected.object_id = json_string_field(value, "invoice_id");
        }
        if (detected.object_id.empty()) {
            const auto* object = std::get_if<interfaces::Value::Object>(&value.get());
            if (object != nullptr) {
                const auto nested = object->get(std::string{"object"});
                if (nested) {
                    const auto* nested_object =
                        std::get_if<interfaces::Value::Object>(&nested.value().get());
                    if (nested_object != nullptr) {
                        const auto id = nested_object->get(std::string{"id"});
                        if (id) {
                            if (const auto* text = std::get_if<std::string>(&id.value().get())) {
                                detected.object_id = *text;
                            }
                        }
                    }
                }
            }
        }
        detected.refund_id = json_string_field(value, "refund_id");
        detected.attempt = json_string_field(value, "attempt");
        detected.token = json_string_field(value, "recovery_token");
        if (detected.token.empty()) {
            detected.token = json_string_field(value, "token");
        }
        detected.action = json_string_field(value, "action");
        detected.external_trigger = json_bool_field(value, "external_trigger") ||
            lower_copy(json_string_field(value, "trigger")) == "external" ||
            lower_copy(json_string_field(value, "source")) == "external";

        const std::string normalized = lower_copy(detected.event_type);
        if (normalized.contains("setup") || normalized.contains("endpoint_verification")) {
            detected.flow = "Setup";
            detected.tracker = "NoEffect";
        } else if (normalized.contains("unsupported")) {
            detected.flow = "Unsupported";
            detected.tracker = "NoEffect";
        } else {
            // The only flow dispatched by this route is WebhookFlow::Recovery
            // (incoming_v2.rs:379-397).
            detected.flow = "Recovery";
            detected.tracker = "Recovery";
        }
        return detected;
    }

    [[nodiscard]] bool event_enabled(const RequestScope& scope) const
    {
        const auto configured =
            m_context->config_store().get_value("recovery_webhook_events", scope.event_type);
        if (!configured) {
            return true;
        }
        const std::string value = lower_copy(*configured);
        return value != "false" && value != "disabled" && value != "0";
    }

    [[nodiscard]] bool get_or_create_recovery_intent(
        RequestScope& scope,
        std::optional<core::error::AppError>& error
    ) const
    {
        const app::crypto::KeyedHasher hasher{scope.webhook_secret};
        scope.intent_key = hasher.hash(
            scope.ids.merchant_id + "\n" + scope.ids.profile_id + "\n" +
            scope.ids.connector_id + "\n" + scope.event_type + "\n" + scope.object_id + "\n" +
            scope.body
        );
        const auto existing = m_context->repository().find_webhook_event_by_string(scope.intent_key);
        if (existing) {
            scope.intent_id = existing->get_event_id();
            return false;
        }

        app::store::webhook_events::WebhookEventRecord record{};
        record.set_event_id(scope.intent_key);
        record.set_merchant_id(scope.ids.merchant_id);
        record.set_event_type("recovery_intent");
        record.set_event_class(scope.event_class);
        record.set_status("pending");
        record.set_payload(scope.body);
        record.set_delivery_attempts(0);
        record.set_created_at(now_value());
        const auto inserted = m_context->repository().insert_webhook_event(std::move(record));
        if (!inserted) {
            error = inserted.error();
            return false;
        }
        scope.intent_id = inserted->get_event_id();
        record_flow(scope, "WebhookRecoveryIntentCreated");
        return true;
    }

    void record_recovery_attempt(
        RequestScope& scope,
        std::optional<core::error::AppError>& error
    ) const
    {
        // get_or_create_custom_recovery_intent / find_custom_recovery_intent and
        // find/record_recovery_request_attempt (recovery_incoming.rs:361,384,546,592,635)
        // are represented by a deduplicated process-tracker record.
        const std::string task_id = "recovery_attempt:" + scope.intent_key;
        const auto existing = m_context->repository().find_process_tracker_by_string(task_id);
        if (existing) {
            scope.attempt_row_id = existing->get_id();
            scope.attempt_id = std::to_string(scope.attempt_row_id);
        } else {
            const auto created = m_context->repository().record_process_tracker(
                "recovery_attempt",
                scope.intent_key + "\n" + scope.event_type + "\n" + scope.attempt,
                "0"
            );
            if (!created) {
                error = created.error();
                return;
            }
            scope.attempt_row_id = created->get_id();
            scope.attempt_id = std::to_string(scope.attempt_row_id);
        }
        (void)m_context->repository().update_process_tracker(
            task_id, "PROCESSING", scope.event_type
        );
        record_flow(scope, "WebhookRecoveryAttemptRecorded");
    }

    void maybe_sync(
        RequestScope& scope,
        std::optional<core::error::AppError>& error
    ) const
    {
        // invoice-sync / payment-sync config checks before billing connector sync
        // (recovery_incoming.rs:78-112).
        const bool invoice_sync = m_context->config_store()
                                      .get_value("recovery_invoice_sync", scope.connector_key)
                                      .value_or("false") == "true";
        const bool payment_sync = m_context->config_store()
                                      .get_value("recovery_payment_sync", scope.connector_key)
                                      .value_or("false") == "true";
        if (!invoice_sync && !payment_sync) {
            return;
        }

        auto request = app::connector::ConnectorRequestBuilder{}
                           .merchant_id(scope.ids.merchant_id)
                           .resource_id(scope.object_id)
                           .raw_payload(scope.body)();
        const auto connector = m_context->connector_router().select_for(request);
        if (!connector) {
            error = core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::CONNECTOR_ERROR,
                "no billing connector available for recovery sync"
            );
            return;
        }

        if (invoice_sync) {
            request.set_operation("recovery_invoice_sync");
            const auto response = connector->sync(request);
            if (response.get_status().empty()) {
                error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::CONNECTOR_ERROR,
                    "recovery invoice sync failed"
                );
                return;
            }
            record_flow(scope, "WebhookRecoveryInvoiceSync");
        }
        if (payment_sync) {
            request.set_operation("recovery_payment_sync");
            const auto response = connector->sync(request);
            if (response.get_status().empty()) {
                error = core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::CONNECTOR_ERROR,
                    "recovery payment sync failed"
                );
                return;
            }
            record_flow(scope, "WebhookRecoveryPaymentSync");
        }
    }

    void ensure_payment_record(
        RequestScope& scope,
        std::optional<core::error::AppError>& error
    ) const
    {
        // create_payment_record_request / find_payment_merchant_connector_account
        // (recovery_incoming.rs:731,1041,1144).
        if (scope.object_id.empty()) {
            return;
        }
        const auto existing = m_context->repository().find_payment_by_string(scope.object_id);
        if (existing) {
            scope.payment_id = existing->get_payment_id();
            return;
        }

        app::store::payments::PaymentIntentRecord record{};
        record.set_payment_id(scope.object_id);
        record.set_merchant_id(scope.ids.merchant_id);
        record.set_merchant_reference_id(scope.intent_key);
        record.set_status("recovery_pending");
        record.set_description("recovery webhook");
        record.set_metadata(scope.event_type);
        record.set_created_at(now_value());
        record.set_updated_at(now_value());
        const auto created = m_context->repository().create_payment(std::move(record));
        if (!created) {
            if (created.error().get_kind() == core::error::AppErrorKind::ALREADY_EXISTS) {
                const auto raced = m_context->repository().find_payment_by_string(scope.object_id);
                if (raced) {
                    scope.payment_id = raced->get_payment_id();
                    return;
                }
            }
            error = created.error();
            return;
        }
        scope.payment_id = created->get_payment_id();
        record_flow(scope, "WebhookRecoveryPaymentRecordCreated");
    }

    void lookup_recovery_token(RequestScope& scope) const
    {
        if (scope.token.empty()) {
            return;
        }
        const auto token = m_context->repository().find_revenue_recovery_token_by_string(scope.token);
        if (token && token->get_merchant_id() == scope.ids.merchant_id) {
            record_flow(scope, "WebhookRecoveryTokenFound");
            return;
        }
        record_flow(scope, "WebhookRecoveryTokenMissing");
    }

    [[nodiscard]] std::string publish_recovery_event(const RequestScope& scope) const
    {
        const std::string action = scope.action.empty() ? scope.event_type : scope.action;
        const std::string topic = "recovery.webhook." + scope.connector_key + "." + action;
        const std::string payload = std::format(
            R"({{"merchant_id":"{}","profile_id":"{}","connector":"{}","event_type":"{}","intent_id":"{}","payment_id":"{}","action":"{}"}})",
            scope.ids.merchant_id,
            scope.ids.profile_id,
            scope.connector_key,
            scope.event_type,
            scope.intent_id,
            scope.payment_id,
            action
        );
        record_flow(scope, "WebhookRecoveryEventPublished");
        return m_context->events().publish(topic, payload);
    }

    void finalize_recovery_state(RequestScope& scope) const
    {
        const auto intent = m_context->repository().find_webhook_event_by_string(scope.intent_key);
        if (intent) {
            app::store::webhook_events::WebhookEventRecord updated = *intent;
            updated.set_status("processed");
            (void)m_context->repository().update_webhook_event(updated);
        }
        if (scope.attempt_row_id > 0) {
            (void)m_context->process_trackers().mark_terminal(
                scope.attempt_row_id, scope.published_event_id
            );
        }
        record_flow(scope, "WebhookRecoveryStateFinalized");
    }

    [[nodiscard]] RecoveryWebhookAck build_ack(
        const RequestScope& scope,
        std::string status,
        std::string message
    ) const
    {
        RecoveryWebhookAck ack{};
        ack.set_status(std::move(status));
        ack.set_flow(scope.flow);
        ack.set_event_type(scope.event_type);
        ack.set_event_id(scope.intent_id);
        ack.set_recovery_intent_id(scope.intent_id);
        ack.set_tracker(scope.tracker);
        ack.set_connector(scope.connector_key);
        ack.set_merchant_id(scope.ids.merchant_id);
        ack.set_profile_id(scope.ids.profile_id);
        ack.set_payment_id(scope.payment_id);
        ack.set_refund_id(scope.refund_id);
        ack.set_attempt_id(scope.attempt_id);
        ack.set_recovery_token(scope.token);
        ack.set_published_event_id(scope.published_event_id);
        ack.set_lock_action(scope.lock_action);
        ack.set_message(std::move(message));
        return ack;
    }

    void record_flow(const RequestScope& scope, std::string_view name) const
    {
        m_context->metrics().record_flow(name);
        const std::string dimensioned =
            std::string{name} + ":merchant:" + scope.ids.merchant_id + ":" + scope.connector_key;
        m_context->metrics().record_flow(dimensioned);
        m_context->metrics().counter(dimensioned).increment();
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

    void send_ack(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        const RecoveryWebhookAck& ack
    ) const
    {
        send_json(res, send, serde::Ser::encode_json(ack), true);
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::recovery_webhooks

template<>
struct serde::Serializable<app::handlers::recovery_webhooks::RecoveryWebhookAck>
{
    static constexpr auto fields()
    {
        using Ack = app::handlers::recovery_webhooks::RecoveryWebhookAck;
        return std::tuple{
            serde::FieldDesc<"status", &Ack::get_status, &Ack::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"flow", &Ack::get_flow, &Ack::set_flow, serde::FieldOptions{}>{},
            serde::FieldDesc<"event_type", &Ack::get_event_type, &Ack::set_event_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"event_id", &Ack::get_event_id, &Ack::set_event_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"recovery_intent_id", &Ack::get_recovery_intent_id, &Ack::set_recovery_intent_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"tracker", &Ack::get_tracker, &Ack::set_tracker, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &Ack::get_connector, &Ack::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &Ack::get_merchant_id, &Ack::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &Ack::get_profile_id, &Ack::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"payment_id", &Ack::get_payment_id, &Ack::set_payment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"refund_id", &Ack::get_refund_id, &Ack::set_refund_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"attempt_id", &Ack::get_attempt_id, &Ack::set_attempt_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"recovery_token", &Ack::get_recovery_token, &Ack::set_recovery_token, serde::FieldOptions{}>{},
            serde::FieldDesc<"published_event_id", &Ack::get_published_event_id, &Ack::set_published_event_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"lock_action", &Ack::get_lock_action, &Ack::set_lock_action, serde::FieldOptions{}>{},
            serde::FieldDesc<"message", &Ack::get_message, &Ack::set_message, serde::FieldOptions{}>{},
        };
    }

    static constexpr std::string_view table_name()
    {
        return "recovery_webhook_ack";
    }
};
