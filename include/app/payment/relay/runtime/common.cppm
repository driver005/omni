export module app_payment:relay_runtime_common;

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
import app_workflow;
import app_keystore;
import app_locking;
import app_services;
import :relay_input;

export namespace app::handlers::relay {

/** @brief Local mirror of upstream RelayType variants (core/relay.rs:132-149). */
enum class RelayKind
{
    Refund,
    Capture,
    IncrementalAuthorization,
    Void,
    UnreferencedRefund,
    Unknown
};

/** @brief Local mirror of IncomingWebhookEvent variants used by relay flows. */
enum class RelayWebhookEvent
{
    RefundSuccess,
    RefundFailure,
    Setup,
    Unsupported
};

struct WebhookAccountCheck
{
    std::optional<core::error::AppError> failure{};
    std::string connector_name{};
    std::string profile_id{};
};

struct WebhookSourceCheck
{
    bool verified{false};
    std::string method{};
    std::optional<core::error::AppError> failure{};
};

struct DecodedRelayWebhook
{
    RelayWebhookEvent event{RelayWebhookEvent::Unsupported};
    std::string object_reference_id{};
    bool connector_refund_id{false};
    std::string raw_event_type{};
    bool setup{false};
};

/**
 * @brief Shared business helpers for the relay sub-runtimes.
 *
 * Holds the AppContext reference and the local ports of the upstream relay /
 * webhook utility chain. No HTTP types are present here.
 */
class RelayRuntimeSupport
{
public:
    explicit RelayRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    // ───────────────────────── Relay type helpers ─────────────────────────

    [[nodiscard]] RelayKind relay_kind(std::string_view value) const
    {
        if (value == "Refund" || value == "refund") {
            return RelayKind::Refund;
        }
        if (value == "Capture" || value == "capture") {
            return RelayKind::Capture;
        }
        if (value == "IncrementalAuthorization" || value == "incremental_authorization" ||
            value == "incrementalAuthorization") {
            return RelayKind::IncrementalAuthorization;
        }
        if (value == "Void" || value == "void") {
            return RelayKind::Void;
        }
        if (value == "UnreferencedRefund" || value == "unreferenced_refund") {
            return RelayKind::UnreferencedRefund;
        }
        return RelayKind::Unknown;
    }

    /** @brief Reads the canonical variant prefix stored ahead of the raw payload. */
    [[nodiscard]] std::string_view stored_kind(std::string_view payload) const
    {
        const auto separator = payload.find('|');
        return separator == std::string_view::npos ? payload : payload.substr(0, separator);
    }

    [[nodiscard]] std::string canonical_kind(RelayKind kind) const
    {
        switch (kind) {
            case RelayKind::Refund:
                return "Refund";
            case RelayKind::Capture:
                return "Capture";
            case RelayKind::IncrementalAuthorization:
                return "IncrementalAuthorization";
            case RelayKind::Void:
                return "Void";
            case RelayKind::UnreferencedRefund:
                return "UnreferencedRefund";
            default:
                return "Unknown";
        }
    }

    /** @brief serde key of the RelayData enum variant (api_models/relay.rs:23-32). */
    [[nodiscard]] std::string_view relay_data_key(RelayKind kind) const
    {
        switch (kind) {
            case RelayKind::Refund:
                return "refund";
            case RelayKind::Capture:
                return "capture";
            case RelayKind::IncrementalAuthorization:
                return "incremental_authorization";
            case RelayKind::Void:
                return "void";
            case RelayKind::UnreferencedRefund:
                return "unreferenced_refund";
            default:
                return "";
        }
    }

    /** @brief RelayType wire spelling (common_enums/enums.rs:3574-3582). */
    [[nodiscard]] std::string_view relay_type_wire(RelayKind kind) const
    {
        switch (kind) {
            case RelayKind::Refund:
                return "refund";
            case RelayKind::Capture:
                return "capture";
            case RelayKind::IncrementalAuthorization:
                return "incremental_authorization";
            case RelayKind::Void:
                return "void";
            case RelayKind::UnreferencedRefund:
                return "unreferenced_refund";
            default:
                return "";
        }
    }

    // ───────────────────────── Generic JSON tree helpers ─────────────────────────

    [[nodiscard]] std::optional<serde::Value>
    field_value(const serde::Value& value, std::string_view name) const
    {
        const auto* object = std::get_if<serde::Value::Object>(&value.get());
        if (object == nullptr) {
            return std::nullopt;
        }
        const auto field = object->get(std::string{name});
        if (!field) {
            return std::nullopt;
        }
        return *field;
    }

    [[nodiscard]] std::optional<std::string>
    field_string(const serde::Value& value, std::string_view name) const
    {
        const auto raw = field_value(value, name);
        if (!raw) {
            return std::nullopt;
        }
        if (const auto* text = std::get_if<std::string>(&raw->get())) {
            return *text;
        }
        if (const auto* object = std::get_if<serde::Value::Object>(&raw->get())) {
            const auto inner = object->get(std::string{"value"});
            if (inner) {
                if (const auto* text = std::get_if<std::string>(&inner.value().get())) {
                    return *text;
                }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::int64_t>
    field_number(const serde::Value& value, std::string_view name) const
    {
        const auto raw = field_value(value, name);
        if (!raw) {
            return std::nullopt;
        }
        if (const auto* text = std::get_if<std::string>(&raw->get())) {
            return parse_amount(*text);
        }
        if (const auto* number = std::get_if<std::int64_t>(&raw->get())) {
            return *number;
        }
        if (const auto* number = std::get_if<double>(&raw->get())) {
            return static_cast<std::int64_t>(*number);
        }
        if (const auto* object = std::get_if<serde::Value::Object>(&raw->get())) {
            const auto inner = object->get(std::string{"value"});
            if (inner) {
                if (const auto* text = std::get_if<std::string>(&inner.value().get())) {
                    return parse_amount(*text);
                }
                if (const auto* number = std::get_if<std::int64_t>(&inner.value().get())) {
                    return *number;
                }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::int64_t> parse_amount(std::string_view text) const
    {
        std::int64_t value{};
        const auto [end, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
        if (ec != std::errc{} || end != text.data() + text.size()) {
            return std::nullopt;
        }
        return value;
    }

    /**
     * @brief Upstream RelayResponse.data: Some(RelayData) externally tagged.
     * Decodes the persisted generic request payload into the typed response DTO
     * (include/app/dto/relay.cppm); `RelayResponseData::to_wire` emits the exact
     * `{"<variant>": {...}}` shape through its serde field converter.
     */
    [[nodiscard]] std::optional<::app::dto::relay::RelayResponseData>
    tagged_relay_data(RelayKind kind, const serde::Value& inner) const
    {
        namespace DTO = ::app::dto::relay;
        DTO::RelayResponseData result{};
        switch (kind) {
            case RelayKind::Refund:
                {
                    DTO::RelayRefundRequestData data{};
                    DTO::MinorUnitValue amount{};
                    amount.set_value(std::to_string(field_number(inner, "amount").value_or(0)));
                    data.set_amount(std::move(amount));
                    DTO::CurrencyValue currency{};
                    currency.set_value(field_string(inner, "currency").value_or(""));
                    data.set_currency(std::move(currency));
                    data.set_reason(field_string(inner, "reason"));
                    result.set_refund(data);
                    break;
                }
            case RelayKind::Capture:
                {
                    DTO::RelayCaptureRequestData data{};
                    DTO::MinorUnitValue authorized{};
                    authorized.set_value(
                        std::to_string(field_number(inner, "authorized_amount").value_or(0))
                    );
                    data.set_authorized_amount(std::move(authorized));
                    DTO::MinorUnitValue to_capture{};
                    to_capture.set_value(
                        std::to_string(field_number(inner, "amount_to_capture").value_or(0))
                    );
                    data.set_amount_to_capture(std::move(to_capture));
                    DTO::CurrencyValue currency{};
                    currency.set_value(field_string(inner, "currency").value_or(""));
                    data.set_currency(std::move(currency));
                    if (const auto method = field_string(inner, "capture_method")) {
                        DTO::CaptureMethodValue value{};
                        value.set_value(*method);
                        data.set_capture_method(std::move(value));
                    }
                    result.set_capture(data);
                    break;
                }
            case RelayKind::IncrementalAuthorization:
                {
                    DTO::RelayIncrementalAuthorizationRequestData data{};
                    DTO::MinorUnitValue total{};
                    total.set_value(
                        std::to_string(field_number(inner, "total_amount").value_or(0))
                    );
                    data.set_total_amount(std::move(total));
                    DTO::MinorUnitValue additional{};
                    additional.set_value(
                        std::to_string(field_number(inner, "additional_amount").value_or(0))
                    );
                    data.set_additional_amount(std::move(additional));
                    DTO::CurrencyValue currency{};
                    currency.set_value(field_string(inner, "currency").value_or(""));
                    data.set_currency(std::move(currency));
                    result.set_incremental_authorization(data);
                    break;
                }
            case RelayKind::Void:
                {
                    DTO::RelayVoidRequestData data{};
                    if (const auto parsed = field_number(inner, "amount")) {
                        DTO::MinorUnitValue amount{};
                        amount.set_value(std::to_string(*parsed));
                        data.set_amount(std::move(amount));
                    }
                    if (const auto currency = field_string(inner, "currency")) {
                        DTO::CurrencyValue value{};
                        value.set_value(*currency);
                        data.set_currency(std::move(value));
                    }
                    data.set_cancellation_reason(field_string(inner, "cancellation_reason"));
                    result.set_void(data);
                    break;
                }
            case RelayKind::UnreferencedRefund:
                {
                    DTO::RelayUnreferencedRefundData data{};
                    DTO::MinorUnitValue amount{};
                    amount.set_value(std::to_string(field_number(inner, "amount").value_or(0)));
                    data.set_amount(std::move(amount));
                    DTO::CurrencyValue currency{};
                    currency.set_value(field_string(inner, "currency").value_or(""));
                    data.set_currency(std::move(currency));
                    data.set_customer_id(field_string(inner, "customer_id"));
                    if (const auto method = field_value(inner, "recipient_payment_method_data")) {
                        DTO::RecipientPaymentMethodDataValue value{};
                        value.set_value(serde::Ser::encode_json(*method));
                        data.set_recipient_payment_method_data(std::move(value));
                    }
                    result.set_unreferenced_refund(data);
                    break;
                }
            default:
                return std::nullopt;
        }
        return result;
    }

    /**
     * @brief Rebuilds the typed response data from the persisted
     * "Kind|<request body>" payload on retrieve (RelayResponse::from).
     */
    [[nodiscard]] std::optional<::app::dto::relay::RelayResponseData>
    reconstruct_relay_data(RelayKind kind, std::string_view payload) const
    {
        const auto separator = payload.find('|');
        if (separator == std::string_view::npos) {
            return std::nullopt;
        }
        auto document =
            serde::Ser::decode_generic("application/json", payload.substr(separator + 1));
        if (!document) {
            return std::nullopt;
        }
        const auto data = field_value(*document, "data");
        if (!data) {
            return std::nullopt;
        }
        const auto inner = field_value(*data, relay_data_key(kind));
        if (!inner) {
            return std::nullopt;
        }
        return tagged_relay_data(kind, *inner);
    }

    // ───────────────────────── Config / connector helpers ─────────────────────────

    [[nodiscard]] std::string now_string() const
    {
        const std::int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
                                     std::chrono::system_clock::now().time_since_epoch()
        )
                                     .count();
        return std::to_string(now);
    }

    /** @brief Local v7-shaped UUID substitute: time-ordered prefix plus IdGenerator entropy. */
    [[nodiscard]] std::string uuid_v7() const
    {
        std::string id = m_context.ids().uuid_v4();
        const auto millis =
            static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                           std::chrono::system_clock::now().time_since_epoch()
            )
                                           .count());
        constexpr std::string_view digits{"0123456789abcdef"};
        std::uint64_t value = millis;
        for (int index = 11; index >= 0; --index) {
            const std::size_t position =
                index < 8 ? static_cast<std::size_t>(index) : static_cast<std::size_t>(index + 1);
            id[position] = digits[static_cast<std::size_t>(value & 0xFULL)];
            value >>= 4;
        }
        id[14] = '7';
        return id;
    }

    [[nodiscard]] bool feature_enabled(std::string_view flag) const
    {
        const auto value = m_context.config_store().get_value("relay_features", flag);
        return value.has_value() && (*value == "true" || *value == "1");
    }

    /** @brief Local stand-in for RelayConnectors::from_connector_name support checks. */
    [[nodiscard]] bool connector_supported(std::string_view connector_name) const
    {
        const auto value = m_context.config_store().get_value("relay_connectors", connector_name);
        return !value.has_value() || (*value != "unsupported" && *value != "false");
    }

    /** @brief Local stand-in for dimension_state::Dimensions min/max payout limits. */
    [[nodiscard]] std::pair<std::int64_t, std::int64_t>
    payout_bounds(std::string_view connector_id) const
    {
        std::int64_t minimum{0};
        std::int64_t maximum{std::numeric_limits<std::int64_t>::max()};
        const auto low =
            m_context.superposition().resolve("relay_payout_limits", "min_amount", connector_id);
        if (low) {
            if (const auto parsed = parse_amount(low->get_value())) {
                minimum = *parsed;
            }
        }
        const auto high =
            m_context.superposition().resolve("relay_payout_limits", "max_amount", connector_id);
        if (high) {
            if (const auto parsed = parse_amount(high->get_value())) {
                maximum = *parsed;
            }
        }
        return {minimum, maximum};
    }

    [[nodiscard]] std::string event_payload(
        std::string_view relay_id,
        std::string_view merchant_id,
        std::string_view profile_id,
        std::string_view kind,
        std::string_view status
    ) const
    {
        serde::Value::Object fields{};
        fields["relay_id"] = serde::Value{std::string{relay_id}};
        fields["merchant_id"] = serde::Value{std::string{merchant_id}};
        fields["profile_id"] = serde::Value{std::string{profile_id}};
        fields["relay_type"] = serde::Value{std::string{kind}};
        fields["status"] = serde::Value{std::string{status}};
        return serde::Ser::encode_json(serde::Value{std::move(fields)});
    }

    void record_flow(std::string_view flow, std::string_view counter) const
    {
        // Upstream server_wrap declares LockAction::NotApplicable for the relay scope;
        // no lock is acquired. The registry is consulted so a stale relay lock is observable.
        (void)m_context.locks().is_held(std::string{"relay:"} + std::string{flow});
        m_context.logger().record_flow(flow);
        m_context.logger().counter(counter).increment();
    }

    [[nodiscard]] std::shared_ptr<::app::connector::IConnector>
    select_connector(std::string_view connector_id, std::string_view connector_name) const
    {
        auto connector = m_context.connector_router().select(connector_name);
        if (!connector && !connector_id.empty()) {
            connector = m_context.connector_router().select(connector_id);
        }
        return connector;
    }

    [[nodiscard]] ::app::connector::ConnectorResponse dispatch(
        RelayKind kind,
        ::app::connector::IConnector& connector,
        const ::app::connector::ConnectorRequest& request
    ) const
    {
        switch (kind) {
            case RelayKind::Refund:
            case RelayKind::UnreferencedRefund:
                return connector.refund(request);
            case RelayKind::Capture:
                return connector.capture(request);
            case RelayKind::Void:
                return connector.cancel(request);
            case RelayKind::IncrementalAuthorization:
                // IConnector has no incremental-authorization method; authorize is the
                // deterministic stand-in for the upstream incremental connector call.
                return connector.authorize(request);
            default:
                return {};
        }
    }

    [[nodiscard]] std::string_view
    connector_name_hint(const ::app::store::relay_records::RelayRecord& record) const
    {
        // The relay table has no connector-name column; the merchant id carries the
        // connector account id, which the router accepts for deterministic selection.
        return record.get_merchant_id();
    }

    // ───────────────────────── Webhook pipeline helpers ─────────────────────────

    /**
     * @brief Local port of validate_webhook_connector_account
     * (incoming.rs:3557-3595): the relay path id is an MCA id; a name form is
     * accepted as well. A miss is Unauthorized, never NotFound.
     */
    [[nodiscard]] WebhookAccountCheck validate_webhook_connector_account(
        std::string_view merchant_id,
        std::string_view connector_id
    ) const
    {
        WebhookAccountCheck result{};
        const auto mca = m_context.repository().find_merchant_connector_by_string(connector_id);
        if (mca && (mca->get_merchant_id().empty() ||
                    mca->get_merchant_id() == std::string{merchant_id})) {
            result.connector_name = mca->get_connector_name().empty() ? std::string{connector_id}
                                                                      : mca->get_connector_name();
            result.profile_id = mca->get_profile_id();
            return result;
        }
        for (const auto& record: m_context.repository().list_merchant_connectors()) {
            if (record.get_merchant_id() != std::string{merchant_id}) {
                continue;
            }
            if (record.get_merchant_connector_id() != std::string{connector_id} &&
                record.get_connector_name() != std::string{connector_id}) {
                continue;
            }
            result.connector_name = record.get_connector_name().empty()
                                        ? std::string{connector_id}
                                        : record.get_connector_name();
            result.profile_id = record.get_profile_id();
            return result;
        }
        if (auto selected = m_context.connector_router().select_checked(std::string{merchant_id});
            selected.has_value() && *selected) {
            result.connector_name = std::string{connector_id};
            return result;
        }
        result.failure = make_error(core::error::AppErrorKind::UNAUTHORIZED, "Unauthorized");
        return result;
    }

    /**
     * @brief Local port of IncomingWebhookVerifier::verify plus the
     * mandatory-verification policy (incoming.rs:234-263, :558-584). With no
     * configured verifier, upstream treats the source as verified.
     */
    [[nodiscard]] WebhookSourceCheck verify_webhook_source(
        std::string_view signature,
        std::string_view body,
        std::string_view merchant_id,
        std::string_view connector_id,
        std::string_view connector_name
    ) const
    {
        WebhookSourceCheck result{};
        if (!signature.empty()) {
            std::string secret = std::string{"hyperswitch-webhook-secret"};
            if (const auto stored =
                    m_context.key_store().get("webhook_signature", std::string{connector_id})) {
                secret = stored->value();
            }
            const ::app::crypto::SecretString secret_value{secret};
            if (!::app::crypto::KeyedHasher{secret_value.value()}
                     .verify(std::string{body}, signature)) {
                result.failure = make_error(
                    core::error::AppErrorKind::FORBIDDEN,
                    "Webhook authentication failed"
                );
                result.method = "hmac_failed";
                return result;
            }
            result.verified = true;
            result.method = "hmac";
        }
        if (auto connector = select_connector(connector_id, connector_name); connector) {
            ::app::connector::ConnectorRequest request{};
            request.set_operation("webhook_source_verify");
            request.set_merchant_id(std::string{merchant_id});
            request.set_raw_payload(std::string{body});
            const auto response = connector->webhook_source_verify(request);
            if (!response.get_status().empty() || !response.get_reference_id().empty()) {
                result.verified = true;
                result.method = std::string{"connector:"} + std::string{connector->name()};
            }
        }
        if (!result.verified) {
            result.verified = true;
            result.method = "policy_default";
        }
        return result;
    }

    [[nodiscard]] std::string to_lower(std::string_view value) const
    {
        std::string output{};
        output.reserve(value.size());
        for (const char character: value) {
            output.push_back(
                static_cast<char>(std::tolower(static_cast<unsigned char>(character)))
            );
        }
        return output;
    }

    /**
     * @brief Local connector-decoder substitute: classifies the webhook body
     * into an IncomingWebhookEvent and an ObjectReferenceId.
     */
    [[nodiscard]] DecodedRelayWebhook decode_relay_webhook(std::string_view body) const
    {
        DecodedRelayWebhook result{};
        auto document = serde::Ser::decode_generic("application/json", body);
        if (!document) {
            const std::string lowered = to_lower(body);
            if (lowered.find("setup") != std::string::npos) {
                result.setup = true;
            } else if (
                lowered.find("refund") != std::string::npos &&
                lowered.find("fail") != std::string::npos
            ) {
                result.event = RelayWebhookEvent::RefundFailure;
            } else if (
                lowered.find("refund") != std::string::npos &&
                lowered.find("success") != std::string::npos
            ) {
                result.event = RelayWebhookEvent::RefundSuccess;
            }
            return result;
        }

        for (const auto* key: {"event_type", "type", "event", "eventType"}) {
            if (const auto value = field_string(*document, key); value && !value->empty()) {
                result.raw_event_type = *value;
                break;
            }
        }
        const std::string lowered = to_lower(
            result.raw_event_type.empty() ? body : std::string_view{result.raw_event_type}
        );
        if (lowered.find("setup") != std::string::npos) {
            result.setup = true;
            return result;
        }
        if (lowered.find("refund") == std::string::npos) {
            return result;
        }
        if (lowered.find("fail") != std::string::npos ||
            lowered.find("denied") != std::string::npos ||
            lowered.find("error") != std::string::npos) {
            result.event = RelayWebhookEvent::RefundFailure;
        } else if (
            lowered.find("success") != std::string::npos ||
            lowered.find("succeeded") != std::string::npos ||
            lowered.find("paid") != std::string::npos
        ) {
            result.event = RelayWebhookEvent::RefundSuccess;
        } else {
            return result;
        }

        for (const auto* key: {"refund_id", "refundId"}) {
            if (const auto value = field_string(*document, key); value && !value->empty()) {
                result.object_reference_id = *value;
                return result;
            }
        }
        if (const auto data = field_value(*document, "data")) {
            for (const auto* key:
                 {"refund_id", "refundId", "id", "connector_refund_id", "connectorRefundId"}) {
                if (const auto value = field_string(*data, key); value && !value->empty()) {
                    result.object_reference_id = *value;
                    result.connector_refund_id =
                        std::string_view{key}.find("connector") != std::string_view::npos;
                    return result;
                }
            }
        }
        for (const auto* key: {"connector_refund_id", "connectorRefundId", "id"}) {
            if (const auto value = field_string(*document, key); value && !value->empty()) {
                result.object_reference_id = *value;
                result.connector_refund_id =
                    std::string_view{key}.find("connector") != std::string_view::npos;
                return result;
            }
        }
        return result;
    }

    /** @brief RelayStatus::foreign_try_from (types/transformers.rs:506-519). */
    [[nodiscard]] std::optional<std::string> relay_status_for_event(RelayWebhookEvent event) const
    {
        switch (event) {
            case RelayWebhookEvent::RefundSuccess:
                return std::string{"Success"};
            case RelayWebhookEvent::RefundFailure:
                return std::string{"Failure"};
            default:
                return std::nullopt;
        }
    }

    /**
     * @brief find_relay_by_id / find_relay_by_profile_id_connector_reference_id
     * (incoming.rs:1560-1585). The connector reference id is mirrored into the
     * relay record's refund_id column when a relay response is persisted.
     */
    [[nodiscard]] std::optional<::app::store::relay_records::RelayRecord>
    find_relay_for_webhook(const DecodedRelayWebhook& decoded, std::string_view profile_id) const
    {
        std::optional<::app::store::relay_records::RelayRecord> selected{};
        const auto& id = decoded.object_reference_id;
        std::int64_t numeric{};
        const auto [end, ec] = std::from_chars(id.data(), id.data() + id.size(), numeric);
        if (ec == std::errc{} && end == id.data() + id.size()) {
            selected = m_context.repository().find_relay_record(numeric);
        }
        if (!selected) {
            selected = m_context.repository().find_relay_record_by_string(id);
        }
        if (!selected && decoded.connector_refund_id) {
            for (const auto& record: m_context.repository().list_relay_records()) {
                if (record.get_refund_id() != id) {
                    continue;
                }
                if (!profile_id.empty() && !record.get_profile_id().empty() &&
                    record.get_profile_id() != profile_id) {
                    continue;
                }
                selected = record;
                break;
            }
        }
        return selected;
    }

    [[nodiscard]] bool is_terminal_relay_status(std::string_view status) const
    {
        return status == "Success" || status == "Failure" || status == "success" ||
               status == "failure";
    }

    /**
     * @brief Unverified-webhook gateway force sync. Mirrors the force-sync arm
     * of relay_retrieve (core/relay.rs:890-957) invoked from
     * incoming.rs:1605-1628.
     */
    void sync_relay_status(
        ::app::store::relay_records::RelayRecord& record,
        std::string_view merchant_id,
        std::string_view profile_id,
        bool force_sync
    ) const
    {
        // Upstream: should_call_connector_for_relay_refund_status /
        // should_call_connector_for_relay_capture_status (core/relay.rs:947-957).
        if (!force_sync || is_terminal_relay_status(record.get_status())) {
            return;
        }

        const std::string tracker_payload =
            event_payload(record.get_relay_id(), merchant_id, profile_id, "Refund", "PENDING");
        const std::int64_t tracker_id = m_context.process_trackers().schedule(
            "relay_status_sync",
            tracker_payload,
            now_string()
        );
        auto connector =
            select_connector(record.get_merchant_id(), std::string{connector_name_hint(record)});
        ::app::connector::ConnectorResponse sync_response{};
        if (connector) {
            ::app::connector::ConnectorRequest request{};
            request.set_operation("sync");
            request.set_merchant_id(record.get_merchant_id());
            request.set_resource_id(record.get_payment_id());
            request.set_payment_method_type(record.get_status());
            sync_response = connector->sync(request);
            if (!sync_response.get_status().empty()) {
                record.set_status(sync_response.get_status());
                if (record.get_id() != 0) {
                    (void)m_context.repository().update_relay_record(record);
                }
                if (!record.get_refund_id().empty()) {
                    (void)m_context.refunds().update_refund_status(
                        record.get_refund_id(),
                        sync_response.get_status()
                    );
                }
            }
        }
        (void)m_context.process_trackers().mark_terminal(
            tracker_id,
            sync_response.get_status().empty() ? record.get_status() : sync_response.get_status()
        );
        (void)m_context.events().publish(
            "relay.synced",
            event_payload(
                record.get_relay_id(),
                merchant_id,
                profile_id,
                "sync",
                record.get_status()
            )
        );
    }

    /** @brief IncomingWebhookEventMetadata + WebhookResponseTracker JSON. */
    void publish_relay_webhook_event(
        const WebhookIds& ids,
        const DecodedRelayWebhook& decoded,
        std::string_view tracker,
        std::string_view status,
        const WebhookSourceCheck& source,
        std::string_view relay_id
    ) const
    {
        serde::Value::Object tracker_data{};
        tracker_data["tracker"] = serde::Value{std::string{tracker}};
        tracker_data["relay_id"] = serde::Value{std::string{relay_id}};
        tracker_data["status"] = serde::Value{std::string{status}};
        tracker_data["event_type"] = serde::Value{decoded.raw_event_type};
        tracker_data["source_verified"] =
            serde::Value{std::string{source.verified ? "true" : "false"}};

        serde::Value::Object metadata{};
        metadata["connector"] = serde::Value{ids.connector_id};
        metadata["merchant_id"] = serde::Value{ids.merchant_id};
        metadata["payment_id"] = serde::Value{std::string{}};
        metadata["refund_id"] = serde::Value{decoded.object_reference_id};
        metadata["serialized_request"] = serde::Value{decoded.raw_event_type};
        metadata["webhook_tracker_data"] =
            serde::Value{serde::Ser::encode_json(serde::Value{std::move(tracker_data)})};
        (void)m_context.events().publish(
            "webhook.incoming.relay",
            serde::Ser::encode_json(serde::Value{std::move(metadata)})
        );
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::relay
