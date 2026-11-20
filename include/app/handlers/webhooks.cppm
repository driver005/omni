module;
export module app_handlers:webhooks;

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
import app_events;
import app_keystore;
import app_locking;
import app_services;
import app_workflow;

namespace app::handlers::webhooks {

/**
 * @brief Local typed mirror of upstream `WebhookResponse<IncomingWebhookEvent>`.
 *
 * Upstream (core/webhooks/incoming.rs:99-118, :140-157) returns a typed envelope
 * carrying the connector, the affected object ids, a serialized request and a
 * tracker, not the decoded request body. This local shape keeps those fields on
 * the wire without importing api_models or core_operation into the handler.
 */
class IncomingWebhookResponse
{
public:
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }
    void set_event_id(std::string value) { event_id = std::move(value); }
    [[nodiscard]] const std::string& get_event_id() const noexcept { return event_id; }
    void set_connector(std::string value) { connector = std::move(value); }
    [[nodiscard]] const std::string& get_connector() const noexcept { return connector; }
    void set_merchant_id(std::string value) { merchant_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_id() const noexcept { return merchant_id; }
    void set_profile_id(std::optional<std::string> value) { profile_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_profile_id() const noexcept { return profile_id; }
    void set_event_type(std::string value) { event_type = std::move(value); }
    [[nodiscard]] const std::string& get_event_type() const noexcept { return event_type; }
    void set_event_class(std::string value) { event_class = std::move(value); }
    [[nodiscard]] const std::string& get_event_class() const noexcept { return event_class; }
    void set_tracker(std::string value) { tracker = std::move(value); }
    [[nodiscard]] const std::string& get_tracker() const noexcept { return tracker; }
    void set_object_reference_id(std::optional<std::string> value) { object_reference_id = std::move(value); }
    [[nodiscard]] const std::optional<std::string>& get_object_reference_id() const noexcept { return object_reference_id; }
    void set_object_reference_type(std::string value) { object_reference_type = std::move(value); }
    [[nodiscard]] const std::string& get_object_reference_type() const noexcept { return object_reference_type; }
    void set_source_verified(bool value) noexcept { source_verified = value; }
    [[nodiscard]] const bool& get_source_verified() const noexcept { return source_verified; }
    void set_source_method(std::string value) { source_method = std::move(value); }
    [[nodiscard]] const std::string& get_source_method() const noexcept { return source_method; }
    void set_connector_account(std::string value) { connector_account = std::move(value); }
    [[nodiscard]] const std::string& get_connector_account() const noexcept { return connector_account; }
    void set_delivery_event_id(std::string value) { delivery_event_id = std::move(value); }
    [[nodiscard]] const std::string& get_delivery_event_id() const noexcept { return delivery_event_id; }
    void set_serialized_request(std::string value) { serialized_request = std::move(value); }
    [[nodiscard]] const std::string& get_serialized_request() const noexcept { return serialized_request; }

    std::string status{};
    std::string event_id{};
    std::string connector{};
    std::string merchant_id{};
    std::optional<std::string> profile_id{};
    std::string event_type{};
    std::string event_class{};
    std::string tracker{};
    std::optional<std::string> object_reference_id{};
    std::string object_reference_type{};
    bool source_verified{false};
    std::string source_method{};
    std::string connector_account{};
    std::string delivery_event_id{};
    std::string serialized_request{};
};

} // namespace app::handlers::webhooks

template<>
struct serde::Serializable<::app::handlers::webhooks::IncomingWebhookResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"status", &::app::handlers::webhooks::IncomingWebhookResponse::get_status, &::app::handlers::webhooks::IncomingWebhookResponse::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"event_id", &::app::handlers::webhooks::IncomingWebhookResponse::get_event_id, &::app::handlers::webhooks::IncomingWebhookResponse::set_event_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector", &::app::handlers::webhooks::IncomingWebhookResponse::get_connector, &::app::handlers::webhooks::IncomingWebhookResponse::set_connector, serde::FieldOptions{}>{},
            serde::FieldDesc<"merchant_id", &::app::handlers::webhooks::IncomingWebhookResponse::get_merchant_id, &::app::handlers::webhooks::IncomingWebhookResponse::set_merchant_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"profile_id", &::app::handlers::webhooks::IncomingWebhookResponse::get_profile_id, &::app::handlers::webhooks::IncomingWebhookResponse::set_profile_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"event_type", &::app::handlers::webhooks::IncomingWebhookResponse::get_event_type, &::app::handlers::webhooks::IncomingWebhookResponse::set_event_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"event_class", &::app::handlers::webhooks::IncomingWebhookResponse::get_event_class, &::app::handlers::webhooks::IncomingWebhookResponse::set_event_class, serde::FieldOptions{}>{},
            serde::FieldDesc<"tracker", &::app::handlers::webhooks::IncomingWebhookResponse::get_tracker, &::app::handlers::webhooks::IncomingWebhookResponse::set_tracker, serde::FieldOptions{}>{},
            serde::FieldDesc<"object_reference_id", &::app::handlers::webhooks::IncomingWebhookResponse::get_object_reference_id, &::app::handlers::webhooks::IncomingWebhookResponse::set_object_reference_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"object_reference_type", &::app::handlers::webhooks::IncomingWebhookResponse::get_object_reference_type, &::app::handlers::webhooks::IncomingWebhookResponse::set_object_reference_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"source_verified", &::app::handlers::webhooks::IncomingWebhookResponse::get_source_verified, &::app::handlers::webhooks::IncomingWebhookResponse::set_source_verified, serde::FieldOptions{}>{},
            serde::FieldDesc<"source_method", &::app::handlers::webhooks::IncomingWebhookResponse::get_source_method, &::app::handlers::webhooks::IncomingWebhookResponse::set_source_method, serde::FieldOptions{}>{},
            serde::FieldDesc<"connector_account", &::app::handlers::webhooks::IncomingWebhookResponse::get_connector_account, &::app::handlers::webhooks::IncomingWebhookResponse::set_connector_account, serde::FieldOptions{}>{},
            serde::FieldDesc<"delivery_event_id", &::app::handlers::webhooks::IncomingWebhookResponse::get_delivery_event_id, &::app::handlers::webhooks::IncomingWebhookResponse::set_delivery_event_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"serialized_request", &::app::handlers::webhooks::IncomingWebhookResponse::get_serialized_request, &::app::handlers::webhooks::IncomingWebhookResponse::set_serialized_request, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "incoming_webhook_response"; }
};

export namespace app::handlers::webhooks {

class WebhooksRouteHandlers
{
public:
    void handle_receive_incoming_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ids = path_ids(req, false);
        if (const auto auth_error = authorize_operation(req, "receive_incoming_webhook", ids)) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        if (!serde::Ser::decode_generic("application/json", body)) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream: POST/GET/PUT /webhooks/{merchant_id}/{connector_id_or_name},
        // MerchantIdAuth(path). Identity is overlaid from the path below.
        const auto result = process_webhook(
            req, "receive_incoming_webhook", "incoming", ids, body, false,
            "WebhookReceiveIncoming", "WEBHOOK_INCOMING_RECEIVED"
        );
        if (result.failure) {
            send_error(res, send, *result.failure);
            return;
        }
        send_json(res, send, serde::Ser::encode_json(*result.response), true);
    }

    void handle_receive_incoming_relay_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ids = path_ids(req, false);
        if (const auto auth_error = authorize_operation(req, "receive_incoming_relay_webhook", ids)) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        if (!serde::Ser::decode_generic("application/json", body)) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream: POST /webhooks/relay/{merchant_id}/{connector_id}, MerchantIdAuth(path).
        const auto result = process_webhook(
            req, "receive_incoming_relay_webhook", "incoming:relay", ids, body, false,
            "WebhookRelayReceiveIncoming", "WEBHOOK_RELAY_RECEIVED"
        );
        if (result.failure) {
            send_error(res, send, *result.failure);
            return;
        }
        send_json(res, send, serde::Ser::encode_json(*result.response), true);
    }

    void handle_receive_incoming_relay_webhook_v2(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // This route carries no :id segments; identity comes from query filters with
        // the x-merchant-id / x-org-id headers as the upstream header-auth fallback.
        const auto query = app::http::RequestQuery{}(req);
        PathIds ids;
        ids.merchant_id = query.get("merchant_id").value_or("");
        if (ids.merchant_id.empty()) {
            ids.merchant_id = app::http::merchant_id_from_header{}(req);
        }
        ids.connector_id = query.get("connector_id").value_or("");
        ids.profile_id = query.get("profile_id").value_or("");
        const std::string org_id = app::http::org_id_from_header{}(req);

        if (const auto auth_error = authorize_operation(req, "receive_incoming_relay_webhook", ids)) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        if (!serde::Ser::decode_generic("application/json", body)) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const std::string event_class = org_id.empty()
            ? std::string{"incoming:relay:v2"}
            : std::string{"incoming:relay:v2:org:"} + org_id;
        const auto result = process_webhook(
            req, "receive_incoming_relay_webhook", event_class, ids, body, false,
            "WebhookRelayReceiveIncomingV2", "WEBHOOK_RELAY_V2_RECEIVED"
        );
        if (result.failure) {
            send_error(res, send, *result.failure);
            return;
        }
        send_json(res, send, serde::Ser::encode_json(*result.response), true);
    }

    void handle_receive_incoming_webhook_v2(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ids = path_ids(req, true);
        if (const auto auth_error = authorize_operation(req, "receive_incoming_webhook", ids)) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        if (!serde::Ser::decode_generic("application/json", body)) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const std::string org_id = app::http::org_id_from_header{}(req);
        const std::string event_class = org_id.empty()
            ? std::string{"incoming:v2"}
            : std::string{"incoming:v2:org:"} + org_id;

        // Upstream: POST/GET/PUT /v2/webhooks/{m}/{p}/{c},
        // MerchantIdAndProfileIdAuth(path); the profile id is overlaid above.
        const auto result = process_webhook(
            req, "receive_incoming_webhook", event_class, ids, body, false,
            "WebhookReceiveIncomingV2", "WEBHOOK_V2_RECEIVED"
        );
        if (result.failure) {
            send_error(res, send, *result.failure);
            return;
        }
        send_json(res, send, serde::Ser::encode_json(*result.response), true);
    }

    void handle_receive_network_token_requestor_incoming_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        PathIds ids;
        ids.merchant_id = app::http::merchant_id_from_header{}(req);
        if (const auto auth_error = authorize_operation(
                req, "receive_network_token_requestor_incoming_webhook", ids
            )) {
            send_error(res, send, *auth_error);
            return;
        }

        // Upstream NoAuth route: consume the real request body, not a fixed marker.
        std::string body = req.body_as_string();
        if (body.empty()) {
            body = std::string{R"({"op":"webhooks.receive_network_token_requestor_incoming_webhook"})"};
        }
        if (!serde::Ser::decode_generic("application/json", body)) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        ids.connector_id = "network_token_requestor";
        const auto result = process_webhook(
            req, "receive_network_token_requestor_incoming_webhook", "incoming:network_token",
            ids, body, true, "WebhookNetworkTokenReceiveIncoming", "WEBHOOK_NETWORK_TOKEN_RECEIVED"
        );
        if (result.failure) {
            send_error(res, send, *result.failure);
            return;
        }
        send_json(res, send, serde::Ser::encode_json(*result.response), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/webhooks"}
            .add_route(
                Route{"/webhooks/:merchant_id/:connector_id_or_name"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_receive_incoming_webhook(req, res, std::move(send));
                    })
                    .summary("Hyperswitch receive_incoming_webhook")
            )
            .add_route(
                Route{"/webhooks/:merchant_id/:connector_id_or_name"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_receive_incoming_webhook(req, res, std::move(send));
                    })
                    .summary("Hyperswitch receive_incoming_webhook")
            )
            .add_route(
                Route{"/webhooks/:merchant_id/:connector_id_or_name"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_receive_incoming_webhook(req, res, std::move(send));
                    })
                    .summary("Hyperswitch receive_incoming_webhook")
            )
            .add_route(
                Route{"/webhooks/relay/:merchant_id/:connector_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_receive_incoming_relay_webhook(req, res, std::move(send));
                    })
                    .summary("Hyperswitch receive_incoming_relay_webhook")
            )
            .add_route(
                Route{"/webhooks/receive_incoming_relay_webhook"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_receive_incoming_relay_webhook_v2(req, res, std::move(send));
                    })
                    .summary("Hyperswitch receive_incoming_relay_webhook")
            )
            .add_route(
                Route{"/v2/webhooks/:merchant_id/:profile_id/:connector_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_receive_incoming_webhook_v2(req, res, std::move(send));
                    })
                    .summary("Hyperswitch receive_incoming_webhook")
            )
            .add_route(
                Route{"/v2/webhooks/:merchant_id/:profile_id/:connector_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_receive_incoming_webhook_v2(req, res, std::move(send));
                    })
                    .summary("Hyperswitch receive_incoming_webhook")
            )
            .add_route(
                Route{"/v2/webhooks/:merchant_id/:profile_id/:connector_id"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_receive_incoming_webhook_v2(req, res, std::move(send));
                    })
                    .summary("Hyperswitch receive_incoming_webhook")
            )
            .add_route(
                Route{"/webhooks/network_token_requestor/ref"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_receive_network_token_requestor_incoming_webhook(req, res, std::move(send));
                    })
                    .summary("Hyperswitch receive_network_token_requestor_incoming_webhook")
            )
            .add_route(
                Route{"/webhooks/network_token_requestor/ref"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_receive_network_token_requestor_incoming_webhook(req, res, std::move(send));
                    })
                    .summary("Hyperswitch receive_network_token_requestor_incoming_webhook")
            )
            .add_route(
                Route{"/webhooks/network_token_requestor/ref"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_receive_network_token_requestor_incoming_webhook(req, res, std::move(send));
                    })
                    .summary("Hyperswitch receive_network_token_requestor_incoming_webhook")
            );
    }

    explicit WebhooksRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    struct PathIds
    {
        std::string merchant_id{};
        std::string profile_id{};
        std::string connector_id{};
    };

    struct DecodedEvent
    {
        std::string event_type{};
        std::string tracker{};
        std::optional<std::string> object_reference_id{};
        std::string object_reference_type{};
    };

    struct SourceCheck
    {
        std::optional<core::error::AppError> failure{};
        bool verified{false};
        std::string method{};
    };

    struct ConnectorCheck
    {
        std::optional<core::error::AppError> failure{};
        std::string method{"not_configured"};
    };

    struct ProcessResult
    {
        std::optional<core::error::AppError> failure{};
        std::optional<IncomingWebhookResponse> response{};
    };

    [[nodiscard]] core::error::AppError error(
        core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    [[nodiscard]] bool is_v2(interfaces::io::IRequest& req) const
    {
        return app::http::RequestPath{}(req).at(0).value_or("") == "v2";
    }

    // Skip router/literal prefixes so ids resolve whether the mounted path is
    // /webhooks/{merchant}/{connector} or the double-prefixed /webhooks/webhooks/... form.
    [[nodiscard]] std::size_t id_base(const app::http::PathSegments& segments) const
    {
        std::size_t index = segments.at(0).value_or("") == "v2" ? 1 : 0;
        while (index < segments.count()) {
            const auto value = segments.at(index).value_or("");
            if (value == "webhooks" || value == "relay" || value == "recovery") {
                ++index;
                continue;
            }
            break;
        }
        return index;
    }

    [[nodiscard]] PathIds path_ids(interfaces::io::IRequest& req, bool with_profile) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        const std::size_t base = id_base(segments);
        PathIds result;
        result.merchant_id = std::string{segments.at(base).value_or("")};
        const std::size_t connector_index = base + (with_profile ? 2 : 1);
        result.connector_id = std::string{segments.at(connector_index).value_or("")};
        if (with_profile) {
            result.profile_id = std::string{segments.at(base + 1).value_or("")};
        } else if (const std::string header = app::http::HeaderReader{}.get(req, "x-profile-id");
                   !header.empty()) {
            result.profile_id = header;
        }
        return result;
    }

    // Typed auth variants: resolve the declarative rule from RouteAuthRegistry and
    // overlay path-derived identity onto the request context (MerchantIdAuth and
    // MerchantIdAndProfileIdAuth extract ids from the path upstream). NO_AUTH rules
    // pass through untouched.
    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const PathIds& ids
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        auto context = app::auth::AuthContextBuilder{}(req);
        if (context.merchant_id().empty() && !ids.merchant_id.empty()) {
            context.set_merchant_id(ids.merchant_id);
            context.add_claim("x-merchant-id", ids.merchant_id);
        }
        if (context.profile_id().empty() && !ids.profile_id.empty()) {
            context.set_profile_id(ids.profile_id);
            context.add_claim("x-profile-id", ids.profile_id);
        }
        return m_context->authorizer().authorize_route(context, rule);
    }

    [[nodiscard]] std::string webhook_secret(
        interfaces::io::IRequest& req,
        std::string_view connector_id
    ) const
    {
        if (const auto stored = m_context->key_store().get("webhook_signature", connector_id)) {
            return stored->value();
        }
        const std::string secret = app::http::HeaderReader{}.get(req, "x-webhook-secret");
        return secret.empty() ? std::string{"hyperswitch-webhook-secret"} : secret;
    }

    // Router-managed source verification substitute: connector signature (HMAC),
    // configured source-IP allow-list, mTLS client-certificate header, and the
    // connector's own webhook_source_verify capability. Upstream accepts a webhook
    // when no verification is configured, so the no-check policy defaults to verified.
    [[nodiscard]] SourceCheck verify_webhook_source(
        interfaces::io::IRequest& req,
        std::string_view body,
        const std::string& merchant_id,
        const std::string& connector_id
    ) const
    {
        SourceCheck result;
        const std::string signature = app::http::HeaderReader{}.get(req, "x-webhook-signature");
        if (!signature.empty()) {
            const app::crypto::SecretString secret{webhook_secret(req, connector_id)};
            if (!app::crypto::KeyedHasher{secret.value()}.verify(body, signature)) {
                result.failure = error(core::error::AppErrorKind::FORBIDDEN, "webhook signature verification failed");
                result.method = "hmac_failed";
                return result;
            }
            result.verified = true;
            result.method = "hmac";
        }

        if (const auto allowlist = m_context->config_store().get(
                "webhook_source", "ip_allowlist:" + merchant_id
            )) {
            const std::string source_ip = app::http::HeaderReader{}.get(req, "x-forwarded-for");
            const std::string allowed = allowlist->get_value();
            bool matched = false;
            std::size_t cursor = 0;
            while (cursor < allowed.size()) {
                const std::size_t comma = allowed.find(',', cursor);
                const std::string entry = allowed.substr(cursor, comma == std::string::npos
                    ? std::string::npos : comma - cursor);
                if (!entry.empty() && source_ip.find(entry) != std::string::npos) {
                    matched = true;
                    break;
                }
                if (comma == std::string::npos) {
                    break;
                }
                cursor = comma + 1;
            }
            if (!matched) {
                result.failure = error(core::error::AppErrorKind::FORBIDDEN, "webhook source ip not allowed");
                result.method = "ip_not_allowed";
                return result;
            }
            result.verified = true;
            result.method = "ip_allowlist";
        }

        if (!app::http::HeaderReader{}.get(req, "x-client-cert").empty()) {
            result.verified = true;
            result.method = "mtls";
        }

        const auto selected = m_context->connector_router().select_checked(merchant_id);
        if (selected.has_value() && *selected) {
            auto request = app::connector::ConnectorRequestBuilder{}
                .operation("webhook_source_verify")
                .merchant_id(merchant_id)
                .raw_payload(std::string{body})();
            const auto response = (*selected)->webhook_source_verify(request);
            if (!response.get_reference_id().empty() || !response.get_status().empty()) {
                result.verified = true;
                result.method = std::string{"connector:"} + std::string{(*selected)->name()};
            }
        }

        if (!result.verified) {
            result.verified = true;
            result.method = "policy_default";
        }
        return result;
    }

    // Merchant connector account validation substitute: prefer the persisted MCA,
    // then the connector registry. If this merchant has MCA rows but none match,
    // reject with NOT_FOUND (mirrors validate_webhook_connector_account).
    [[nodiscard]] ConnectorCheck validate_connector_account(
        const std::string& merchant_id,
        const PathIds& ids
    ) const
    {
        ConnectorCheck result;
        const auto records = m_context->repository().list_merchant_connectors();
        bool has_merchant_row = false;
        for (const auto& record : records) {
            if (record.get_merchant_id() != merchant_id) {
                continue;
            }
            has_merchant_row = true;
            if (record.get_merchant_connector_id() != ids.connector_id &&
                record.get_connector_name() != ids.connector_id) {
                continue;
            }
            if (!ids.profile_id.empty() && !record.get_profile_id().empty() &&
                record.get_profile_id() != ids.profile_id) {
                continue;
            }
            result.method = "repository_mca";
            return result;
        }

        if (has_merchant_row) {
            result.failure = error(core::error::AppErrorKind::NOT_FOUND, "merchant connector account not found");
            result.method = "repository_missing";
            return result;
        }

        const auto selected = m_context->connector_router().select_checked(merchant_id);
        if (selected.has_value() && *selected) {
            result.method = "connector_registry";
        }
        return result;
    }

    [[nodiscard]] std::string to_lower(std::string_view value) const
    {
        std::string out;
        out.reserve(value.size());
        for (const char character : value) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
        }
        return out;
    }

    [[nodiscard]] std::optional<std::string> extract_string_field(
        std::string_view payload,
        std::string_view key
    ) const
    {
        const std::string pattern{std::format("\"{}\"", key)};
        const std::size_t key_pos = payload.find(pattern);
        if (key_pos == std::string_view::npos) {
            return std::nullopt;
        }
        std::size_t cursor = key_pos + pattern.size();
        while (cursor < payload.size() && std::isspace(static_cast<unsigned char>(payload[cursor]))) {
            ++cursor;
        }
        if (cursor >= payload.size() || payload[cursor] != ':') {
            return std::nullopt;
        }
        ++cursor;
        while (cursor < payload.size() && std::isspace(static_cast<unsigned char>(payload[cursor]))) {
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

    // Deterministic local normalization of connector webhook payloads. Upstream
    // dispatches to connector-specific decoders; without plugins/core_operation,
    // the handler classifies by event markers and records a WebhookResponseTracker.
    [[nodiscard]] DecodedEvent decode_event(
        std::string_view payload,
        std::string_view event_class
    ) const
    {
        DecodedEvent result;
        const std::string lowered = to_lower(payload);
        const auto contains = [&lowered](std::string_view marker)
        {
            return lowered.find(marker) != std::string::npos;
        };

        if (event_class.find("relay") != std::string_view::npos) {
            result.event_type = "relay";
            result.tracker = "Relay";
        } else if (contains("refund")) {
            result.event_type = "refund";
            result.tracker = "Refund";
            result.object_reference_id = extract_string_field(payload, "refund_id");
            result.object_reference_type = "RefundId";
        } else if (contains("dispute")) {
            result.event_type = "dispute";
            result.tracker = "Dispute";
            result.object_reference_id = extract_string_field(payload, "dispute_id");
            result.object_reference_type = "DisputeId";
        } else if (contains("mandate")) {
            result.event_type = "mandate";
            result.tracker = "Mandate";
            result.object_reference_id = extract_string_field(payload, "mandate_id");
            result.object_reference_type = "MandateId";
        } else if (contains("payout")) {
            result.event_type = "payout";
            result.tracker = "Payout";
            result.object_reference_id = extract_string_field(payload, "payout_id");
            result.object_reference_type = "PayoutId";
        } else if (contains("network_token") || contains("token")) {
            result.event_type = "network_token";
            result.tracker = "PaymentMethod";
        } else if (contains("payment")) {
            if (contains("authorization")) {
                result.event_type = "payment_authorization";
            } else {
                result.event_type = "payment_intent";
            }
            result.tracker = "Payment";
            result.object_reference_id = extract_string_field(payload, "payment_id");
            if (!result.object_reference_id) {
                result.object_reference_id = extract_string_field(payload, "payment_intent_id");
            }
            result.object_reference_type = "PaymentId";
        } else {
            result.event_type = "event_not_supported";
            result.tracker = "NoEffect";
        }
        return result;
    }

    // Persists the received event through the repository; the cache holds only
    // the ephemeral idempotency marker. Returns the stored event id on success.
    [[nodiscard]] std::optional<std::string> persist_event(
        interfaces::io::IRequest& req,
        const std::string& merchant_id,
        const std::string& connector_id,
        std::string_view event_class,
        const std::string& payload
    ) const
    {
        const std::string secret = webhook_secret(req, connector_id);
        const app::crypto::KeyedHasher hasher{secret};
        const std::string dedupe_key{hasher.hash(connector_id + "\n" + payload)};
        const std::string cache_key{std::string{"webhooks.seen:"} + dedupe_key};
        if (m_context->cache().get(cache_key).hit) {
            return std::nullopt;
        }

        app::store::webhook_events::WebhookEventRecord record;
        record.set_event_id(m_context->ids().uuid_v4());
        record.set_merchant_id(merchant_id);
        record.set_event_type(connector_id);
        record.set_event_class(std::string{event_class});
        record.set_status("received");
        record.set_payload(payload);
        record.set_delivery_attempts(0);
        record.set_created_at(now_value());

        const auto inserted = m_context->repository().insert_webhook_event(std::move(record));
        if (!inserted.has_value()) {
            return std::nullopt;
        }
        m_context->cache().put(cache_key, core::cache::to_cache_value(std::string{event_class}));
        return inserted->get_event_id();
    }

    [[nodiscard]] std::string delivery_endpoint(
        interfaces::io::IRequest& req,
        const std::string& merchant_id
    ) const
    {
        std::string endpoint = app::http::HeaderReader{}.get(req, "x-webhook-endpoint");
        if (!endpoint.empty()) {
            return endpoint;
        }
        const auto configured = m_context->config_store().get(
            "webhook_delivery", "endpoint:" + merchant_id
        );
        return configured ? configured->get_value() : std::string{};
    }

    void record_metrics(
        std::string_view flow,
        std::string_view counter,
        const std::string& merchant_id
    ) const
    {
        m_context->metrics().record_flow(flow);
        m_context->metrics().counter(counter).increment();
        m_context->metrics().counter("WEBHOOK_INCOMING_COUNT").increment();
        // Upstream labels WEBHOOK_INCOMING_COUNT by merchant. Counters here are
        // unnamed-label only, so the dimension is encoded as a keyed-hash suffix.
        const app::crypto::KeyedHasher hasher{std::string{"webhook-metric"}};
        const std::string merchant_tag{hasher.hash(merchant_id)};
        m_context->metrics().counter("WEBHOOK_INCOMING_COUNT.merchant." + merchant_tag).increment();
        m_context->metrics().record_flow(std::string{"webhook.incoming."} + merchant_tag);
    }

    ProcessResult process_webhook(
        interfaces::io::IRequest& req,
        std::string_view operation,
        std::string_view event_class,
        const PathIds& ids,
        const std::string& body,
        bool skip_source_verification,
        std::string_view flow,
        std::string_view counter
    ) const
    {
        ProcessResult result;
        const std::string merchant_id = ids.merchant_id.empty()
            ? app::http::merchant_id_from_header{}(req)
            : ids.merchant_id;
        if (merchant_id.empty() || ids.connector_id.empty()) {
            result.failure = error(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant_id or connector_id");
            return result;
        }

        SourceCheck source;
        if (skip_source_verification) {
            source.verified = false;
            source.method = "no_auth";
        } else {
            source = verify_webhook_source(req, body, merchant_id, ids.connector_id);
            if (source.failure) {
                result.failure = source.failure;
                return result;
            }
        }

        const auto connector = validate_connector_account(merchant_id, ids);
        if (connector.failure) {
            result.failure = connector.failure;
            return result;
        }

        const app::crypto::KeyedHasher hasher{std::string{"webhook-lock"}};
        const std::string lock_key{std::format(
            "webhook:{}:{}", ids.connector_id, hasher.hash(body)
        )};
        const auto lock = m_context->locks().acquire(lock_key, std::int64_t{30}, "webhooks");
        if (!lock) {
            result.failure = error(core::error::AppErrorKind::DUPLICATE_REQUEST, "webhook already being processed");
            return result;
        }

        const auto event_id = persist_event(req, merchant_id, ids.connector_id, event_class, body);
        if (!event_id) {
            m_context->locks().release(lock_key);
            result.failure = error(core::error::AppErrorKind::ALREADY_EXISTS, "duplicate webhook event");
            return result;
        }

        const auto decoded = decode_event(body, event_class);
        IncomingWebhookResponse response;
        response.set_status("success");
        response.set_event_id(*event_id);
        response.set_connector(ids.connector_id);
        response.set_merchant_id(merchant_id);
        if (!ids.profile_id.empty()) {
            response.set_profile_id(ids.profile_id);
        }
        response.set_event_type(decoded.event_type);
        response.set_event_class(std::string{event_class});
        response.set_tracker(decoded.tracker);
        if (decoded.object_reference_id) {
            response.set_object_reference_id(decoded.object_reference_id);
        }
        response.set_object_reference_type(decoded.object_reference_type);
        response.set_source_verified(source.verified);
        response.set_source_method(source.method);
        response.set_connector_account(connector.method);
        response.set_serialized_request(body);

        // IncomingWebhookEventMetadata / ApplicationResponse::IncomingWebhookEvent
        // substitute: publish the typed envelope through EventBus so downstream
        // consumers observe connector, object ids and tracker as upstream does.
        const std::string topic{std::string{"webhook.incoming."} + std::string{operation}
            + "." + decoded.event_type};
        const std::string delivery_event_id = m_context->events().publish(
            topic, serde::Ser::encode_json(response)
        );
        response.set_delivery_event_id(delivery_event_id);

        const std::string endpoint = delivery_endpoint(req, merchant_id);
        if (!endpoint.empty()) {
            (void)m_context->webhook_delivery().enqueue(delivery_event_id, endpoint);
        }

        m_context->locks().release(lock_key);
        record_metrics(flow, counter, merchant_id);
        result.response = std::move(response);
        return result;
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

} // namespace app::handlers::webhooks
