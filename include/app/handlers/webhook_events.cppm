module;
export module app_handlers:webhook_events;

import std;
import interfaces;
import core_router;
import core_serde;
import core_error;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_store;
import app_http;
import app_crypto;
import app_metrics;
import app_events;
import app_workflow;
import app_locking;
import app_services;
import app_keystore;

export namespace app::handlers::webhook_events {

class WebhookEventsRouteHandlers
{
public:
    // Upstream: POST /events/profile/list
    // routes/webhook_events.rs:57-103 -> JWTAuth(ProfileWebhookEventRead,
    // allow_connected, allow_platform) -> core list_initial_delivery_attempts.
    // The merchant AND profile are forced from UserFromToken into the constraints;
    // the body never supplies them (routes/webhook_events.rs:76-86).
    void handle_list_initial_webhook_delivery_attempts_with_jwtauth(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        auto identity = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize(identity, "list_initial_webhook_delivery_attempts_with_jwtauth")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto merchant_id = merchant_from_identity(identity);
        if (merchant_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant_id"));
            return;
        }
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto constraints = parse_constraints(body);
        if (!constraints) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid constraints JSON"));
            return;
        }
        // Upstream validates constraints (foreign_try_from) before the profile lookup.
        if (const auto profile_error = validate_profile(identity.profile_id(), merchant_id)) {
            send_error(res, send, *profile_error);
            return;
        }
        // UserFromToken anchoring (routes/webhook_events.rs:80-85): override body profile.
        app::dto::webhook_events::ProfileIdValue anchored;
        anchored.set_value(identity.profile_id());
        constraints->set_profile_id(std::move(anchored));

        auto listed = list_initial_delivery_attempts(merchant_id, identity.profile_id(), *constraints);
        if (!listed) {
            send_error(res, send, std::move(listed.error()));
            return;
        }
        app::dto::webhook_events::TotalEventsResponse response;
        response.set_total_count(listed->total_count);
        response.set_events(std::move(listed->items));

        m_context->metrics().record_flow("WebhookEventInitialDeliveryAttemptList");
        m_context->metrics().counter("WEBHOOK_EVENT_INITIAL_ATTEMPTS_LIST").increment();
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // Upstream: POST /events/{merchant_id}
    // routes/webhook_events.rs:18-55 -> JWTAuthMerchantFromRoute(merchant_id,
    // MerchantWebhookEventRead, allow_connected, allow_platform).
    void handle_list_initial_webhook_delivery_attempts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ids = path_ids(req);
        if (ids.merchant_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant_id"));
            return;
        }
        auto identity = app::auth::AuthContextBuilder{}(req);
        // JWTAuthMerchantFromRoute extracts merchant_id from the path.
        identity.set_merchant_id(ids.merchant_id);
        identity.add_claim("x-merchant-id", ids.merchant_id);
        if (const auto auth_error = authorize(identity, "list_initial_webhook_delivery_attempts")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto constraints = parse_constraints(body);
        if (!constraints) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid constraints JSON"));
            return;
        }
        std::string profile_id = identity.profile_id();
        if (constraints->get_profile_id() && !constraints->get_profile_id()->get_value().empty()) {
            profile_id = constraints->get_profile_id()->get_value();
        }
        // Upstream validates constraints (foreign_try_from) before the profile lookup.
        if (const auto profile_error = validate_profile(profile_id, ids.merchant_id)) {
            send_error(res, send, *profile_error);
            return;
        }

        auto listed = list_initial_delivery_attempts(ids.merchant_id, profile_id, *constraints);
        if (!listed) {
            send_error(res, send, std::move(listed.error()));
            return;
        }
        app::dto::webhook_events::TotalEventsResponse response;
        response.set_total_count(listed->total_count);
        response.set_events(std::move(listed->items));

        m_context->metrics().record_flow("WebhookEventInitialDeliveryAttemptList");
        m_context->metrics().counter("WEBHOOK_EVENT_INITIAL_ATTEMPTS_LIST").increment();
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    // Upstream: GET /events/{merchant_id}/{initial_attempt_id}/attempts
    // routes/webhook_events.rs:105-140 -> list_delivery_attempts (core:261-335).
    void handle_list_webhook_delivery_attempts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ids = path_ids(req);
        if (ids.merchant_id.empty() || ids.event_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant_id or initial_attempt_id"));
            return;
        }
        auto identity = app::auth::AuthContextBuilder{}(req);
        identity.set_merchant_id(ids.merchant_id);
        identity.add_claim("x-merchant-id", ids.merchant_id);
        if (const auto auth_error = authorize(identity, "list_webhook_delivery_attempts")) {
            send_error(res, send, *auth_error);
            return;
        }

        // Port of core::webhooks::webhook_events::list_delivery_attempts.
        // list_events_by_initiator_merchant_id_initial_attempt_id scopes by the
        // unique initial_attempt_id; ownership is verified below for every row.
        std::vector<std::pair<Record, std::string>> chain{};
        for (const auto& record : m_context->repository().list_webhook_events()) {
            const auto payload = payload_of(record, ids.merchant_id);
            if (initial_attempt_id_of(record, payload) != ids.event_id) { continue; }
            if (initiator_of(record, payload) != ids.merchant_id) { continue; }
            chain.emplace_back(record, payload);
        }
        if (chain.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "event not found"));
            return;
        }

        // All rows in a chain share a business profile: one lookup covers the list.
        // A missing profile or a failed lookup yields nullopt => redact every value
        // (core/webhooks/utils.rs:576-588, core:300-318).
        const auto sensitive = sensitive_header_names(profile_id_of(chain.front().second));

        std::vector<app::dto::webhook_events::EventRetrieveResponse> events{};
        events.reserve(chain.size());
        for (const auto& [record, payload] : chain) {
            auto item = to_list_item(record, payload, DeliverySource::ListDeliveryAttempts, identity.profile_id());
            if (!item) {
                send_error(res, send, std::move(item.error()));
                return;
            }
            events.push_back(to_retrieve(*item, record, payload, sensitive));
        }

        m_context->metrics().record_flow("WebhookEventDeliveryAttemptList");
        m_context->metrics().counter("WEBHOOK_EVENT_DELIVERY_ATTEMPTS_LIST").increment();
        send_json(res, send, serde::Ser::encode_json(events), true);
    }

    // Upstream: POST /events/{merchant_id}/{event_id}/retry (v1 only)
    // routes/webhook_events.rs:142-178 -> retry_delivery_attempt (core:337-479).
    void handle_retry_webhook_delivery_attempt(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ids = path_ids(req);
        if (ids.merchant_id.empty() || ids.event_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant_id or event_id"));
            return;
        }
        auto identity = app::auth::AuthContextBuilder{}(req);
        identity.set_merchant_id(ids.merchant_id);
        identity.add_claim("x-merchant-id", ids.merchant_id);
        if (const auto auth_error = authorize(identity, "retry_webhook_delivery_attempt")) {
            send_error(res, send, *auth_error);
            return;
        }
        if (const auto profile_error = validate_profile(identity.profile_id(), ids.merchant_id)) {
            send_error(res, send, *profile_error);
            return;
        }

        const std::string lock_key = "webhook_retry:" + ids.merchant_id + ":" + ids.event_id;
        const auto lock = m_context->locks().acquire(lock_key, std::int64_t{30}, "webhook_events");
        if (!lock) {
            m_context->metrics().counter("WEBHOOK_EVENT_RETRY_LOCK_CONTENTED").increment();
            send_error(res, send, error(core::error::AppErrorKind::DUPLICATE_REQUEST, "retry already in progress"));
            return;
        }

        // find_event_by_event_id + owner check with legacy merchant_id fallback
        // (core:364-371).
        const auto original = m_context->repository().find_webhook_event_by_event_id(ids.event_id);
        const auto original_payload = original ? payload_of(*original, ids.merchant_id) : std::string{};
        if (!original || initiator_of(*original, original_payload) != ids.merchant_id) {
            m_context->locks().release(lock_key);
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "event not found"));
            return;
        }

        // provider / processor / profile resolution (core:373-395).
        const std::string provider_merchant_id = original->get_merchant_id().empty()
            ? ids.merchant_id
            : original->get_merchant_id();
        const std::string stored_processor = payload_field(original_payload, "processor_merchant_id");
        const std::string processor_merchant_id = stored_processor.empty() ? provider_merchant_id : stored_processor;
        const std::string business_profile_id = profile_id_of(original_payload, identity.profile_id());
        if (const auto profile_error = validate_profile(business_profile_id, ids.merchant_id)) {
            m_context->locks().release(lock_key);
            send_error(res, send, *profile_error);
            return;
        }
        const auto sensitive = sensitive_header_names(business_profile_id);

        // generate_event_id + get_idempotent_event_id (core:397-429; utils.rs:229-261).
        const std::string new_event_id = generate_event_id();
        const std::string idempotent_event_id = get_idempotent_event_id(
            object_id_of(original_payload),
            original->get_event_type(),
            "manual_retry"
        );

        // get_idempotent_event_id: a repeated retry returns the event already created.
        for (const auto& candidate : m_context->repository().list_webhook_events(
                 app::store::WebhookEventQuery{}.merchant_id(ids.merchant_id))) {
            const auto payload = payload_of(candidate, ids.merchant_id);
            if (payload_field(payload, "idempotent_event_id") == idempotent_event_id) {
                m_context->locks().release(lock_key);
                m_context->metrics().record_flow("WebhookEventDeliveryRetry");
                m_context->metrics().counter("WEBHOOK_EVENT_DELIVERY_RETRY").increment();
                auto item = to_list_item(candidate, payload, DeliverySource::ListDeliveryAttempts, identity.profile_id());
                if (!item) {
                    send_error(res, send, std::move(item.error()));
                    return;
                }
                send_json(res, send, serde::Ser::encode_json(to_retrieve(*item, candidate, payload, sensitive)), true);
                return;
            }
        }

        app::store::webhook_events::WebhookEventRecord retry;
        retry.set_event_id(new_event_id);
        retry.set_merchant_id(provider_merchant_id);
        retry.set_event_type(original->get_event_type());
        retry.set_event_class(original->get_event_class());
        retry.set_status("initial_attempt");
        retry.set_payload(build_retry_payload(
            new_event_id,
            initial_attempt_id_of(*original, original_payload),
            idempotent_event_id,
            processor_merchant_id,
            provider_merchant_id,
            business_profile_id,
            ids.merchant_id,
            recipient_of(original_payload),
            original_payload
        ));
        retry.set_delivery_attempts(original->get_delivery_attempts() + 1);
        retry.set_created_at(now_value());

        auto inserted = m_context->repository().insert_webhook_event(std::move(retry));
        if (!inserted) {
            m_context->locks().release(lock_key);
            send_error(res, send, inserted.error());
            return;
        }

        // trigger_webhook_and_raise_event (core:431-435): publish the bus event,
        // then drive one deterministic local delivery attempt.
        const std::string bus_event_id = m_context->events().publish(
            "webhook.delivery.retry", inserted->get_payload()
        );
        const std::string endpoint = m_context->config_store()
                                         .get_value("webhook_endpoints", ids.merchant_id)
                                         .value_or("");
        m_context->webhook_delivery().enqueue(bus_event_id, endpoint);
        if (!m_context->webhook_delivery().attempt(bus_event_id)) {
            m_context->webhook_delivery().retry(bus_event_id, app::workflow::RetryPolicy{});
        }

        // Re-fetch the saved event (core:450-478).
        const auto stored = m_context->repository().find_webhook_event_by_event_id(new_event_id);
        if (!stored) {
            m_context->locks().release(lock_key);
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "event not found"));
            return;
        }
        const auto stored_payload = payload_of(*stored, ids.merchant_id);
        m_context->locks().release(lock_key);

        auto item = to_list_item(*stored, stored_payload, DeliverySource::ListDeliveryAttempts, identity.profile_id());
        if (!item) {
            send_error(res, send, std::move(item.error()));
            return;
        }
        m_context->metrics().record_flow("WebhookEventDeliveryRetry");
        m_context->metrics().counter("WEBHOOK_EVENT_DELIVERY_RETRY").increment();
        send_json(res, send, serde::Ser::encode_json(to_retrieve(*item, *stored, stored_payload, sensitive)), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/events"}
            .add_route(
                Route{"/profile/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_initial_webhook_delivery_attempts_with_jwtauth(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_initial_webhook_delivery_attempts_with_jwtauth")
            )
            .add_route(
                Route{"/:merchant_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_initial_webhook_delivery_attempts(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_initial_webhook_delivery_attempts")
            )
            .add_route(
                Route{"/:merchant_id/:event_id/attempts"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_webhook_delivery_attempts(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_webhook_delivery_attempts")
            )
            .add_route(
                Route{"/:merchant_id/:event_id/retry"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retry_webhook_delivery_attempt(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retry_webhook_delivery_attempt")
            );
    }

    explicit WebhookEventsRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    using Record = app::store::webhook_events::WebhookEventRecord;
    using Constraints = app::dto::webhook_events::EventListConstraints;
    using InternalKind = app::dto::webhook_events::EventListConstraintsInternal;

    // Port of api_models::webhook_events::EventListConstraintsInternal.
    struct InternalConstraints
    {
        InternalKind kind{InternalKind::GenericFilter};
        // ObjectIdFilter
        std::string object_id{};
        // EventIdFilter
        std::string event_id{};
        // GenericFilter
        std::optional<std::string> created_after{};
        std::optional<std::string> created_before{};
        std::optional<std::int64_t> limit{};
        std::optional<std::int64_t> offset{};
        std::vector<std::string> event_classes{};
        std::vector<std::string> event_types{};
        std::optional<bool> is_delivered{};
        std::optional<std::string> recipient{};
    };

    enum class DeliverySource
    {
        ListInitialEvents,
        ListDeliveryAttempts,
    };

    struct PathIds
    {
        std::string merchant_id{};
        // Attempts route: upstream initial_attempt_id. Retry route: event_id.
        std::string event_id{};
    };

    struct EventListResult
    {
        std::vector<app::dto::webhook_events::EventListItemResponse> items{};
        std::int64_t total_count{0};
    };

    [[nodiscard]] core::error::AppError error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    // Local typed rules mirror the upstream extractors
    // (routes/webhook_events.rs:46-51, :95-99, :131-136, :169-174). The shared
    // registry carries only the legacy flat permissions; building the rule here
    // keeps allow_connected / allow_platform upstream-identical without editing
    // the shared auth module.
    [[nodiscard]] app::auth::RouteAuthRule auth_rule(std::string_view operation) const
    {
        using app::auth::AppPermission;
        using app::auth::AuthScheme;
        using app::auth::RouteAuthRuleBuilder;

        const AppPermission permission =
            operation == "retry_webhook_delivery_attempt" ? AppPermission::WEBHOOK_WRITE : AppPermission::WEBHOOK_READ;
        return RouteAuthRuleBuilder{}
            .scheme(AuthScheme::JWT)
            .permission(permission)
            .allow_connected(true)
            .allow_platform(true)();
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        const app::auth::AuthContext& identity,
        std::string_view operation
    ) const
    {
        return m_context->authorizer().authorize_route(identity, auth_rule(operation));
    }

    [[nodiscard]] std::string merchant_from_identity(const app::auth::AuthContext& identity) const
    {
        if (!identity.merchant_id().empty()) {
            return identity.merchant_id();
        }
        if (const auto connected = identity.claim(app::auth::kClaimConnectedMerchantId)) {
            return std::string{*connected};
        }
        return {};
    }

    // Local in-process substitute for find_business_profile_by_merchant_id_profile_id
    // / find_business_profile_by_profile_id (core:30-53, :344-353).
    [[nodiscard]] std::optional<core::error::AppError> validate_profile(
        const std::string& profile_id,
        const std::string& merchant_id
    ) const
    {
        if (profile_id.empty()) {
            return std::nullopt;
        }
        const auto stored = m_context->config_store().get_value("business_profile", profile_id);
        if (!stored) {
            m_context->metrics().counter("WEBHOOK_EVENT_PROFILE_UNRESOLVED").increment();
            return std::nullopt;
        }
        if (!merchant_id.empty() && *stored != merchant_id) {
            return error(core::error::AppErrorKind::FORBIDDEN, "business profile does not belong to merchant");
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string event_key(const std::string& merchant_id) const
    {
        const std::string key_id = merchant_id.empty() ? std::string{"v1"} : merchant_id;
        if (const auto secret = m_context->key_store().get("webhook_event", key_id)) {
            return secret->value();
        }
        return std::string{"webhook-event-local-substitute"};
    }

    // Deterministic decrypt of the local enc1 framing; plaintext rows pass through.
    [[nodiscard]] std::string payload_of(const Record& record, const std::string& merchant_id) const
    {
        const std::string& value = record.get_payload();
        constexpr std::string_view marker{"enc1:"};
        if (!value.starts_with(marker)) {
            return value;
        }
        const std::string_view encoded = std::string_view{value}.substr(marker.size());
        std::string raw{};
        raw.reserve(encoded.size() / 2U);
        const auto nibble = [](char ch) -> int
        {
            if (ch >= '0' && ch <= '9') { return ch - '0'; }
            if (ch >= 'a' && ch <= 'f') { return ch - 'a' + 10; }
            if (ch >= 'A' && ch <= 'F') { return ch - 'A' + 10; }
            return -1;
        };
        for (std::size_t i = 0; i + 1 < encoded.size(); i += 2) {
            const int high = nibble(encoded[i]);
            const int low = nibble(encoded[i + 1]);
            if (high < 0 || low < 0) {
                return value;
            }
            raw.push_back(static_cast<char>((high << 4) | low));
        }
        return ::app::crypto::AesWrap{}.decrypt(raw, event_key(merchant_id));
    }

    [[nodiscard]] std::string payload_field(std::string_view payload, std::string_view key) const
    {
        const std::string needle = std::format("\"{}\":\"", key);
        const auto pos = payload.find(needle);
        if (pos == std::string_view::npos) {
            return {};
        }
        const std::size_t start = pos + needle.size();
        const std::size_t end = payload.find('"', start);
        if (end == std::string_view::npos) {
            return {};
        }
        return std::string{payload.substr(start, end - start)};
    }

    // Boolean fields are stored unquoted in JSON.
    [[nodiscard]] std::optional<bool> payload_bool(std::string_view payload, std::string_view key) const
    {
        const std::string needle = std::format("\"{}\":", key);
        const auto pos = payload.find(needle);
        if (pos == std::string_view::npos) {
            return std::nullopt;
        }
        std::size_t cursor = pos + needle.size();
        while (cursor < payload.size() && std::isspace(static_cast<unsigned char>(payload[cursor]))) { ++cursor; }
        if (payload.substr(cursor, 4) == "true") { return true; }
        if (payload.substr(cursor, 5) == "false") { return false; }
        return std::nullopt;
    }

    [[nodiscard]] std::string json_escape(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char ch : value) {
            switch (ch) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(ch); break;
            }
        }
        return out;
    }

    [[nodiscard]] std::string initial_attempt_id_of(const Record& record, std::string_view payload) const
    {
        const std::string stored = payload_field(payload, "initial_attempt_id");
        return stored.empty() ? record.get_event_id() : stored;
    }

    [[nodiscard]] std::string initiator_of(const Record& record, std::string_view payload) const
    {
        const std::string stored = payload_field(payload, "initiator_merchant_id");
        return stored.empty() ? record.get_merchant_id() : stored;
    }

    [[nodiscard]] std::string profile_id_of(std::string_view payload, std::string_view fallback = {}) const
    {
        const std::string stored = payload_field(payload, "business_profile_id");
        if (!stored.empty()) { return stored; }
        const std::string alt = payload_field(payload, "profile_id");
        return alt.empty() ? std::string{fallback} : alt;
    }

    [[nodiscard]] std::string object_id_of(std::string_view payload) const
    {
        return payload_field(payload, "primary_object_id");
    }

    [[nodiscard]] std::string recipient_of(std::string_view payload) const
    {
        const std::string stored = payload_field(payload, "recipient");
        return stored.empty() ? std::string{"merchant"} : stored;
    }

    // Upstream resolve_delivery_success (event.rs:102-107):
    //   ListInitialEvents     -> is_overall_delivery_successful
    //   ListDeliveryAttempts  -> Some(is_webhook_notified)
    [[nodiscard]] std::optional<bool> delivery_success_of(
        const Record& record,
        std::string_view payload,
        DeliverySource source
    ) const
    {
        if (source == DeliverySource::ListInitialEvents) {
            const auto overall = payload_bool(payload, "is_overall_delivery_successful");
            if (overall) { return overall; }
        } else {
            const auto notified = payload_bool(payload, "is_webhook_notified");
            if (notified) { return notified; }
        }
        // Local fallback for rows seeded without the upstream domain fields.
        const auto legacy = payload_bool(payload, "is_delivered");
        if (legacy) { return legacy; }
        const auto& status = record.get_status();
        if (status == "DELIVERED" || status == "SUCCESS" || status == "Success") { return true; }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::int64_t> parse_seconds(std::string_view value) const
    {
        if (value.empty()) {
            return std::nullopt;
        }
        try {
            std::size_t consumed{0};
            const auto parsed = std::stoll(std::string{value}, &consumed);
            if (consumed == value.size()) {
                return parsed;
            }
        } catch (...) {
        }
        return std::nullopt;
    }

    // Upstream bounds are PrimitiveDateTime deserialized from RFC3339/ISO8601
    // (common_utils::custom_serde::iso8601). Accept both ISO strings and the
    // numeric seconds used by locally seeded records.
    [[nodiscard]] std::optional<std::int64_t> parse_timestamp(std::string_view value) const
    {
        if (const auto numeric = parse_seconds(value)) { return numeric; }
        if (value.size() < 19) { return std::nullopt; }
        const auto number = [value](std::size_t pos, std::size_t length) -> std::optional<int>
        {
            int result{0};
            for (std::size_t i{0}; i < length; ++i) {
                if (pos + i >= value.size() || !std::isdigit(static_cast<unsigned char>(value[pos + i]))) {
                    return std::nullopt;
                }
                result = result * 10 + (value[pos + i] - '0');
            }
            return result;
        };
        const auto year = number(0, 4);
        const auto month = number(5, 2);
        const auto day = number(8, 2);
        const auto hour = number(11, 2);
        const auto minute = number(14, 2);
        const auto second = number(17, 2);
        if (!year || !month || !day || !hour || !minute || !second) { return std::nullopt; }
        if (value[4] != '-' || value[7] != '-' || (value[10] != 'T' && value[10] != ' ') ||
            value[13] != ':' || value[16] != ':') {
            return std::nullopt;
        }

        // Days since 1970-01-01 (Howard Hinnant's civil-date algorithm).
        int y = *year - (*month <= 2 ? 1 : 0);
        const int era = (y >= 0 ? y : y - 399) / 400;
        const unsigned yoe = static_cast<unsigned>(y - era * 400);
        const unsigned doy = (153 * static_cast<unsigned>(*month + (*month > 2 ? -3 : 9)) + 2) / 5
                             + static_cast<unsigned>(*day) - 1;
        const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
        const std::int64_t days = static_cast<std::int64_t>(era) * 146097 + static_cast<std::int64_t>(doe) - 719468;
        std::int64_t seconds = days * 86400 + static_cast<std::int64_t>(*hour) * 3600
                               + static_cast<std::int64_t>(*minute) * 60 + *second;

        // Optional timezone offset (Z or ±HH:MM).
        const auto zone = value.find_last_of("Zz+-");
        if (zone != std::string_view::npos && zone > 10 && value[zone] != 'Z' && value[zone] != 'z') {
            const auto offset_hour = number(zone + 1, 2);
            const auto offset_minute = number(zone + 4, 2);
            if (offset_hour && offset_minute) {
                const std::int64_t offset = static_cast<std::int64_t>(*offset_hour) * 3600 + *offset_minute * 60;
                seconds += value[zone] == '-' ? offset : -offset;
            }
        }
        return seconds;
    }

    // Port of EventListConstraintsInternal::foreign_try_from
    // (transformers.rs:2224-2269).
    [[nodiscard]] std::expected<InternalConstraints, core::error::AppError> to_internal(
        const Constraints& constraints
    ) const
    {
        const bool has_object = constraints.get_object_id().has_value();
        const bool has_event = constraints.get_event_id().has_value();
        const bool has_generic =
            constraints.get_created_after().has_value() || constraints.get_created_before().has_value() ||
            constraints.get_limit().has_value() || constraints.get_offset().has_value() ||
            constraints.get_event_classes().has_value() || constraints.get_event_types().has_value();

        if ((has_object || has_event) && has_generic) {
            return std::unexpected{error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Either `object_id` alone, or `event_id` alone, or one or more of `created_after`, "
                "`created_before`, `limit`, `offset`, `event_classes` and `event_types` must be specified"
            )};
        }
        if (has_object && has_event) {
            return std::unexpected{error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Cannot specify both `object_id` and `event_id`. Please provide only one."
            )};
        }

        InternalConstraints out{};
        if (has_object) {
            out.kind = InternalKind::ObjectIdFilter;
            out.object_id = *constraints.get_object_id();
            if (constraints.get_recipient()) {
                out.recipient = constraints.get_recipient()->get_value();
            }
            return out;
        }
        if (has_event) {
            out.kind = InternalKind::EventIdFilter;
            out.event_id = *constraints.get_event_id();
            return out;
        }

        out.kind = InternalKind::GenericFilter;
        if (constraints.get_created_after()) { out.created_after = constraints.get_created_after()->get_value(); }
        if (constraints.get_created_before()) { out.created_before = constraints.get_created_before()->get_value(); }
        if (constraints.get_limit()) { out.limit = static_cast<std::int64_t>(*constraints.get_limit()); }
        if (constraints.get_offset()) { out.offset = static_cast<std::int64_t>(*constraints.get_offset()); }
        if (constraints.get_event_classes()) {
            for (const auto& value : *constraints.get_event_classes()) { out.event_classes.push_back(value.get_value()); }
        }
        if (constraints.get_event_types()) {
            for (const auto& value : *constraints.get_event_types()) { out.event_types.push_back(value.get_value()); }
        }
        if (constraints.get_is_delivered()) { out.is_delivered = *constraints.get_is_delivered(); }
        if (constraints.get_recipient()) { out.recipient = constraints.get_recipient()->get_value(); }
        return out;
    }

    // EventClass::event_types() (common_enums/src/enums.rs:2065-2107), snake_case wire values.
    [[nodiscard]] std::vector<std::string> event_types_of_class(std::string_view event_class) const
    {
        if (event_class == "payments") {
            return {
                "payment_succeeded", "payment_failed", "payment_processing", "payment_cancelled",
                "payment_cancelled_post_capture", "payment_authorized", "payment_captured",
                "payment_expired", "action_required", "surcharge_payment_succeeded",
            };
        }
        if (event_class == "refunds") {
            return {"refund_succeeded", "refund_failed", "surcharge_refund_succeeded", "refund_review"};
        }
        if (event_class == "disputes") {
            return {
                "dispute_opened", "dispute_expired", "dispute_accepted", "dispute_cancelled",
                "dispute_challenged", "dispute_won", "dispute_lost",
            };
        }
        if (event_class == "mandates") {
            return {"mandate_active", "mandate_revoked"};
        }
        if (event_class == "payouts") {
            return {
                "payout_success", "payout_failed", "payout_initiated", "payout_processing",
                "payout_cancelled", "payout_expired", "payout_reversed", "payout_not_permitted",
            };
        }
        if (event_class == "subscriptions") {
            return {"invoice_paid"};
        }
        if (event_class == "revenue_recovery") {
            return {"recovery_retry_scheduled", "recovery_limits_exhausted"};
        }
        return {};
    }

    [[nodiscard]] static bool contains(const std::vector<std::string>& values, const std::string& needle)
    {
        return std::find(values.begin(), values.end(), needle) != values.end();
    }

    [[nodiscard]] static bool disjoint(const std::vector<std::string>& lhs, const std::vector<std::string>& rhs)
    {
        for (const auto& value : lhs) {
            if (contains(rhs, value)) { return false; }
        }
        return true;
    }

    // Port of finalize_event_types (core:481-524).
    [[nodiscard]] std::expected<std::vector<std::string>, core::error::AppError> finalize_event_types(
        const std::vector<std::string>& event_classes,
        std::vector<std::string> event_types
    ) const
    {
        std::vector<std::string> possible{};
        for (const auto& event_class : event_classes) {
            for (const auto& event_type : event_types_of_class(event_class)) {
                if (!contains(possible, event_type)) { possible.push_back(event_type); }
            }
        }

        if (event_types.empty()) {
            return possible;
        }

        // Extend with each class whose event types are disjoint from the request.
        for (const auto& event_class : event_classes) {
            const auto valid = event_types_of_class(event_class);
            if (disjoint(event_types, valid)) {
                for (const auto& event_type : valid) {
                    if (!contains(event_types, event_type)) { event_types.push_back(event_type); }
                }
            }
        }

        for (const auto& event_type : event_types) {
            if (!contains(possible, event_type)) {
                return std::unexpected{error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "`event_types` must be a subset of `event_classes`"
                )};
            }
        }
        return event_types;
    }

    [[nodiscard]] std::string now_value() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
    }

    // Port of list_initial_delivery_attempts (core:20-259).
    [[nodiscard]] std::expected<EventListResult, core::error::AppError> list_initial_delivery_attempts(
        const std::string& merchant_id,
        const std::string& profile_id,
        const Constraints& constraints
    ) const
    {
        auto internal = to_internal(constraints);
        if (!internal) {
            return std::unexpected{std::move(internal.error())};
        }

        EventListResult result{};
        switch (internal->kind) {
        case InternalKind::ObjectIdFilter: {
            // list_initial_events_by_initiator_merchant_id_primary_object_id
            // (core:60-85): no limit/offset, total_count = events.len().
            std::vector<std::pair<Record, std::string>> matched{};
            for (const auto& record : m_context->repository().list_webhook_events(
                     app::store::WebhookEventQuery{}.merchant_id(merchant_id))) {
                const auto payload = payload_of(record, merchant_id);
                if (!is_initial_attempt(record, payload)) { continue; }
                if (initiator_of(record, payload) != merchant_id) { continue; }
                if (object_id_of(payload) != internal->object_id) { continue; }
                if (!profile_id.empty() && profile_id_of(payload, profile_id) != profile_id) { continue; }
                if (internal->recipient && !recipient_matches(recipient_of(payload), *internal->recipient)) { continue; }
                matched.emplace_back(record, payload);
            }
            // Upstream orders by created_at DESC.
            std::stable_sort(
                matched.begin(), matched.end(),
                [](const auto& lhs, const auto& rhs)
                {
                    return lhs.first.get_created_at() > rhs.first.get_created_at();
                }
            );
            result.total_count = static_cast<std::int64_t>(matched.size());
            for (const auto& [record, payload] : matched) {
                auto item = to_list_item(record, payload, DeliverySource::ListInitialEvents, profile_id);
                if (!item) { return std::unexpected{std::move(item.error())}; }
                result.items.push_back(std::move(*item));
            }
            return result;
        }
        case InternalKind::EventIdFilter: {
            // find_event_by_event_id + initiator/profile/initial-attempt scoping
            // (core:87-116): total_count is 0 or 1.
            const auto found = m_context->repository().find_webhook_event_by_event_id(internal->event_id);
            if (!found) { return result; }
            const auto payload = payload_of(*found, merchant_id);
            const bool matches_initiator = initiator_of(*found, payload) == merchant_id;
            const bool matches_profile = profile_id.empty() || profile_id_of(payload, profile_id) == profile_id;
            if (!matches_initiator || !matches_profile ||
                initial_attempt_id_of(*found, payload) != found->get_event_id()) {
                return result;
            }
            auto item = to_list_item(*found, payload, DeliverySource::ListInitialEvents, profile_id);
            if (!item) { return std::unexpected{std::move(item.error())}; }
            result.items.push_back(std::move(*item));
            result.total_count = 1;
            return result;
        }
        case InternalKind::GenericFilter:
            return list_generic(merchant_id, profile_id, *internal);
        }
        return result;
    }

    [[nodiscard]] bool is_initial_attempt(const Record& record, std::string_view payload) const
    {
        return initial_attempt_id_of(record, payload) == record.get_event_id();
    }

    [[nodiscard]] bool recipient_matches(const std::string& stored, const std::string& requested) const
    {
        if (requested == "merchant") {
            // NULL/empty recipient is treated as Merchant for backward compatibility.
            return stored.empty() || stored == "merchant";
        }
        return stored == requested;
    }

    [[nodiscard]] std::expected<EventListResult, core::error::AppError> list_generic(
        const std::string& merchant_id,
        const std::string& profile_id,
        const InternalConstraints& internal
    ) const
    {
        const std::int64_t now = parse_seconds(now_value()).value_or(0);
        const std::int64_t begin_time = (now / 86400 - 90) * 86400; // midnight UTC, 90 days ago

        // limit validation (core:123-131).
        std::int64_t limit{100};
        if (internal.limit) {
            if (*internal.limit > 100) {
                return std::unexpected{error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "`limit` must be a number less than 100"
                )};
            }
            limit = *internal.limit;
        }
        std::optional<std::int64_t> offset{};
        if (internal.offset && *internal.offset > 0) { offset = *internal.offset; }

        std::vector<std::string> event_types = internal.event_types;
        if (!internal.event_classes.empty()) {
            auto resolved = finalize_event_types(internal.event_classes, event_types);
            if (!resolved) { return std::unexpected{std::move(resolved.error())}; }
            event_types = std::move(*resolved);
        }

        // created_after/created_before validation (core:143-173).
        if (internal.created_after && internal.created_before &&
            *internal.created_after > *internal.created_before) {
            return std::unexpected{error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "The `created_after` timestamp must be an earlier timestamp compared to the `created_before` timestamp"
            )};
        }
        std::int64_t created_after = begin_time;
        if (internal.created_after) {
            const auto parsed = parse_timestamp(*internal.created_after);
            if (!parsed || *parsed < begin_time) {
                return std::unexpected{error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "`created_after` must be a timestamp within the past 90 days."
                )};
            }
            created_after = *parsed;
        }
        std::int64_t created_before = now;
        if (internal.created_before) {
            const auto parsed = parse_timestamp(*internal.created_before);
            if (!parsed || *parsed < begin_time) {
                return std::unexpected{error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "`created_before` must be a timestamp within the past 90 days."
                )};
            }
            created_before = *parsed;
        }

        // The profile variant filters by business_profile_id; the merchant variant
        // filters by initiator_merchant_id with the legacy merchant_id fallback.
        std::vector<std::pair<Record, std::string>> all{};
        for (const auto& record : m_context->repository().list_webhook_events(
                 app::store::WebhookEventQuery{}.merchant_id(merchant_id))) {
            const auto payload = payload_of(record, merchant_id);
            if (!is_initial_attempt(record, payload)) { continue; }
            if (!profile_id.empty()) {
                if (profile_id_of(payload, profile_id) != profile_id) { continue; }
            } else if (initiator_of(record, payload) != merchant_id) {
                continue;
            }
            const auto created = parse_timestamp(record.get_created_at());
            if (created && (*created < created_after || *created > created_before)) { continue; }
            if (!event_types.empty() && !contains(event_types, record.get_event_type())) { continue; }
            if (internal.is_delivered &&
                delivery_success_of(record, payload, DeliverySource::ListInitialEvents) != *internal.is_delivered) {
                continue;
            }
            if (internal.recipient && !recipient_matches(recipient_of(payload), *internal.recipient)) { continue; }
            all.emplace_back(record, payload);
        }

        // created_at DESC ordering (upstream `.order(created_at.desc())`).
        std::stable_sort(
            all.begin(), all.end(),
            [](const auto& lhs, const auto& rhs)
            {
                return lhs.first.get_created_at() > rhs.first.get_created_at();
            }
        );

        EventListResult result{};
        // Separate COUNT query: counts every matching row, ignoring limit/offset.
        result.total_count = static_cast<std::int64_t>(all.size());

        // limit/offset applied after ordering.
        std::size_t skipped{0};
        for (const auto& [record, payload] : all) {
            if (offset && skipped++ < static_cast<std::size_t>(*offset)) { continue; }
            if (result.items.size() >= static_cast<std::size_t>(limit)) { break; }
            auto item = to_list_item(record, payload, DeliverySource::ListInitialEvents, profile_id);
            if (!item) { return std::unexpected{std::move(item.error())}; }
            result.items.push_back(std::move(*item));
        }
        return result;
    }

    [[nodiscard]] std::expected<app::dto::webhook_events::EventListItemResponse, core::error::AppError> to_list_item(
        const Record& record,
        std::string_view payload,
        DeliverySource source,
        std::string_view fallback_profile_id
    ) const
    {
        // Upstream requires merchant_id, business_profile_id and initial_attempt_id
        // to be populated (transformers.rs:2285-2300).
        if (record.get_merchant_id().empty()) {
            return std::unexpected{error(core::error::AppErrorKind::INTERNAL_ERROR, "missing merchant_id")};
        }
        const std::string profile = profile_id_of(payload, fallback_profile_id);
        if (profile.empty()) {
            return std::unexpected{error(core::error::AppErrorKind::INTERNAL_ERROR, "missing business_profile_id")};
        }

        app::dto::webhook_events::EventListItemResponse item;
        item.set_event_id(record.get_event_id());

        app::dto::webhook_events::MerchantIdValue merchant;
        merchant.set_value(record.get_merchant_id());
        item.set_merchant_id(std::move(merchant));

        app::dto::webhook_events::ProfileIdValue value;
        value.set_value(profile);
        item.set_profile_id(std::move(value));

        item.set_object_id(object_id_of(payload));

        app::dto::webhook_events::EventTypeValue type;
        type.set_value(record.get_event_type());
        item.set_event_type(std::move(type));

        app::dto::webhook_events::EventClassValue event_class;
        event_class.set_value(record.get_event_class());
        item.set_event_class(std::move(event_class));

        item.set_is_delivery_successful(delivery_success_of(record, payload, source));

        app::dto::webhook_events::PrimitiveDateTimeValue created;
        created.set_value(record.get_created_at());
        item.set_created(std::move(created));

        item.set_initial_attempt_id(initial_attempt_id_of(record, payload));

        const std::string processor = payload_field(payload, "processor_merchant_id");
        if (!processor.empty()) {
            app::dto::webhook_events::MerchantIdValue processor_value;
            processor_value.set_value(processor);
            item.set_processor_merchant_id(std::move(processor_value));
        }
        return item;
    }

    // Redact header values exactly like core/webhooks/utils.rs:576-588.
    // nullopt sensitive names => every value is redacted.
    [[nodiscard]] app::dto::webhook_events::EventRetrieveResponse to_retrieve(
        const app::dto::webhook_events::EventListItemResponse& item,
        const Record& record,
        std::string_view payload,
        const std::optional<std::vector<std::string>>& sensitive
    ) const
    {
        app::dto::webhook_events::EventRetrieveResponse response;
        response.set_event_information(item);

        app::dto::webhook_events::SecretStringValue body;
        body.set_value(std::string{payload});
        app::dto::webhook_events::OutgoingWebhookRequestContent request;
        request.set_body(std::move(body));

        std::vector<std::pair<std::string, app::dto::webhook_events::SecretStringValue>> headers{};
        for (const auto& [name, value] : parse_headers(payload)) {
            const std::string lowered = to_lower(name);
            const bool redact = !sensitive || contains(*sensitive, lowered);
            app::dto::webhook_events::SecretStringValue masked;
            masked.set_value(redact ? std::string{"*** ***"} : std::string{value});
            headers.emplace_back(name, std::move(masked));
        }
        request.set_headers(std::move(headers));
        response.set_request(std::move(request));

        app::dto::webhook_events::OutgoingWebhookResponseContent outgoing;
        app::dto::webhook_events::SecretStringValue response_body;
        response_body.set_value(record.get_status());
        outgoing.set_body(std::move(response_body));
        response.set_response(std::move(outgoing));

        app::dto::webhook_events::WebhookDeliveryAttemptValue attempt;
        const std::string stored_attempt = payload_field(payload, "delivery_attempt");
        attempt.set_value(stored_attempt.empty() ? std::string{"initial_attempt"} : stored_attempt);
        response.set_delivery_attempt(std::move(attempt));
        return response;
    }

    // Parse "headers":[["name","value"], ...] from the persisted request content.
    [[nodiscard]] std::vector<std::pair<std::string, std::string>> parse_headers(std::string_view payload) const
    {
        std::vector<std::pair<std::string, std::string>> headers{};
        std::size_t cursor = payload.find("\"headers\"");
        if (cursor == std::string_view::npos) { return headers; }
        cursor += 9;
        while (cursor < payload.size() && payload[cursor] != '[') { ++cursor; }
        if (cursor >= payload.size()) { return headers; }
        ++cursor;
        while (cursor < payload.size()) {
            while (cursor < payload.size() && (std::isspace(static_cast<unsigned char>(payload[cursor])) || payload[cursor] == ',')) { ++cursor; }
            if (cursor >= payload.size() || payload[cursor] == ']') { break; }
            if (payload[cursor] != '[') { break; }
            ++cursor;
            const auto name = parse_quoted(payload, cursor);
            while (cursor < payload.size() && (std::isspace(static_cast<unsigned char>(payload[cursor])) || payload[cursor] == ',')) { ++cursor; }
            const auto value = parse_quoted(payload, cursor);
            if (!name && !value) { break; }
            headers.emplace_back(name.value_or(""), value.value_or(""));
            while (cursor < payload.size() && payload[cursor] != ']' && payload[cursor] != ',') { ++cursor; }
            if (cursor < payload.size() && payload[cursor] == ']') { ++cursor; }
        }
        return headers;
    }

    [[nodiscard]] std::optional<std::string> parse_quoted(std::string_view text, std::size_t& cursor) const
    {
        while (cursor < text.size() && std::isspace(static_cast<unsigned char>(text[cursor]))) { ++cursor; }
        if (cursor >= text.size() || text[cursor] != '"') { return std::nullopt; }
        ++cursor;
        std::string out{};
        while (cursor < text.size() && text[cursor] != '"') {
            if (text[cursor] == '\\' && cursor + 1 < text.size()) { ++cursor; }
            out.push_back(text[cursor++]);
        }
        if (cursor < text.size()) { ++cursor; }
        return out;
    }

    [[nodiscard]] std::string to_lower(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char ch : value) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return out;
    }

    // Local substitute for business_profile.get_sensitive_webhook_header_names().
    // nullopt means the profile could not be resolved: redact every value.
    [[nodiscard]] std::optional<std::vector<std::string>> sensitive_header_names(const std::string& profile_id) const
    {
        if (profile_id.empty()) { return std::nullopt; }
        const auto configured = m_context->config_store().get_value("webhook_sensitive_headers", profile_id);
        if (!configured) { return std::nullopt; }
        std::vector<std::string> names{};
        std::size_t start{0};
        while (start <= configured->size()) {
            const std::size_t comma = configured->find(',', start);
            const std::string_view part = std::string_view{*configured}.substr(
                start, comma == std::string_view::npos ? std::string_view::npos : comma - start
            );
            std::size_t first{0};
            std::size_t last{part.size()};
            while (first < last && std::isspace(static_cast<unsigned char>(part[first]))) { ++first; }
            while (last > first && std::isspace(static_cast<unsigned char>(part[last - 1]))) { --last; }
            if (last > first) { names.emplace_back(to_lower(part.substr(first, last - first))); }
            if (comma == std::string_view::npos) { break; }
            start = comma + 1;
        }
        return names;
    }

    [[nodiscard]] std::optional<app::dto::webhook_events::EventListConstraints> parse_constraints(
        std::string_view body
    ) const
    {
        if (!serde::Ser::decode_generic("application/json", body)) {
            return std::nullopt;
        }
        auto parsed = serde::Ser::deserialize<Constraints>("application/json", body);
        if (!parsed) {
            return std::nullopt;
        }
        return *parsed;
    }

    // ---- upstream id generation (core/webhooks/utils.rs:229-261) --------------

    // generate_event_id(): "evt_" + UUIDv7 simple (32 hex digits).
    [[nodiscard]] std::string generate_event_id() const
    {
        std::string raw = m_context->ids().uuid_v4();
        std::string simple{};
        simple.reserve(32);
        for (const char ch : raw) {
            if (ch != '-') { simple.push_back(ch); }
        }
        if (simple.size() == 32) { simple[12] = '7'; }
        return "evt_" + simple;
    }

    // get_idempotent_event_id(): base64url(SHA256(primary_object_id_event_type))
    // plus an 8-character NanoID suffix for retries.
    [[nodiscard]] std::string get_idempotent_event_id(
        const std::string& primary_object_id,
        const std::string& event_type,
        std::string_view delivery_attempt
    ) const
    {
        const std::string common = primary_object_id + "_" + event_type;
        const auto digest = sha256_bytes(common);
        const std::string encoded = app::crypto::Base64Url{}.encode(
            std::string_view{reinterpret_cast<const char*>(digest.data()), digest.size()}
        );
        if (delivery_attempt == "manual_retry" || delivery_attempt == "automatic_retry") {
            return encoded + "_" + nanoid_suffix(8);
        }
        return encoded;
    }

    [[nodiscard]] std::string nanoid_suffix(std::size_t length) const
    {
        static constexpr std::string_view alphabets{
            "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
        };
        std::string out{};
        out.reserve(length);
        for (std::size_t i{0}; i < length; ++i) {
            const auto index = std::hash<std::string>{}(m_context->ids().uuid_v4()) % alphabets.size();
            out.push_back(alphabets[index]);
        }
        return out;
    }

    [[nodiscard]] std::array<unsigned char, 32> sha256_bytes(std::string_view data) const
    {
        static constexpr std::array<std::uint32_t, 64> k{
            0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
            0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U, 0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
            0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU, 0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
            0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U, 0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
            0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U, 0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
            0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U, 0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
            0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U, 0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
            0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U, 0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U,
        };

        std::uint32_t h[8]{
            0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
            0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U,
        };

        std::vector<unsigned char> message{data.begin(), data.end()};
        const std::uint64_t bit_length = static_cast<std::uint64_t>(message.size()) * 8U;
        message.push_back(0x80U);
        while (message.size() % 64U != 56U) { message.push_back(0x00U); }
        for (int shift{56}; shift >= 0; shift -= 8) {
            message.push_back(static_cast<unsigned char>(bit_length >> shift));
        }

        const auto rotr = [](std::uint32_t value, unsigned bits)
        {
            return (value >> bits) | (value << (32U - bits));
        };

        for (std::size_t offset{0}; offset < message.size(); offset += 64U) {
            std::uint32_t w[64]{};
            for (std::size_t i{0}; i < 16U; ++i) {
                w[i] = (static_cast<std::uint32_t>(message[offset + i * 4]) << 24) |
                       (static_cast<std::uint32_t>(message[offset + i * 4 + 1]) << 16) |
                       (static_cast<std::uint32_t>(message[offset + i * 4 + 2]) << 8) |
                       static_cast<std::uint32_t>(message[offset + i * 4 + 3]);
            }
            for (std::size_t i{16}; i < 64U; ++i) {
                const std::uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
                const std::uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }

            std::uint32_t a{h[0]}, b{h[1]}, c{h[2]}, d{h[3]};
            std::uint32_t e{h[4]}, f{h[5]}, g{h[6]}, hh{h[7]};
            for (std::size_t i{0}; i < 64U; ++i) {
                const std::uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
                const std::uint32_t choice = (e & f) ^ (~e & g);
                const std::uint32_t temp1 = hh + s1 + choice + k[i] + w[i];
                const std::uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
                const std::uint32_t majority = (a & b) ^ (a & c) ^ (b & c);
                const std::uint32_t temp2 = s0 + majority;
                hh = g; g = f; f = e; e = d + temp1;
                d = c; c = b; b = a; a = temp1 + temp2;
            }
            h[0] += a; h[1] += b; h[2] += c; h[3] += d;
            h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
        }

        std::array<unsigned char, 32> digest{};
        for (std::size_t i{0}; i < 8U; ++i) {
            digest[i * 4] = static_cast<unsigned char>(h[i] >> 24);
            digest[i * 4 + 1] = static_cast<unsigned char>(h[i] >> 16);
            digest[i * 4 + 2] = static_cast<unsigned char>(h[i] >> 8);
            digest[i * 4 + 3] = static_cast<unsigned char>(h[i]);
        }
        return digest;
    }

    // Full upstream Event field set for a manual retry (core:397-429).
    [[nodiscard]] std::string build_retry_payload(
        const std::string& new_event_id,
        const std::string& initial_attempt_id,
        const std::string& idempotent_event_id,
        const std::string& processor_merchant_id,
        const std::string& provider_merchant_id,
        const std::string& business_profile_id,
        const std::string& initiator_merchant_id,
        const std::string& recipient,
        std::string_view original_payload
    ) const
    {
        const std::string primary_object_id = object_id_of(original_payload);
        const std::string primary_object_type = payload_field(original_payload, "primary_object_type");
        const std::string primary_object_created_at = payload_field(original_payload, "primary_object_created_at");
        const std::string metadata = payload_field(original_payload, "metadata");
        return std::format(
            "{{\"event_id\":\"{}\",\"initial_attempt_id\":\"{}\",\"idempotent_event_id\":\"{}\","
            "\"processor_merchant_id\":\"{}\",\"merchant_id\":\"{}\",\"business_profile_id\":\"{}\","
            "\"primary_object_id\":\"{}\",\"primary_object_type\":\"{}\",\"primary_object_created_at\":\"{}\","
            "\"initiator_merchant_id\":\"{}\",\"recipient\":\"{}\",\"metadata\":\"{}\","
            "\"delivery_attempt\":\"manual_retry\",\"is_webhook_notified\":false,"
            "\"is_overall_delivery_successful\":false,\"created_at\":\"{}\",\"request\":\"{}\"}}",
            json_escape(new_event_id),
            json_escape(initial_attempt_id),
            json_escape(idempotent_event_id),
            json_escape(processor_merchant_id),
            json_escape(provider_merchant_id),
            json_escape(business_profile_id),
            json_escape(primary_object_id),
            json_escape(primary_object_type),
            json_escape(primary_object_created_at),
            json_escape(initiator_merchant_id),
            json_escape(recipient),
            json_escape(metadata),
            json_escape(now_value()),
            json_escape(original_payload)
        );
    }

    // Skip router/literal prefixes so ids resolve for both mounted forms.
    [[nodiscard]] std::size_t id_base(const app::http::PathSegments& segments) const
    {
        std::size_t index = 0;
        while (index < segments.count()) {
            const auto value = segments.at(index).value_or("");
            if (value == "events" || value == "profile" || value == "list") {
                ++index;
                continue;
            }
            break;
        }
        return index;
    }

    [[nodiscard]] PathIds path_ids(interfaces::io::IRequest& req) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        const std::size_t base = id_base(segments);
        return PathIds{
            std::string{segments.at(base).value_or("")},
            std::string{segments.at(base + 1).value_or("")},
        };
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error_value
    ) const
    {
        auto text = serde::Ser::encode_json(error_value);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error_value.get_kind()));
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

} // namespace app::handlers::webhook_events
