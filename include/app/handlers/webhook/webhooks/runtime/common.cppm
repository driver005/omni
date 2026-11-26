export module app_webhook:webhooks_runtime_common;

import std;
import interfaces;
import core_serde;
import app_dto;
import app_context;
import core_error;
import app_http;
import app_metrics;
import app_connector;
import app_crypto;
import app_events;
import app_keystore;
import app_store;

import :webhooks_input;
import :webhooks_result;

export namespace app::handlers::webhooks {

/** Local mirror of upstream `ApiErrorResponse` -> `ErrorResponse` ids. */
enum class ErrorId : std::uint8_t
{
    UNAUTHORIZED,
    WEBHOOK_AUTH_FAILED,
    WEBHOOK_BAD_REQUEST,
    WEBHOOK_PROCESSING_FAILURE,
    INCORRECT_CONNECTOR_NAME,
    WEBHOOK_UNPROCESSABLE,
    WEBHOOK_RATE_LIMITED,
    INTERNAL,
};

/** @brief Local mirror of `fetch_three_ds_execution_path` outcome (incoming.rs:473-534). */
enum class ExecutionPath : std::uint8_t
{
    DIRECT,
    UNIFIED_AUTHENTICATION_SERVICE,
    UNIFIED_CONNECTOR_SERVICE,
};

/** @brief Local mirror of `VerificationStatus` (source_verification.rs:29-68). */
enum class VerificationStatus : std::uint8_t
{
    VERIFIED,
    REQUIRED_BUT_DISABLED,
    NOT_REQUIRED,
};

class NetworkVerification
{
public:
    VerificationStatus mtls{VerificationStatus::NOT_REQUIRED};
    VerificationStatus source_ip{VerificationStatus::NOT_REQUIRED};

    /** @brief Port of NetworkVerification::resolve_source_verified (source_verification.rs:60-68).
     */
    [[nodiscard]] bool resolve_source_verified(bool connector_source_verified) const noexcept
    {
        if (mtls == VerificationStatus::REQUIRED_BUT_DISABLED) {
            return false;
        }
        if (mtls == VerificationStatus::VERIFIED || source_ip == VerificationStatus::VERIFIED) {
            return true;
        }
        return connector_source_verified;
    }
};

struct ConnectorAccount
{
    std::optional<::app::store::merchant_connectors::MerchantConnectorRecord> mca{};
    std::string connector_name{};
};

struct ObjectRef
{
    std::string id{};
    std::string type{};
};

/**
 * @brief Shared business helpers for the webhook sub-runtimes.
 *
 * Error values are `core::error::AppError`s. The kind is chosen so that
 * `get_status()` matches upstream exactly; `kind_name` carries the upstream
 * ApiErrorResponse code (IR_01 / WE_01 / ...) and serializes as the `error`
 * field. The real human-readable text stays in `get_message()`.
 */
class WebhooksRuntimeSupport
{
public:
    explicit WebhooksRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ---- lexical helpers -------------------------------------------------

    [[nodiscard]] std::string quote(std::string_view value) const
    {
        return serde::Ser::encode_json(std::string{value});
    }

    [[nodiscard]] std::string to_lower(std::string_view value) const
    {
        std::string out;
        out.reserve(value.size());
        for (const char character: value) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
        }
        return out;
    }

    [[nodiscard]] std::optional<std::string>
    extract_string_field(std::string_view payload, std::string_view key) const
    {
        const std::string pattern{std::format("\"{}\"", key)};
        const std::size_t key_pos = payload.find(pattern);
        if (key_pos == std::string_view::npos) {
            return std::nullopt;
        }
        std::size_t cursor = key_pos + pattern.size();
        while (cursor < payload.size() &&
               std::isspace(static_cast<unsigned char>(payload[cursor]))) {
            ++cursor;
        }
        if (cursor >= payload.size() || payload[cursor] != ':') {
            return std::nullopt;
        }
        ++cursor;
        while (cursor < payload.size() &&
               std::isspace(static_cast<unsigned char>(payload[cursor]))) {
            ++cursor;
        }
        if (cursor >= payload.size() || payload[cursor] != '"') {
            return std::nullopt;
        }
        ++cursor;
        const std::size_t end = payload.find('"', cursor);
        if (end == std::string_view::npos) {
            return std::nullopt;
        }
        return std::string{payload.substr(cursor, end - cursor)};
    }

    [[nodiscard]] bool contains(std::string_view haystack, std::string_view needle) const
    {
        return haystack.find(needle) != std::string_view::npos;
    }

    [[nodiscard]] std::optional<std::string>
    config(std::string_view section, std::string_view key) const
    {
        return m_context.config_store().get_value(section, key);
    }

    [[nodiscard]] bool
    config_bool(std::string_view section, std::string_view key, bool fallback = false) const
    {
        const auto value = config(section, key);
        if (!value) {
            return fallback;
        }
        const std::string lowered = to_lower(*value);
        return lowered == "true" || lowered == "1";
    }

    [[nodiscard]] std::string now_seconds() const
    {
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
                                 std::chrono::system_clock::now().time_since_epoch()
        )
                                 .count();
        return std::to_string(seconds);
    }

    // ---- errors ----------------------------------------------------------

    [[nodiscard]] core::error::AppError make_error(ErrorId id, std::string message) const
    {
        using core::error::AppErrorKind;
        AppErrorKind kind = AppErrorKind::INTERNAL;
        switch (id) {
            case ErrorId::UNAUTHORIZED:
            case ErrorId::WEBHOOK_AUTH_FAILED:
                kind = AppErrorKind::UNAUTHORIZED;
                break;
            case ErrorId::WEBHOOK_BAD_REQUEST:
                kind = AppErrorKind::INVALID_REQUEST;
                break;
            case ErrorId::WEBHOOK_PROCESSING_FAILURE:
            case ErrorId::INTERNAL:
                kind = AppErrorKind::INTERNAL;
                break;
            case ErrorId::INCORRECT_CONNECTOR_NAME:
                kind = AppErrorKind::NOT_FOUND;
                break;
            case ErrorId::WEBHOOK_UNPROCESSABLE:
                kind = AppErrorKind::VALIDATION_FAILED;
                break;
            case ErrorId::WEBHOOK_RATE_LIMITED:
                kind = AppErrorKind::RATE_LIMITED;
                break;
        }
        auto error = core::error::AppErrorBuilder{}(kind, std::move(message));
        error.set_kind_name(std::string{code_for(id)});
        return error;
    }

    // ---- MCA / execution path --------------------------------------------

    [[nodiscard]] bool is_connector_name(
        std::string_view token,
        const std::vector<::app::store::merchant_connectors::MerchantConnectorRecord>& records
    ) const
    {
        for (const auto& record: records) {
            if (record.get_connector_name() == token) {
                return true;
            }
        }
        // Local substitute for Connector::from_str (incoming.rs:3562). MCA ids
        // are prefixed upstream; every other token is treated as a connector name.
        return !token.starts_with("mca_");
    }

    // Port of validate_webhook_connector_account (incoming.rs:3556-3595): not
    // found is ApiErrorResponse::Unauthorized, never NOT_FOUND.
    [[nodiscard]] std::optional<core::error::AppError> validate_webhook_connector_account(
        const std::string& merchant_id,
        const PathIds& ids,
        ConnectorAccount& account
    ) const
    {
        const auto records = m_context.repository().list_merchant_connectors();
        const bool by_name = is_connector_name(ids.connector_id, records);

        if (by_name) {
            bool found = false;
            for (const auto& record: records) {
                if (record.get_merchant_id() != merchant_id ||
                    record.get_connector_name() != ids.connector_id) {
                    continue;
                }
                if (!ids.profile_id.empty() && !record.get_profile_id().empty() &&
                    record.get_profile_id() != ids.profile_id) {
                    continue;
                }
                account.mca = record;
                found = true;
                break;
            }
            if (!found) {
                return make_error(
                    ErrorId::UNAUTHORIZED,
                    "API key not provided or invalid API key used"
                );
            }
            account.connector_name = ids.connector_id;
            return std::nullopt;
        }

        for (const auto& record: records) {
            if (record.get_merchant_id() == merchant_id &&
                record.get_merchant_connector_id() == ids.connector_id) {
                if (!ids.profile_id.empty() && !record.get_profile_id().empty() &&
                    record.get_profile_id() != ids.profile_id) {
                    continue;
                }
                account.mca = record;
                account.connector_name = record.get_connector_name().empty()
                                             ? ids.connector_id
                                             : record.get_connector_name();
                return std::nullopt;
            }
        }
        return make_error(ErrorId::UNAUTHORIZED, "API key not provided or invalid API key used");
    }

    // Port of fetch_three_ds_execution_path + confirm_path_for_flow
    // (incoming.rs:160-173, :473-517) using local config as the substitute for
    // superposition/authentication-service state.
    [[nodiscard]] ExecutionPath
    fetch_execution_path(const std::string& merchant_id, const std::string& connector_name) const
    {
        const auto integration = config("webhooks", "connector_integration_type:" + connector_name);
        if (integration && to_lower(*integration) == "ucs") {
            return ExecutionPath::UNIFIED_CONNECTOR_SERVICE;
        }

        const auto eligible = config("authentication_service", "enabled_connectors");
        const bool connector_eligible =
            eligible && contains(to_lower(*eligible), to_lower(connector_name));
        const bool merchant_eligible =
            config_bool("authentication_service", "merchant_eligible:" + merchant_id);
        if (connector_eligible && merchant_eligible) {
            return ExecutionPath::UNIFIED_AUTHENTICATION_SERVICE;
        }
        return ExecutionPath::DIRECT;
    }

    // ---- source verification ---------------------------------------------

    // Port of source_verification.rs:161-247 with the local header/config
    // substitute for the trusted-ingress mTLS signal.
    [[nodiscard]] std::optional<core::error::AppError> verify_mtls(
        const HeaderReaderFn& header,
        const std::string& connector_name,
        VerificationStatus& status
    ) const
    {
        status = VerificationStatus::NOT_REQUIRED;
        if (!config_bool("webhook_source", "mtls_required:" + connector_name)) {
            return std::nullopt;
        }
        if (!config_bool("webhook_source", "mtls_enabled")) {
            status = VerificationStatus::REQUIRED_BUT_DISABLED;
            return std::nullopt;
        }
        const std::string header_name = config("incoming_webhooks", "mtls_verified_header")
                                            .value_or(std::string{"x-webhook-mtls-verified"});
        const std::string value = header(header_name);
        if (value != "true") {
            return make_error(
                ErrorId::WEBHOOK_AUTH_FAILED,
                "Incoming webhook mTLS verification failed: the request has missing or invalid "
                "verification header"
            );
        }
        status = VerificationStatus::VERIFIED;
        return std::nullopt;
    }

    // Port of ip_verification.rs:51-104. The local config value is the
    // connector's allowed-range policy; CIDR prefixes are matched on their
    // network portion (local in-process substitute for the Rust IpAddr parser).
    [[nodiscard]] std::optional<core::error::AppError> verify_source_ip(
        const HeaderReaderFn& header,
        const std::string& connector_name,
        VerificationStatus& status
    ) const
    {
        status = VerificationStatus::NOT_REQUIRED;
        if (!config_bool("webhook_source", "ip_required:" + connector_name)) {
            return std::nullopt;
        }
        if (!config_bool("webhook_source", "ip_verification_enabled")) {
            status = VerificationStatus::REQUIRED_BUT_DISABLED;
            return std::nullopt;
        }
        const auto allowed = config("webhook_source", "ip_allowlist:" + connector_name);
        if (!allowed || allowed->empty()) {
            return make_error(
                ErrorId::INTERNAL,
                "Invalid incoming webhook allowed IP ranges config"
            );
        }
        const std::string header_name = config("incoming_webhooks", "source_ip_header")
                                            .value_or(std::string{"x-forwarded-for"});
        const std::string source_ip = header(header_name);
        if (source_ip.empty()) {
            return make_error(
                ErrorId::WEBHOOK_AUTH_FAILED,
                "Incoming webhook source IP header is missing"
            );
        }

        bool matched = false;
        std::size_t cursor = 0;
        while (cursor < allowed->size()) {
            const std::size_t comma = allowed->find(',', cursor);
            const std::string entry = allowed->substr(
                cursor,
                comma == std::string::npos ? std::string::npos : comma - cursor
            );
            const std::size_t slash = entry.find('/');
            const std::string network = slash == std::string::npos ? entry : entry.substr(0, slash);
            if (!network.empty() && source_ip.starts_with(network)) {
                matched = true;
                break;
            }
            if (comma == std::string::npos) {
                break;
            }
            cursor = comma + 1;
        }
        if (!matched) {
            return make_error(
                ErrorId::WEBHOOK_AUTH_FAILED,
                "Incoming webhook source IP is outside the allowed ranges"
            );
        }
        status = VerificationStatus::VERIFIED;
        return std::nullopt;
    }

    [[nodiscard]] std::optional<core::error::AppError> verify_network_source(
        const HeaderReaderFn& header,
        const std::string& connector_name,
        NetworkVerification& verification
    ) const
    {
        if (const auto failure = verify_mtls(header, connector_name, verification.mtls)) {
            return failure;
        }
        return verify_source_ip(header, connector_name, verification.source_ip);
    }

    [[nodiscard]] std::string
    webhook_secret(const HeaderReaderFn& header, const ConnectorAccount& account) const
    {
        if (account.mca) {
            const auto details = account.mca->get_connector_webhook_details();
            if (const auto secret = extract_string_field(details, "webhook_secret")) {
                return *secret;
            }
            if (const auto secret = extract_string_field(details, "merchant_secret")) {
                return *secret;
            }
        }
        if (const auto stored =
                m_context.key_store().get("webhook_signature", account.connector_name)) {
            return stored->value();
        }
        return header("x-webhook-secret");
    }

    // Local connector-level verify_webhook_source (incoming_v2.rs:246-290):
    // HMAC signature over the raw body, or the connector source-verification
    // call when configured for this connector.
    [[nodiscard]] bool verify_webhook_source_call(
        const HeaderReaderFn& header,
        std::string_view body,
        const ConnectorAccount& account,
        ExecutionPath path
    ) const
    {
        const auto call_config = config("webhook_source_verification_call", "connectors");
        const bool use_call =
            call_config && contains(to_lower(*call_config), to_lower(account.connector_name));

        if (use_call || path == ExecutionPath::UNIFIED_CONNECTOR_SERVICE) {
            const auto selected =
                m_context.connector_router().select_checked(account.connector_name);
            if (selected.has_value() && *selected) {
                auto request =
                    ::app::connector::ConnectorRequestBuilder{}
                        .operation("webhook_source_verify")
                        .merchant_id(account.mca ? account.mca->get_merchant_id() : std::string{})
                        .raw_payload(std::string{body})();
                const auto response = (*selected)->webhook_source_verify(request);
                return !response.get_status().empty() || !response.get_reference_id().empty();
            }
            return false;
        }

        static constexpr std::string_view signature_headers[] = {
            "x-webhook-signature",
            "stripe-signature",
            "adyen-hmac-signature",
            "x-signature",
        };
        std::string signature;
        for (const auto header_name: signature_headers) {
            signature = header(header_name);
            if (!signature.empty()) {
                break;
            }
        }
        if (signature.empty()) {
            return false;
        }
        const std::string secret = webhook_secret(header, account);
        if (secret.empty()) {
            return false;
        }
        return ::app::crypto::KeyedHasher{::app::crypto::SecretString{secret}.value()}.verify(
            body,
            signature
        );
    }

    [[nodiscard]] bool webhook_source_mandatory(const std::string& connector_name) const
    {
        return config_bool("webhook_source", "mandatory:" + connector_name);
    }

    // ---- webhook body decoding -------------------------------------------

    // Local substitute for connector `decode_webhook_body` (incoming_v2.rs:151-160).
    [[nodiscard]] std::optional<core::error::AppError>
    decode_webhook_body(std::string_view body) const
    {
        if (!serde::Ser::decode_generic("application/json", body)) {
            return make_error(
                ErrorId::WEBHOOK_UNPROCESSABLE,
                "There was an issue processing the webhook body"
            );
        }
        return std::nullopt;
    }

    // Local substitute for connector `get_webhook_event_type`
    // (incoming_v2.rs:164-212). Nullopt makes the caller ack without effects,
    // matching upstream's early return for unidentified event types.
    [[nodiscard]] std::optional<std::string> get_webhook_event_type(std::string_view body) const
    {
        auto declared = extract_string_field(body, "event_type");
        if (!declared) {
            declared = extract_string_field(body, "type");
        }
        if (declared) {
            const std::string lowered = to_lower(*declared);
            if (contains(lowered, "setup")) {
                return std::string{"setup_webhook"};
            }
            if (contains(lowered, "not_supported") || contains(lowered, "unknown")) {
                return std::string{"event_not_supported"};
            }
            if (contains(lowered, "payment")) {
                if (contains(lowered, "refund")) {
                    return std::string{"refund_success"};
                }
                if (contains(lowered, "authorization")) {
                    return std::string{"payment_intent_authorization_success"};
                }
                if (contains(lowered, "capture")) {
                    return std::string{"payment_intent_capture_success"};
                }
                if (contains(lowered, "fail")) {
                    return std::string{"payment_intent_failure"};
                }
                return std::string{"payment_intent_success"};
            }
            if (contains(lowered, "refund")) {
                return std::string{"refund_success"};
            }
            if (contains(lowered, "dispute")) {
                return std::string{"dispute_opened"};
            }
            if (contains(lowered, "mandate")) {
                return std::string{"mandate_active"};
            }
            if (contains(lowered, "payout")) {
                return std::string{"payout_success"};
            }
            if (contains(lowered, "recovery")) {
                return std::string{"recovery_payment_success"};
            }
            return std::string{"event_not_supported"};
        }

        const std::string lowered = to_lower(body);
        if (contains(lowered, "refund")) {
            return std::string{"refund_success"};
        }
        if (contains(lowered, "dispute")) {
            return std::string{"dispute_opened"};
        }
        if (contains(lowered, "mandate")) {
            return std::string{"mandate_active"};
        }
        if (contains(lowered, "payout")) {
            return std::string{"payout_success"};
        }
        if (contains(lowered, "recovery")) {
            return std::string{"recovery_payment_success"};
        }
        if (contains(lowered, "payment")) {
            return std::string{"payment_intent_success"};
        }
        return std::nullopt;
    }

    [[nodiscard]] ObjectRef
    get_webhook_object_reference_id(std::string_view body, const std::string& event_type) const
    {
        ObjectRef ref;
        const auto pick = [&](std::initializer_list<std::string_view> keys, std::string_view type)
        {
            for (const auto key: keys) {
                if (const auto value = extract_string_field(body, key)) {
                    ref.id = *value;
                    ref.type = std::string{type};
                    return true;
                }
            }
            return false;
        };

        if (contains(event_type, "refund")) {
            pick({"refund_id", "connector_refund_id"}, "RefundId");
        } else if (contains(event_type, "dispute")) {
            pick({"dispute_id", "connector_dispute_id"}, "DisputeId");
        } else if (contains(event_type, "mandate")) {
            pick({"mandate_id", "connector_mandate_id"}, "MandateId");
        } else if (contains(event_type, "payout")) {
            pick({"payout_id", "connector_payout_id"}, "PayoutId");
        } else if (contains(event_type, "recovery")) {
            pick({"invoice_id", "payment_id"}, "InvoiceId");
        } else {
            pick({"payment_id", "payment_intent_id", "connector_transaction_id"}, "PaymentId");
        }
        return ref;
    }

    [[nodiscard]] bool is_webhook_event_disabled(
        const std::string& connector_name,
        const std::string& event_type
    ) const
    {
        return config_bool("webhooks", "disabled_event:" + connector_name + ":" + event_type);
    }

    // Default upstream get_webhook_api_response -> WebhookResponse::StatusOk
    // (webhooks.rs:346-354). Local config mirrors connector-specific overrides
    // without a plugin registry: a value starting with '{' is JSON, otherwise
    // it is a plain-text acknowledgement.
    [[nodiscard]] WebhookAck get_webhook_api_response(const std::string& connector_name) const
    {
        WebhookAck ack;
        const auto override_value = config("webhook_ack", connector_name);
        if (!override_value || override_value->empty()) {
            ack.set_kind(AckKind::STATUS_OK);
            return ack;
        }
        if (override_value->starts_with('{')) {
            ack.set_kind(AckKind::JSON);
        } else {
            ack.set_kind(AckKind::TEXT_PLAIN);
        }
        ack.set_body(*override_value);
        return ack;
    }

    // ---- persistence / event logging substitutes -------------------------

    [[nodiscard]] std::optional<std::string> persist_event(
        const std::string& merchant_id,
        const std::string& connector_name,
        const std::string& event_type,
        std::string_view event_class,
        const std::string& payload
    ) const
    {
        ::app::store::webhook_events::WebhookEventRecord record;
        record.set_event_id(m_context.ids().uuid_v4());
        record.set_merchant_id(merchant_id);
        record.set_event_type(event_type);
        record.set_event_class(std::string{event_class});
        record.set_status("received");
        record.set_payload(payload);
        record.set_delivery_attempts(0);
        record.set_created_at(now_seconds());

        const auto inserted = m_context.repository().insert_webhook_event(std::move(record));
        if (!inserted.has_value()) {
            return std::nullopt;
        }
        return inserted->get_event_id();
    }

    // Local in-process substitute for ApplicationResponse::IncomingWebhookEvent
    // metadata consumption (incoming.rs:101-118, incoming_v2.rs:81-99). The
    // metadata is published on the local event bus, never added to the wire ack.
    void publish_metadata(
        std::string_view operation,
        std::string_view event_kind,
        const std::string& connector,
        const std::string& payment_id,
        const std::string& refund_id,
        const std::string& payment_method_id,
        const std::string& serialized_request,
        const std::string& tracker,
        const ObjectRef& object_ref,
        const std::string& event_type,
        std::string_view event_class,
        bool source_verified
    ) const
    {
        const std::string event_type_json =
            event_kind == "NetworkTokenWebhook"
                ? std::format(
                      R"({{"NetworkTokenWebhook":{{"payment_method_id":{}}}}})",
                      quote(payment_method_id)
                  )
                : std::format(
                      R"({{"Webhooks":{{"connector":{},"payment_id":{},"refund_id":{}}}}})",
                      quote(connector),
                      quote(payment_id),
                      quote(refund_id)
                  );
        const std::string tracker_json = std::format(
            R"({{"tracker":{},"object_reference_id":{},"object_reference_type":{},"event_type":{},"event_class":{},"source_verified":{}}})",
            quote(tracker),
            quote(object_ref.id),
            quote(object_ref.type),
            quote(event_type),
            quote(std::string{event_class}),
            source_verified ? "true" : "false"
        );
        const std::string metadata = std::format(
            R"({{"event_type":{},"serialized_request":{},"webhook_tracker_data":{}}})",
            event_type_json,
            quote(serialized_request),
            tracker_json
        );
        (void)m_context.events().publish(
            std::string{"webhook.incoming."} + std::string{operation},
            metadata
        );
    }

    void record_metrics(
        std::string_view flow,
        std::string_view counter,
        const std::string& merchant_id
    ) const
    {
        m_context.metrics().record_flow(flow);
        m_context.metrics().counter(counter).increment();
        m_context.metrics().counter("WEBHOOK_INCOMING_COUNT").increment();
        // Upstream labels the counter by merchant (incoming.rs:194-200); local
        // counters have no labels, so the dimension is a keyed-hash suffix.
        const ::app::crypto::KeyedHasher hasher{std::string{"webhook-metric"}};
        const std::string merchant_tag{hasher.hash(merchant_id)};
        m_context.metrics().counter("WEBHOOK_INCOMING_COUNT.merchant." + merchant_tag).increment();
        m_context.metrics().record_flow(std::string{"webhook.incoming."} + merchant_tag);
    }

    // ---- core: incoming_webhooks_wrapper + incoming_webhooks_core ---------

    // Port of incoming.rs:77-119 / :177-470 and incoming_v2.rs:52-103 / :103-...
    // Connector decoders, 3DS/UAS/UCS plugins and business-effect handlers are
    // replaced by the local substitutes above; the control flow, error shapes,
    // source-verification policy and wire acknowledgement stay upstream-shaped.
    [[nodiscard]] std::expected<WebhookAck, core::error::AppError> incoming_webhooks_wrapper(
        const PathIds& ids,
        const std::string& body,
        const HeaderReaderFn& header,
        std::string_view operation,
        std::string_view flow,
        std::string_view counter,
        bool is_relay,
        bool with_profile
    ) const
    {
        const std::string merchant_id =
            ids.merchant_id.empty() ? header("x-merchant-id") : ids.merchant_id;
        if (merchant_id.empty() || ids.connector_id.empty()) {
            return std::unexpected{
                make_error(ErrorId::WEBHOOK_BAD_REQUEST, "Bad request body received")
            };
        }

        ConnectorAccount account;
        if (const auto failure = validate_webhook_connector_account(merchant_id, ids, account)) {
            return std::unexpected{*failure};
        }

        record_metrics(flow, counter, merchant_id);
        const ExecutionPath path = fetch_execution_path(merchant_id, account.connector_name);

        NetworkVerification network;
        if (const auto failure = verify_network_source(header, account.connector_name, network)) {
            return std::unexpected{*failure};
        }

        if (const auto failure = decode_webhook_body(body)) {
            return std::unexpected{*failure};
        }

        ObjectRef object_ref;
        std::string event_type;
        std::string tracker{"NoEffect"};
        bool source_verified{false};

        const auto event = get_webhook_event_type(body);
        if (!event) {
            // Upstream early return for unidentified event types
            // (incoming_v2.rs:179-211): ack via connector, no effects, Null body.
            const auto ack = get_webhook_api_response(account.connector_name);
            publish_metadata(
                operation,
                "Webhooks",
                account.connector_name,
                "",
                "",
                "",
                "null",
                tracker,
                object_ref,
                "event_not_supported",
                "NoEffect",
                false
            );
            return ack;
        }
        event_type = *event;

        if (event_type == "setup_webhook") {
            // incoming_v2.rs:215-222 -> WebhookResponse::StatusOk.
            const auto ack = get_webhook_api_response(account.connector_name);
            publish_metadata(
                operation,
                "Webhooks",
                account.connector_name,
                "",
                "",
                "",
                "{}",
                tracker,
                object_ref,
                event_type,
                "NoEffect",
                true
            );
            return ack;
        }

        const bool supported = event_type != "event_not_supported";
        const bool enabled = !is_webhook_event_disabled(account.connector_name, event_type);
        const bool process_further = supported && enabled;

        if (process_further) {
            object_ref = get_webhook_object_reference_id(body, event_type);
            const bool connector_source_verified =
                verify_webhook_source_call(header, body, account, path);
            source_verified = network.resolve_source_verified(connector_source_verified);

            if (source_verified) {
                m_context.metrics().counter("WEBHOOK_SOURCE_VERIFIED_COUNT").increment();
            } else if (webhook_source_mandatory(account.connector_name)) {
                // incoming_v2.rs:305-318 -> WebhookAuthenticationFailed.
                return std::unexpected{
                    make_error(ErrorId::WEBHOOK_AUTH_FAILED, "Webhook authentication failed")
                };
            }

            tracker = is_relay                           ? "Relay"
                      : contains(event_type, "refund")   ? "Refund"
                      : contains(event_type, "dispute")  ? "Dispute"
                      : contains(event_type, "mandate")  ? "Mandate"
                      : contains(event_type, "payout")   ? "Payout"
                      : contains(event_type, "recovery") ? "Payment"
                      : contains(event_type, "payment")  ? "Payment"
                                                         : "NoEffect";

            const auto event_id = persist_event(
                merchant_id,
                account.connector_name,
                event_type,
                is_relay ? "incoming:relay" : (with_profile ? "incoming:v2" : "incoming"),
                body
            );
            if (!event_id || event_id->empty()) {
                return std::unexpected{make_error(
                    ErrorId::WEBHOOK_PROCESSING_FAILURE,
                    "There was an issue processing the webhook"
                )};
            }
        }

        const std::string event_class = with_profile ? "incoming:v2" : "incoming";
        publish_metadata(
            operation,
            "Webhooks",
            account.connector_name,
            tracker == "Payment" ? object_ref.id : "",
            tracker == "Refund" ? object_ref.id : "",
            "",
            body,
            tracker,
            object_ref,
            event_type,
            event_class,
            source_verified
        );

        return get_webhook_api_response(account.connector_name);
    }

    // Port of incoming.rs:121-158 / :863-934. Upstream NoAuth at the transport
    // layer; source verification is an exact Authorization-header comparison
    // against the network-tokenization service secret.
    [[nodiscard]] std::expected<WebhookAck, core::error::AppError>
    network_token_incoming_webhooks_wrapper(
        const PathIds& ids,
        const std::string& body,
        const HeaderReaderFn& header,
        std::string_view operation,
        std::string_view flow,
        std::string_view counter
    ) const
    {
        // get_network_token_resource_object (incoming.rs:872-878): deserialization
        // failure is InternalServerError.
        if (!serde::Ser::decode_generic("application/json", body)) {
            return std::unexpected{make_error(
                ErrorId::INTERNAL,
                "Network Token Requestor Webhook deserialization failed"
            )};
        }

        // Network tokenization service must be configured (incoming.rs:880-886).
        const auto secret =
            config("network_tokenization_service", "webhook_source_verification_key");
        if (!secret || secret->empty()) {
            return std::unexpected{
                make_error(ErrorId::INTERNAL, "Network Tokenization Service not configured")
            };
        }

        // Authorization::verify_webhook_source (network_tokenization_incoming.rs:212-237).
        const std::string authorization = header("authorization");
        if (authorization != *secret) {
            return std::unexpected{
                make_error(ErrorId::WEBHOOK_AUTH_FAILED, "Webhook authentication failed")
            };
        }

        // NetworkTokenWebhookResponse parse (incoming.rs:893-896).
        const bool has_card_reference =
            extract_string_field(body, "card_reference").has_value() ||
            extract_string_field(body, "card_reference_id").has_value() ||
            extract_string_field(body, "reference").has_value();
        if (!has_card_reference) {
            return std::unexpected{make_error(
                ErrorId::WEBHOOK_UNPROCESSABLE,
                "There was an issue processing the webhook body"
            )};
        }

        // callback_mapper substitute (network_tokenization_incoming.rs:54-70):
        // the local store has no callback mapper table, so ids are read from
        // the webhook payload with the x-merchant-id header as fallback.
        const std::string merchant_id =
            extract_string_field(body, "merchant_id").value_or(ids.merchant_id);
        const std::string payment_method_id =
            extract_string_field(body, "payment_method_id")
                .value_or(extract_string_field(body, "payment_method").value_or(""));
        const std::string customer_id = extract_string_field(body, "customer_id").value_or("");
        if (merchant_id.empty()) {
            return std::unexpected{make_error(
                ErrorId::WEBHOOK_UNPROCESSABLE,
                "There was an issue processing the webhook body"
            )};
        }

        m_context.metrics().counter("WEBHOOK_SOURCE_VERIFIED_COUNT").increment();
        record_metrics(flow, counter, merchant_id);

        ObjectRef ref;
        ref.id = payment_method_id;
        ref.type = "PaymentMethod";
        const auto event_id = persist_event(
            merchant_id,
            "network_token_requestor",
            "network_token_metadata_update",
            "incoming:network_token",
            body
        );
        if (!event_id || event_id->empty()) {
            return std::unexpected{make_error(
                ErrorId::WEBHOOK_PROCESSING_FAILURE,
                "There was an issue processing the webhook"
            )};
        }

        publish_metadata(
            operation,
            "NetworkTokenWebhook",
            "network_token_requestor",
            "",
            "",
            payment_method_id,
            body,
            "PaymentMethod",
            ref,
            "network_token_metadata_update",
            "incoming:network_token",
            true
        );
        (void)customer_id;

        // Upstream returns WebhookResponse::StatusOk (incoming.rs:928-933).
        return get_webhook_api_response("network_token_requestor");
    }

private:
    [[nodiscard]] std::string_view code_for(ErrorId id) const noexcept
    {
        switch (id) {
            case ErrorId::UNAUTHORIZED:
                return "IR_01";
            case ErrorId::WEBHOOK_AUTH_FAILED:
                return "WE_01";
            case ErrorId::WEBHOOK_BAD_REQUEST:
                return "WE_02";
            case ErrorId::WEBHOOK_PROCESSING_FAILURE:
                return "WE_03";
            case ErrorId::INCORRECT_CONNECTOR_NAME:
                return "HE_04";
            case ErrorId::WEBHOOK_UNPROCESSABLE:
                return "WE_05";
            case ErrorId::WEBHOOK_RATE_LIMITED:
                return "WE_07";
            case ErrorId::INTERNAL:
                return "HE_05";
        }
        return "HE_05";
    }

    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::webhooks
