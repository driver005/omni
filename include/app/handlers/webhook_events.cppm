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
    void handle_list_initial_webhook_delivery_attempts_with_jwtauth(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: POST /events/profile/list, JWTAuth ProfileWebhookEventRead.
        // Merchant and profile come from the token (allow_connected), never the body.
        const auto identity = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize(identity, "list_initial_webhook_delivery_attempts_with_jwtauth")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto merchant_id = merchant_from_identity(identity);
        if (merchant_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant_id"));
            return;
        }
        if (const auto profile_error = validate_profile(identity.profile_id(), merchant_id)) {
            send_error(res, send, *profile_error);
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
        if (!identity.profile_id().empty()) {
            app::dto::webhook_events::ProfileIdValue profile;
            profile.set_value(identity.profile_id());
            constraints->set_profile_id(std::move(profile));
        }

        auto listed = list_initial_events(merchant_id, *constraints, identity.profile_id());
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

    void handle_list_initial_webhook_delivery_attempts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: POST /events/{merchant_id}, JWTAuthMerchantFromRoute.
        const auto identity = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize(identity, "list_initial_webhook_delivery_attempts")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto ids = path_ids(req);
        if (ids.merchant_id.empty()) {
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
        std::string profile_id = identity.profile_id();
        if (constraints->get_profile_id() && !constraints->get_profile_id()->get_value().empty()) {
            profile_id = constraints->get_profile_id()->get_value();
        }
        if (const auto profile_error = validate_profile(profile_id, ids.merchant_id)) {
            send_error(res, send, *profile_error);
            return;
        }

        auto listed = list_initial_events(ids.merchant_id, *constraints, profile_id);
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

    void handle_list_webhook_delivery_attempts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: GET /events/{merchant_id}/{initial_attempt_id}/attempts, no JSON body.
        const auto identity = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize(identity, "list_webhook_delivery_attempts")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto ids = path_ids(req);
        if (ids.merchant_id.empty() || ids.event_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant_id or initial_attempt_id"));
            return;
        }

        std::vector<app::dto::webhook_events::EventRetrieveResponse> events;
        const auto records = m_context->repository().list_webhook_events(
            app::store::WebhookEventQuery{}.merchant_id(ids.merchant_id)
        );
        for (const auto& record : records) {
            const auto payload = payload_of(record, ids.merchant_id);
            // Upstream keys this lookup by initial_attempt_id (the retry chain root),
            // not by the row's own event_id.
            if (initial_attempt_id_of(record, payload) != ids.event_id) {
                continue;
            }
            if (initiator_of(record, payload) != ids.merchant_id) {
                continue;
            }
            events.push_back(to_retrieve(record, payload, identity.profile_id()));
        }
        if (events.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "event not found"));
            return;
        }

        m_context->metrics().record_flow("WebhookEventDeliveryAttemptList");
        m_context->metrics().counter("WEBHOOK_EVENT_DELIVERY_ATTEMPTS_LIST").increment();
        send_json(res, send, serde::Ser::encode_json(events), true);
    }

    void handle_retry_webhook_delivery_attempt(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: POST /events/{merchant_id}/{event_id}/retry, no JSON body.
        const auto identity = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize(identity, "retry_webhook_delivery_attempt")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto ids = path_ids(req);
        if (ids.merchant_id.empty() || ids.event_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant_id or event_id"));
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

        const auto original = m_context->repository().find_webhook_event_by_event_id(ids.event_id);
        if (!original) {
            m_context->locks().release(lock_key);
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "event not found"));
            return;
        }
        const auto original_payload = payload_of(*original, ids.merchant_id);
        // Upstream checks initiator_merchant_id with a legacy merchant_id fallback.
        if (initiator_of(*original, original_payload) != ids.merchant_id) {
            m_context->locks().release(lock_key);
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "event not found"));
            return;
        }

        const std::string idem_key = app::crypto::KeyedHasher{event_key(ids.merchant_id)}
                                         .hash_with_domain("idempotency", ids.event_id + ":" + ids.merchant_id);
        // get_idempotent_event_id: a repeated retry returns the event already created.
        for (const auto& candidate : m_context->repository().list_webhook_events(
                 app::store::WebhookEventQuery{}.merchant_id(ids.merchant_id))) {
            const auto payload = payload_of(candidate, ids.merchant_id);
            if (payload_field(payload, "idempotent_event_id") == idem_key) {
                m_context->locks().release(lock_key);
                m_context->metrics().record_flow("WebhookEventDeliveryRetry");
                m_context->metrics().counter("WEBHOOK_EVENT_DELIVERY_RETRY").increment();
                send_json(res, send, serde::Ser::encode_json(to_retrieve(candidate, payload, identity.profile_id())), true);
                return;
            }
        }

        const std::string new_event_id = m_context->ids().uuid_v4();
        const std::string initial_attempt_id = initial_attempt_id_of(*original, original_payload);
        const std::string processor_merchant_id = resolve_processor_merchant(ids.merchant_id, original_payload);
        const std::string recipient = resolve_recipient(ids.merchant_id, original_payload);

        app::store::webhook_events::WebhookEventRecord retry;
        retry.set_event_id(new_event_id);
        retry.set_merchant_id(ids.merchant_id);
        retry.set_event_type(original->get_event_type());
        retry.set_event_class(original->get_event_class());
        retry.set_status("manual_retry");
        retry.set_payload(build_retry_payload(
            new_event_id, initial_attempt_id, idem_key, processor_merchant_id,
            recipient, ids.merchant_id, identity.profile_id(), original_payload
        ));
        retry.set_delivery_attempts(original->get_delivery_attempts() + 1);
        retry.set_created_at(now_value());

        auto inserted = m_context->repository().insert_webhook_event(std::move(retry));
        if (!inserted) {
            m_context->locks().release(lock_key);
            send_error(res, send, inserted.error());
            return;
        }

        // trigger_webhook_and_raise_event: publish a Kafka-shaped event, then drive one
        // deterministic local delivery attempt through WebhookDeliveryService.
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

        const auto stored = m_context->repository().find_webhook_event_by_event_id(inserted->get_event_id());
        if (!stored) {
            m_context->locks().release(lock_key);
            send_error(res, send, error(core::error::AppErrorKind::INTERNAL_ERROR, "retry event lost"));
            return;
        }
        const auto stored_payload = payload_of(*stored, ids.merchant_id);
        m_context->locks().release(lock_key);

        m_context->metrics().record_flow("WebhookEventDeliveryRetry");
        m_context->metrics().counter("WEBHOOK_EVENT_DELIVERY_RETRY").increment();
        send_json(res, send, serde::Ser::encode_json(to_retrieve(*stored, stored_payload, identity.profile_id())), true);
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

    struct PathIds
    {
        std::string merchant_id{};
        // Attempts route: upstream initial_attempt_id. Retry route: event_id.
        std::string event_id{};
    };

    struct ConstraintPlan
    {
        std::string merchant_id{};
        std::string profile_id{};
        std::optional<std::string> object_id{};
        std::optional<std::string> event_id{};
        std::vector<std::string> event_classes{};
        std::vector<std::string> event_types{};
        std::optional<bool> is_delivered{};
        std::optional<std::string> recipient{};
        std::string created_after{};
        std::string created_before{};
        std::size_t limit{100};
        std::size_t offset{0};
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

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        const app::auth::AuthContext& identity,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(identity, rule);
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

    // Local substitute for the upstream business-profile lookup: the injected
    // ConfigStore carries business_profile/<profile_id> -> merchant_id rows.
    // With no local profile store seeded, an unknown profile is accepted but
    // recorded as a scoped lookup; a known profile must belong to the merchant.
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

    [[nodiscard]] bool delivered_of(const Record& record, std::string_view payload) const
    {
        const std::string flag = payload_field(payload, "is_delivered");
        if (flag == "true") { return true; }
        if (flag == "false") { return false; }
        const auto& status = record.get_status();
        return status == "DELIVERED" || status == "SUCCESS" || status == "Success";
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

    [[nodiscard]] std::string resolve_processor_merchant(
        const std::string& merchant_id,
        std::string_view original_payload
    ) const
    {
        const auto configured = m_context->config_store().get_value("webhook_processor_merchant", merchant_id);
        if (configured && !configured->empty()) {
            return *configured;
        }
        const std::string stored = payload_field(original_payload, "processor_merchant_id");
        return stored.empty() ? merchant_id : stored;
    }

    [[nodiscard]] std::string resolve_recipient(
        const std::string& merchant_id,
        std::string_view original_payload
    ) const
    {
        const auto configured = m_context->config_store().get_value("webhook_recipient", merchant_id);
        if (configured && !configured->empty()) {
            return *configured;
        }
        return payload_field(original_payload, "recipient");
    }

    [[nodiscard]] std::string build_retry_payload(
        const std::string& new_event_id,
        const std::string& initial_attempt_id,
        const std::string& idempotent_event_id,
        const std::string& processor_merchant_id,
        const std::string& recipient,
        const std::string& initiator_merchant_id,
        const std::string& profile_id,
        std::string_view original_payload
    ) const
    {
        return std::format(
            "{{\"event_id\":\"{}\",\"initial_attempt_id\":\"{}\",\"idempotent_event_id\":\"{}\","
            "\"processor_merchant_id\":\"{}\",\"recipient\":\"{}\",\"initiator_merchant_id\":\"{}\","
            "\"profile_id\":\"{}\",\"is_webhook_notified\":false,\"body\":\"{}\"}}",
            json_escape(new_event_id),
            json_escape(initial_attempt_id),
            json_escape(idempotent_event_id),
            json_escape(processor_merchant_id),
            json_escape(recipient),
            json_escape(initiator_merchant_id),
            json_escape(profile_id),
            json_escape(original_payload)
        );
    }

    [[nodiscard]] std::vector<std::string> sensitive_header_names(const std::string& profile_id) const
    {
        const auto configured = m_context->config_store().get_value(
            "webhook_sensitive_headers", profile_id.empty() ? std::string{"default"} : profile_id
        );
        if (!configured || configured->empty()) {
            return {
                "authorization",
                "proxy-authorization",
                "x-api-key",
                "x-client-secret",
                "x-webhook-signature",
                "x-webhook-signature-256",
            };
        }
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
            if (last > first) {
                names.emplace_back(part.substr(first, last - first));
            }
            if (comma == std::string_view::npos) {
                break;
            }
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

    // finalize_event_types: class -> type resolution plus subset validation.
    [[nodiscard]] std::expected<std::vector<std::string>, core::error::AppError> finalize_event_types(
        const Constraints& constraints,
        std::vector<std::string>& classes_out
    ) const
    {
        if (constraints.get_event_classes()) {
            for (const auto& value : *constraints.get_event_classes()) {
                classes_out.push_back(value.get_value());
            }
        }

        std::vector<std::string> resolved{};
        if (!classes_out.empty()) {
            for (const auto& event_class : classes_out) {
                if (event_class == "payment") {
                    resolved.insert(resolved.end(), {
                        "payment_intent.succeeded", "payment_intent.failed",
                        "payment_intent.processing", "payment_intent.requires_action",
                    });
                } else if (event_class == "refund") {
                    resolved.insert(resolved.end(), {"refund.succeeded", "refund.failed", "refund.pending"});
                } else if (event_class == "payout") {
                    resolved.insert(resolved.end(), {"payout.paid", "payout.failed", "payout.created"});
                } else if (event_class == "dispute") {
                    resolved.insert(resolved.end(), {"dispute.opened", "dispute.won", "dispute.lost"});
                } else if (event_class == "mandate") {
                    resolved.insert(resolved.end(), {"mandate.created", "mandate.revoked"});
                } else if (event_class == "subscription") {
                    resolved.insert(resolved.end(), {"subscription.created", "subscription.cancelled"});
                } else if (event_class == "incoming") {
                    resolved.insert(resolved.end(), {"incoming.webhook.received"});
                } else {
                    return std::unexpected{error(
                        core::error::AppErrorKind::VALIDATION_FAILED,
                        "unknown event class: " + event_class
                    )};
                }
            }
        }

        if (constraints.get_event_types()) {
            std::vector<std::string> requested{};
            for (const auto& value : *constraints.get_event_types()) {
                requested.push_back(value.get_value());
            }
            if (!classes_out.empty()) {
                for (const auto& type : requested) {
                    if (std::find(resolved.begin(), resolved.end(), type) == resolved.end()) {
                        return std::unexpected{error(
                            core::error::AppErrorKind::VALIDATION_FAILED,
                            "event type not in event class: " + type
                        )};
                    }
                }
                return requested;
            }
            return requested;
        }
        return resolved;
    }

    [[nodiscard]] std::expected<ConstraintPlan, core::error::AppError> build_plan(
        const Constraints& constraints,
        const std::string& merchant_id,
        const std::string& profile_id,
        std::int64_t now
    ) const
    {
        ConstraintPlan plan{};
        plan.merchant_id = merchant_id;
        plan.profile_id = profile_id;

        if (constraints.get_limit()) {
            if (*constraints.get_limit() > 100) {
                return std::unexpected{error(
                    core::error::AppErrorKind::VALIDATION_FAILED, "limit must not exceed 100"
                )};
            }
            plan.limit = *constraints.get_limit();
        }
        if (constraints.get_offset()) {
            plan.offset = *constraints.get_offset();
        }
        if (constraints.get_object_id()) {
            plan.object_id = *constraints.get_object_id();
        }
        if (constraints.get_event_id()) {
            plan.event_id = *constraints.get_event_id();
        }
        if (constraints.get_is_delivered()) {
            plan.is_delivered = *constraints.get_is_delivered();
        }
        if (constraints.get_recipient()) {
            plan.recipient = constraints.get_recipient()->get_value();
        }

        const std::string after = constraints.get_created_after()
                                      ? constraints.get_created_after()->get_value()
                                      : std::to_string(now - 90LL * 24 * 3600);
        const std::string before = constraints.get_created_before()
                                       ? constraints.get_created_before()->get_value()
                                       : std::to_string(now);
        if (!constraints.get_created_after() && !constraints.get_created_before()) {
            // The 90-day window is applied unconditionally when no explicit bounds exist.
        } else if (after > before) {
            return std::unexpected{error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "created_after must not be after created_before"
            )};
        }
        plan.created_after = after;
        plan.created_before = before;

        auto types = finalize_event_types(constraints, plan.event_classes);
        if (!types) {
            return std::unexpected{std::move(types.error())};
        }
        plan.event_types = std::move(*types);
        return plan;
    }

    [[nodiscard]] bool matches(const Record& record, const ConstraintPlan& plan, std::string_view payload) const
    {
        if (plan.object_id && payload_field(payload, "object_id") != *plan.object_id) {
            return false;
        }
        if (plan.event_id && record.get_event_id() != *plan.event_id
            && payload_field(payload, "event_id") != *plan.event_id) {
            return false;
        }
        if (plan.recipient && payload_field(payload, "recipient") != *plan.recipient) {
            return false;
        }
        if (plan.is_delivered && delivered_of(record, payload) != *plan.is_delivered) {
            return false;
        }
        const std::string event_profile = payload_field(payload, "profile_id");
        if (!plan.profile_id.empty() && !event_profile.empty() && event_profile != plan.profile_id) {
            return false;
        }
        if (!plan.event_classes.empty()
            && std::find(plan.event_classes.begin(), plan.event_classes.end(), record.get_event_class())
                   == plan.event_classes.end()) {
            return false;
        }
        if (!plan.event_types.empty()
            && std::find(plan.event_types.begin(), plan.event_types.end(), record.get_event_type())
                   == plan.event_types.end()) {
            return false;
        }
        const auto created = parse_seconds(record.get_created_at());
        const auto after = parse_seconds(plan.created_after);
        const auto before = parse_seconds(plan.created_before);
        if (created && after && *created < *after) {
            return false;
        }
        if (created && before && *created > *before) {
            return false;
        }
        if (!created && !record.get_created_at().empty()) {
            if (!plan.created_after.empty() && record.get_created_at() < plan.created_after) {
                return false;
            }
            if (!plan.created_before.empty() && record.get_created_at() > plan.created_before) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::expected<EventListResult, core::error::AppError> list_initial_events(
        const std::string& merchant_id,
        const Constraints& constraints,
        const std::string& profile_id
    ) const
    {
        const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now().time_since_epoch()
                         ).count();
        auto plan = build_plan(constraints, merchant_id, profile_id, now);
        if (!plan) {
            return std::unexpected{std::move(plan.error())};
        }

        auto query = app::store::WebhookEventQuery{}.merchant_id(merchant_id);
        if (plan->event_id) {
            query.event_id(*plan->event_id);
        }
        if (plan->event_types.size() == 1) {
            query.event_type(plan->event_types.front());
        }

        // Separate store query plays the role of upstream COUNT(*).
        const auto counted = m_context->repository().list_webhook_events(query);
        EventListResult result{};
        result.total_count = static_cast<std::int64_t>(counted.size());

        std::vector<std::pair<Record, std::string>> matched{};
        for (const auto& record : counted) {
            const auto payload = payload_of(record, merchant_id);
            if (!matches(record, *plan, payload)) {
                continue;
            }
            matched.emplace_back(record, payload);
        }

        std::size_t skipped{0};
        for (const auto& [record, payload] : matched) {
            if (skipped++ < plan->offset) {
                continue;
            }
            if (result.items.size() >= plan->limit) {
                break;
            }
            result.items.push_back(to_list_item(record, payload));
        }
        return result;
    }

    [[nodiscard]] app::dto::webhook_events::EventListItemResponse to_list_item(
        const Record& record,
        std::string_view payload
    ) const
    {
        app::dto::webhook_events::EventListItemResponse item;
        item.set_event_id(record.get_event_id());

        app::dto::webhook_events::MerchantIdValue merchant;
        merchant.set_value(record.get_merchant_id());
        item.set_merchant_id(std::move(merchant));

        const std::string profile = payload_field(payload, "profile_id");
        if (!profile.empty()) {
            app::dto::webhook_events::ProfileIdValue value;
            value.set_value(profile);
            item.set_profile_id(std::move(value));
        }

        const std::string object_id = payload_field(payload, "object_id");
        if (!object_id.empty()) {
            item.set_object_id(object_id);
        }

        app::dto::webhook_events::EventTypeValue type;
        type.set_value(record.get_event_type());
        item.set_event_type(std::move(type));

        app::dto::webhook_events::EventClassValue event_class;
        event_class.set_value(record.get_event_class());
        item.set_event_class(std::move(event_class));

        const std::string delivered = payload_field(payload, "is_delivered");
        if (delivered == "true" || delivered == "false") {
            item.set_is_delivery_successful(delivered == "true");
        }

        app::dto::webhook_events::PrimitiveDateTimeValue created;
        created.set_value(record.get_created_at());
        item.set_created(std::move(created));

        item.set_initial_attempt_id(initial_attempt_id_of(record, payload));

        const std::string processor = payload_field(payload, "processor_merchant_id");
        if (!processor.empty()) {
            app::dto::webhook_events::MerchantIdValue value;
            value.set_value(processor);
            item.set_processor_merchant_id(std::move(value));
        }
        return item;
    }

    [[nodiscard]] app::dto::webhook_events::EventRetrieveResponse to_retrieve(
        const Record& record,
        std::string_view payload,
        const std::string& profile_id
    ) const
    {
        app::dto::webhook_events::EventRetrieveResponse response;
        response.set_event_information(to_list_item(record, payload));

        app::dto::webhook_events::SecretStringValue body;
        body.set_value(std::string{payload});
        app::dto::webhook_events::OutgoingWebhookRequestContent request;
        request.set_body(std::move(body));

        // Sensitive webhook headers from the business profile are redacted, not echoed.
        std::vector<std::pair<std::string, app::dto::webhook_events::SecretStringValue>> headers{};
        for (const auto& name : sensitive_header_names(profile_id)) {
            app::dto::webhook_events::SecretStringValue value;
            value.set_value("***REDACTED***");
            headers.emplace_back(name, std::move(value));
        }
        request.set_headers(std::move(headers));
        response.set_request(std::move(request));

        app::dto::webhook_events::OutgoingWebhookResponseContent outgoing;
        app::dto::webhook_events::SecretStringValue response_body;
        response_body.set_value(record.get_status());
        outgoing.set_body(std::move(response_body));
        response.set_response(std::move(outgoing));

        app::dto::webhook_events::WebhookDeliveryAttemptValue attempt;
        attempt.set_value(record.get_status());
        response.set_delivery_attempt(std::move(attempt));
        return response;
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

    [[nodiscard]] std::string now_value() const
    {
        return std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
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
