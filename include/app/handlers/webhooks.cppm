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

export namespace app::handlers::webhooks {

using Status = interfaces::io::types::Status;

/**
 * @brief Local mirror of upstream `WebhookResponse<R>`
 * (hyperswitch_domain_models/src/api.rs:33-39). Only the acknowledgement is on the
 * wire; `ApplicationResponse::IncomingWebhookEvent` metadata is consumed by
 * server_wrap_util for the ApiEvent log (router/src/services/api.rs:323-341, :477-485).
 * Upstream's default connector trait returns `WebhookResponse::StatusOk`
 * (hyperswitch_interfaces/src/webhooks.rs:346-354), which is HTTP 200 with an empty
 * body (router/src/services/api.rs:730-732).
 */
enum class AckKind : std::uint8_t
{
    STATUS_OK,
    JSON,
    TEXT_PLAIN,
};

class WebhookAck
{
public:
    void set_kind(AckKind value) noexcept { m_kind = value; }
    [[nodiscard]] AckKind kind() const noexcept { return m_kind; }
    void set_body(std::string value) { m_body = std::move(value); }
    [[nodiscard]] const std::string& body() const noexcept { return m_body; }

private:
    AckKind m_kind{AckKind::STATUS_OK};
    std::string m_body{};
};

/**
 * @brief Local mirror of upstream `ApiErrorResponse` -> `ErrorResponse`
 * (api_models/src/errors/types.rs:37-58). The local core `AppError` shape
 * (`{"error":KIND,"message":TEXT}`) is not the upstream wire shape, so webhook
 * errors are materialized here to keep the wire identical.
 */
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

class ApiError
{
public:
    ApiError() = default;
    ApiError(ErrorId id, std::string message) :
        m_id{id}, m_message{std::move(message)}
    {
    }

    void set_id(ErrorId value) noexcept { m_id = value; }
    [[nodiscard]] ErrorId id() const noexcept { return m_id; }
    void set_message(std::string value) { m_message = std::move(value); }
    [[nodiscard]] const std::string& message() const noexcept { return m_message; }

    [[nodiscard]] Status status() const noexcept
    {
        switch (m_id) {
        case ErrorId::UNAUTHORIZED:
        case ErrorId::WEBHOOK_AUTH_FAILED:
            return Status::UNAUTHORIZED;
        case ErrorId::WEBHOOK_BAD_REQUEST:
            return Status::BAD_REQUEST;
        case ErrorId::WEBHOOK_PROCESSING_FAILURE:
        case ErrorId::INTERNAL:
            return Status::INTERNAL_SERVER_ERROR;
        case ErrorId::INCORRECT_CONNECTOR_NAME:
            return Status::NOT_FOUND;
        case ErrorId::WEBHOOK_UNPROCESSABLE:
            return Status::UNPROCESSABLE_CONTENT;
        case ErrorId::WEBHOOK_RATE_LIMITED:
            return Status::TOO_MANY_REQUESTS;
        }
        return Status::INTERNAL_SERVER_ERROR;
    }

    [[nodiscard]] std::string to_json() const
    {
        const std::string_view type = (m_id == ErrorId::WEBHOOK_PROCESSING_FAILURE ||
                                       m_id == ErrorId::INTERNAL)
                                          ? "api"
                                          : "invalid_request";
        const std::string_view code = code_for(m_id);
        const auto quote = [](std::string_view value)
        {
            return serde::Ser::encode_json(std::string{value});
        };
        return std::format(
            R"({{"error":{{"type":{},"message":{},"code":{}}}}})",
            quote(type), quote(m_message), quote(code)
        );
    }

private:
    [[nodiscard]] static constexpr std::string_view code_for(ErrorId id) noexcept
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

    ErrorId m_id{ErrorId::INTERNAL};
    std::string m_message{};
};

/** @brief Which upstream auth extractor a route uses (routes/webhooks.rs:40,79,120-123,163-166,190). */
enum class AuthVariant : std::uint8_t
{
    MERCHANT_ID,
    MERCHANT_AND_PROFILE_ID,
    NO_AUTH,
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

    /** @brief Port of NetworkVerification::resolve_source_verified (source_verification.rs:60-68). */
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

struct PathIds
{
    std::string merchant_id{};
    std::string profile_id{};
    std::string connector_id{};
};

struct ConnectorAccount
{
    std::optional<app::store::merchant_connectors::MerchantConnectorRecord> mca{};
    std::string connector_name{};
};

struct ObjectRef
{
    std::string id{};
    std::string type{};
};

struct ProcessResult
{
    std::optional<ApiError> failure{};
    WebhookAck ack{};
};

class WebhooksRouteHandlers
{
public:
    // Upstream POST/GET/PUT /webhooks/{merchant_id}/{connector_id_or_name},
    // MerchantIdAuth(path) (routes/webhooks.rs:11-44, app.rs:2352-2357).
    void handle_receive_incoming_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ids = path_ids(req, false);
        if (const auto auth_error = authorize(req, AuthVariant::MERCHANT_ID, ids)) {
            send_error(res, send, *auth_error);
            return;
        }
        const std::string body = req.body_as_string();
        const auto result = incoming_webhooks_wrapper(
            req, ids, body, false, false, "receive_incoming_webhook",
            "IncomingWebhookReceive", "WEBHOOK_INCOMING_RECEIVED"
        );
        if (result.failure) {
            send_error(res, send, *result.failure);
            return;
        }
        send_ack(res, send, result.ack);
    }

    // Upstream POST /webhooks/relay/{merchant_id}/{connector_id},
    // MerchantIdAuth(path), is_relay_webhook=true (routes/webhooks.rs:46-83, app.rs:2376-2380).
    void handle_receive_incoming_relay_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ids = path_ids(req, false);
        if (const auto auth_error = authorize(req, AuthVariant::MERCHANT_ID, ids)) {
            send_error(res, send, *auth_error);
            return;
        }
        const std::string body = req.body_as_string();
        const auto result = incoming_webhooks_wrapper(
            req, ids, body, true, false, "receive_incoming_relay_webhook",
            "IncomingRelayWebhookReceive", "WEBHOOK_RELAY_RECEIVED"
        );
        if (result.failure) {
            send_error(res, send, *result.failure);
            return;
        }
        send_ack(res, send, result.ack);
    }

    // Upstream POST/GET/PUT /v2/webhooks/{merchant_id}/{profile_id}/{connector_id},
    // MerchantIdAndProfileIdAuth(path) (routes/webhooks.rs:129-170, app.rs:2391-2395).
    void handle_receive_incoming_webhook_v2(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto ids = path_ids(req, true);
        if (const auto auth_error = authorize(req, AuthVariant::MERCHANT_AND_PROFILE_ID, ids)) {
            send_error(res, send, *auth_error);
            return;
        }
        const std::string body = req.body_as_string();
        const auto result = incoming_webhooks_wrapper(
            req, ids, body, false, true, "receive_incoming_webhook",
            "IncomingWebhookReceive", "WEBHOOK_V2_RECEIVED"
        );
        if (result.failure) {
            send_error(res, send, *result.failure);
            return;
        }
        send_ack(res, send, result.ack);
    }

    // Upstream POST/GET/PUT /webhooks/network_token_requestor/ref, NoAuth
    // (routes/webhooks.rs:172-194, app.rs:2346-2351). Core still verifies the
    // Authorization header against the network-tokenization service secret
    // (incoming.rs:863-934, network_tokenization_incoming.rs:201-238).
    void handle_receive_network_token_requestor_incoming_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        PathIds ids;
        ids.merchant_id = app::http::merchant_id_from_header{}(req);
        if (const auto auth_error = authorize(
                req, AuthVariant::NO_AUTH, ids
            )) {
            send_error(res, send, *auth_error);
            return;
        }
        const std::string body = req.body_as_string();
        const auto result = network_token_incoming_webhooks_wrapper(
            req, ids, body, "receive_network_token_requestor_incoming_webhook",
            "IncomingNetworkTokenWebhookReceive", "WEBHOOK_NETWORK_TOKEN_RECEIVED"
        );
        if (result.failure) {
            send_error(res, send, *result.failure);
            return;
        }
        send_ack(res, send, result.ack);
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

    explicit WebhooksRouteHandlers(app::context::AppContext& context) noexcept :
        m_context{&context}
    {
    }

private:
    // ---- small lexical helpers -------------------------------------------------

    [[nodiscard]] ApiError error(ErrorId id, std::string message) const
    {
        return ApiError{id, std::move(message)};
    }

    [[nodiscard]] static std::string quote(std::string_view value)
    {
        return serde::Ser::encode_json(std::string{value});
    }

    [[nodiscard]] static std::string to_lower(std::string_view value)
    {
        std::string out;
        out.reserve(value.size());
        for (const char character : value) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
        }
        return out;
    }

    [[nodiscard]] static std::optional<std::string>
    extract_string_field(std::string_view payload, std::string_view key)
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

    [[nodiscard]] static bool contains(std::string_view haystack, std::string_view needle)
    {
        return haystack.find(needle) != std::string_view::npos;
    }

    [[nodiscard]] std::optional<std::string> config(
        std::string_view section,
        std::string_view key
    ) const
    {
        return m_context->config_store().get_value(section, key);
    }

    [[nodiscard]] bool config_bool(
        std::string_view section,
        std::string_view key,
        bool fallback = false
    ) const
    {
        const auto value = config(section, key);
        if (!value) {
            return fallback;
        }
        const std::string lowered = to_lower(*value);
        return lowered == "true" || lowered == "1";
    }

    // ---- path / auth -----------------------------------------------------------

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
        }
        return result;
    }

    // Local typed rules mirror the upstream extractors (routes/webhooks.rs:40,
    // :79, :120-123, :163-166, :190). The shared auth registry still carries the
    // legacy HEADER_MERCHANT rule; the handler builds the typed rule locally so
    // the auth semantics stay upstream-identical without editing shared files.
    [[nodiscard]] app::auth::RouteAuthRule auth_rule(AuthVariant variant) const
    {
        using app::auth::AppPermission;
        using app::auth::AuthScheme;
        using app::auth::RouteAuthRuleBuilder;

        if (variant == AuthVariant::NO_AUTH) {
            return RouteAuthRuleBuilder{}
                .scheme(AuthScheme::NO_AUTH)
                .permission(AppPermission::ADMIN)();
        }
        if (variant == AuthVariant::MERCHANT_AND_PROFILE_ID) {
            return RouteAuthRuleBuilder{}
                .scheme(AuthScheme::MERCHANT_ID)
                .permission(AppPermission::WEBHOOK_WRITE)
                .id_source("x-profile-id")();
        }
        return RouteAuthRuleBuilder{}
            .scheme(AuthScheme::MERCHANT_ID)
            .permission(AppPermission::WEBHOOK_WRITE)();
    }

    [[nodiscard]] std::optional<ApiError> authorize(
        interfaces::io::IRequest& req,
        AuthVariant variant,
        const PathIds& ids
    ) const
    {
        auto context = app::auth::AuthContextBuilder{}(req);
        if (!ids.merchant_id.empty()) {
            context.set_merchant_id(ids.merchant_id);
            context.add_claim("x-merchant-id", ids.merchant_id);
        }
        if (!ids.profile_id.empty()) {
            context.set_profile_id(ids.profile_id);
            context.add_claim("x-profile-id", ids.profile_id);
        }
        const auto denied = m_context->authorizer().authorize_route(context, auth_rule(variant));
        if (!denied) {
            return std::nullopt;
        }
        // Upstream MerchantIdAuth / MerchantIdAndProfileIdAuth failures are
        // ApiErrorResponse::Unauthorized (api_error_response.rs:560-564).
        return error(
            ErrorId::UNAUTHORIZED,
            "API key not provided or invalid API key used"
        );
    }

    // ---- MCA / execution path --------------------------------------------------

    [[nodiscard]] bool is_connector_name(
        std::string_view token,
        const std::vector<app::store::merchant_connectors::MerchantConnectorRecord>& records
    ) const
    {
        for (const auto& record : records) {
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
    [[nodiscard]] std::optional<ApiError> validate_webhook_connector_account(
        const std::string& merchant_id,
        const PathIds& ids,
        ConnectorAccount& account
    ) const
    {
        const auto records = m_context->repository().list_merchant_connectors();
        const bool by_name = is_connector_name(ids.connector_id, records);

        if (by_name) {
            bool found = false;
            for (const auto& record : records) {
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
                return error(ErrorId::UNAUTHORIZED, "API key not provided or invalid API key used");
            }
            account.connector_name = ids.connector_id;
            return std::nullopt;
        }

        for (const auto& record : records) {
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
        return error(ErrorId::UNAUTHORIZED, "API key not provided or invalid API key used");
    }

    // Port of fetch_three_ds_execution_path + confirm_path_for_flow
    // (incoming.rs:160-173, :473-517) using local config as the substitute for
    // superposition/authentication-service state.
    [[nodiscard]] ExecutionPath fetch_execution_path(
        const std::string& merchant_id,
        const std::string& connector_name
    ) const
    {
        const auto integration = config("webhooks", "connector_integration_type:" + connector_name);
        if (integration && to_lower(*integration) == "ucs") {
            return ExecutionPath::UNIFIED_CONNECTOR_SERVICE;
        }

        const auto eligible = config("authentication_service", "enabled_connectors");
        const bool connector_eligible = eligible &&
            contains(to_lower(*eligible), to_lower(connector_name));
        const bool merchant_eligible = config_bool(
            "authentication_service", "merchant_eligible:" + merchant_id
        );
        if (connector_eligible && merchant_eligible) {
            return ExecutionPath::UNIFIED_AUTHENTICATION_SERVICE;
        }
        return ExecutionPath::DIRECT;
    }

    // ---- source verification ---------------------------------------------------

    // Port of source_verification.rs:161-247 with the local header/config
    // substitute for the trusted-ingress mTLS signal.
    [[nodiscard]] std::optional<ApiError> verify_mtls(
        interfaces::io::IRequest& req,
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
        const std::string header = config("incoming_webhooks", "mtls_verified_header")
            .value_or(std::string{"x-webhook-mtls-verified"});
        const std::string value = app::http::HeaderReader{}.get(req, header);
        if (value != "true") {
            return error(
                ErrorId::WEBHOOK_AUTH_FAILED,
                "Incoming webhook mTLS verification failed: the request has missing or invalid verification header"
            );
        }
        status = VerificationStatus::VERIFIED;
        return std::nullopt;
    }

    // Port of ip_verification.rs:51-104. The local config value is the
    // connector's allowed-range policy; CIDR prefixes are matched on their
    // network portion (local in-process substitute for the Rust IpAddr parser).
    [[nodiscard]] std::optional<ApiError> verify_source_ip(
        interfaces::io::IRequest& req,
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
            return error(
                ErrorId::INTERNAL,
                "Invalid incoming webhook allowed IP ranges config"
            );
        }
        const std::string header = config("incoming_webhooks", "source_ip_header")
            .value_or(std::string{"x-forwarded-for"});
        const std::string source_ip = app::http::HeaderReader{}.get(req, header);
        if (source_ip.empty()) {
            return error(ErrorId::WEBHOOK_AUTH_FAILED, "Incoming webhook source IP header is missing");
        }

        bool matched = false;
        std::size_t cursor = 0;
        while (cursor < allowed->size()) {
            const std::size_t comma = allowed->find(',', cursor);
            const std::string entry = allowed->substr(
                cursor, comma == std::string::npos ? std::string::npos : comma - cursor
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
            return error(ErrorId::WEBHOOK_AUTH_FAILED, "Incoming webhook source IP is outside the allowed ranges");
        }
        status = VerificationStatus::VERIFIED;
        return std::nullopt;
    }

    [[nodiscard]] std::optional<ApiError> verify_network_source(
        interfaces::io::IRequest& req,
        const std::string& connector_name,
        NetworkVerification& verification
    ) const
    {
        if (const auto failure = verify_mtls(req, connector_name, verification.mtls)) {
            return failure;
        }
        return verify_source_ip(req, connector_name, verification.source_ip);
    }

    [[nodiscard]] std::string webhook_secret(
        interfaces::io::IRequest& req,
        const ConnectorAccount& account
    ) const
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
        if (const auto stored = m_context->key_store().get(
                "webhook_signature", account.connector_name
            )) {
            return stored->value();
        }
        const std::string header = app::http::HeaderReader{}.get(req, "x-webhook-secret");
        return header;
    }

    // Local connector-level verify_webhook_source (incoming_v2.rs:246-290):
    // HMAC signature over the raw body, or the connector source-verification
    // call when configured for this connector.
    [[nodiscard]] bool verify_webhook_source_call(
        interfaces::io::IRequest& req,
        std::string_view body,
        const ConnectorAccount& account,
        ExecutionPath path
    ) const
    {
        const auto call_config = config("webhook_source_verification_call", "connectors");
        const bool use_call = call_config &&
            contains(to_lower(*call_config), to_lower(account.connector_name));

        if (use_call || path == ExecutionPath::UNIFIED_CONNECTOR_SERVICE) {
            const auto selected = m_context->connector_router().select_checked(account.connector_name);
            if (selected.has_value() && *selected) {
                auto request = app::connector::ConnectorRequestBuilder{}
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
        for (const auto header : signature_headers) {
            signature = app::http::HeaderReader{}.get(req, header);
            if (!signature.empty()) {
                break;
            }
        }
        if (signature.empty()) {
            return false;
        }
        const std::string secret = webhook_secret(req, account);
        if (secret.empty()) {
            return false;
        }
        return app::crypto::KeyedHasher{app::crypto::SecretString{secret}.value()}
            .verify(body, signature);
    }

    [[nodiscard]] bool webhook_source_mandatory(const std::string& connector_name) const
    {
        return config_bool("webhook_source", "mandatory:" + connector_name);
    }

    // ---- webhook body decoding -------------------------------------------------

    // Local substitute for connector `decode_webhook_body` (incoming_v2.rs:151-160).
    [[nodiscard]] std::optional<ApiError> decode_webhook_body(std::string_view body) const
    {
        if (!serde::Ser::decode_generic("application/json", body)) {
            return error(ErrorId::WEBHOOK_UNPROCESSABLE, "There was an issue processing the webhook body");
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

    [[nodiscard]] ObjectRef get_webhook_object_reference_id(
        std::string_view body,
        const std::string& event_type
    ) const
    {
        ObjectRef ref;
        const auto pick = [&](std::initializer_list<std::string_view> keys, std::string_view type)
        {
            for (const auto key : keys) {
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

    // ---- persistence / event logging substitutes ------------------------------

    [[nodiscard]] std::optional<std::string> persist_event(
        const std::string& merchant_id,
        const std::string& connector_name,
        const std::string& event_type,
        std::string_view event_class,
        const std::string& payload
    ) const
    {
        app::store::webhook_events::WebhookEventRecord record;
        record.set_event_id(m_context->ids().uuid_v4());
        record.set_merchant_id(merchant_id);
        record.set_event_type(event_type);
        record.set_event_class(std::string{event_class});
        record.set_status("received");
        record.set_payload(payload);
        record.set_delivery_attempts(0);
        record.set_created_at(now_value());

        const auto inserted = m_context->repository().insert_webhook_event(std::move(record));
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
        const std::string event_type_json = event_kind == "NetworkTokenWebhook"
            ? std::format(
                  R"({{"NetworkTokenWebhook":{{"payment_method_id":{}}}}})",
                  quote(payment_method_id)
              )
            : std::format(
                  R"({{"Webhooks":{{"connector":{},"payment_id":{},"refund_id":{}}}}})",
                  quote(connector), quote(payment_id), quote(refund_id)
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
            event_type_json, quote(serialized_request), tracker_json
        );
        (void)m_context->events().publish(
            std::string{"webhook.incoming."} + std::string{operation}, metadata
        );
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
        // Upstream labels the counter by merchant (incoming.rs:194-200); local
        // counters have no labels, so the dimension is a keyed-hash suffix.
        const app::crypto::KeyedHasher hasher{std::string{"webhook-metric"}};
        const std::string merchant_tag{hasher.hash(merchant_id)};
        m_context->metrics().counter("WEBHOOK_INCOMING_COUNT.merchant." + merchant_tag).increment();
        m_context->metrics().record_flow(std::string{"webhook.incoming."} + merchant_tag);
    }

    // ---- core: incoming_webhooks_wrapper + incoming_webhooks_core --------------

    // Port of incoming.rs:77-119 / :177-470 and incoming_v2.rs:52-103 / :103-...
    // Connector decoders, 3DS/UAS/UCS plugins and business-effect handlers are
    // replaced by the local substitutes above; the control flow, error shapes,
    // source-verification policy and wire acknowledgement stay upstream-shaped.
    [[nodiscard]] ProcessResult incoming_webhooks_wrapper(
        interfaces::io::IRequest& req,
        const PathIds& ids,
        const std::string& body,
        bool is_relay,
        bool with_profile,
        std::string_view operation,
        std::string_view flow,
        std::string_view counter
    ) const
    {
        ProcessResult result;
        const std::string merchant_id = ids.merchant_id.empty()
            ? app::http::merchant_id_from_header{}(req)
            : ids.merchant_id;
        if (merchant_id.empty() || ids.connector_id.empty()) {
            result.failure = error(ErrorId::WEBHOOK_BAD_REQUEST, "Bad request body received");
            return result;
        }

        ConnectorAccount account;
        if (const auto failure = validate_webhook_connector_account(merchant_id, ids, account)) {
            result.failure = failure;
            return result;
        }

        record_metrics(flow, counter, merchant_id);
        const ExecutionPath path = fetch_execution_path(merchant_id, account.connector_name);

        NetworkVerification network;
        if (const auto failure = verify_network_source(req, account.connector_name, network)) {
            result.failure = failure;
            return result;
        }

        if (const auto failure = decode_webhook_body(body)) {
            result.failure = failure;
            return result;
        }

        ObjectRef object_ref;
        std::string event_type;
        std::string tracker{"NoEffect"};
        bool source_verified{false};
        bool process_further{false};

        const auto event = get_webhook_event_type(body);
        if (!event) {
            // Upstream early return for unidentified event types
            // (incoming_v2.rs:179-211): ack via connector, no effects, Null body.
            result.ack = get_webhook_api_response(account.connector_name);
            publish_metadata(
                operation, "Webhooks", account.connector_name, "", "", "",
                "null", tracker, object_ref, "event_not_supported", "NoEffect", false
            );
            return result;
        }
        event_type = *event;

        if (event_type == "setup_webhook") {
            // incoming_v2.rs:215-222 -> WebhookResponse::StatusOk.
            result.ack = get_webhook_api_response(account.connector_name);
            publish_metadata(
                operation, "Webhooks", account.connector_name, "", "", "",
                "{}", tracker, object_ref, event_type, "NoEffect", true
            );
            return result;
        }

        const bool supported = event_type != "event_not_supported";
        const bool enabled = !is_webhook_event_disabled(account.connector_name, event_type);
        process_further = supported && enabled;

        if (process_further) {
            object_ref = get_webhook_object_reference_id(body, event_type);
            const bool connector_source_verified =
                verify_webhook_source_call(req, body, account, path);
            source_verified = network.resolve_source_verified(connector_source_verified);

            if (source_verified) {
                m_context->metrics().counter("WEBHOOK_SOURCE_VERIFIED_COUNT").increment();
            } else if (webhook_source_mandatory(account.connector_name)) {
                // incoming_v2.rs:305-318 -> WebhookAuthenticationFailed.
                result.failure = error(
                    ErrorId::WEBHOOK_AUTH_FAILED, "Webhook authentication failed"
                );
                return result;
            }

            tracker = is_relay ? "Relay"
                : contains(event_type, "refund") ? "Refund"
                : contains(event_type, "dispute") ? "Dispute"
                : contains(event_type, "mandate") ? "Mandate"
                : contains(event_type, "payout") ? "Payout"
                : contains(event_type, "recovery") ? "Payment"
                : contains(event_type, "payment") ? "Payment"
                : "NoEffect";

            const auto event_id = persist_event(
                merchant_id, account.connector_name, event_type,
                is_relay ? "incoming:relay" : (with_profile ? "incoming:v2" : "incoming"),
                body
            );
            if (!event_id || event_id->empty()) {
                result.failure = error(
                    ErrorId::WEBHOOK_PROCESSING_FAILURE,
                    "There was an issue processing the webhook"
                );
                return result;
            }
        }

        const std::string event_class = with_profile ? "incoming:v2" : "incoming";
        publish_metadata(
            operation, "Webhooks", account.connector_name,
            tracker == "Payment" ? object_ref.id : "",
            tracker == "Refund" ? object_ref.id : "",
            "", body, tracker, object_ref, event_type, event_class, source_verified
        );

        result.ack = get_webhook_api_response(account.connector_name);
        return result;
    }

    // Port of incoming.rs:121-158 / :863-934. Upstream NoAuth at the transport
    // layer; source verification is an exact Authorization-header comparison
    // against the network-tokenization service secret.
    [[nodiscard]] ProcessResult network_token_incoming_webhooks_wrapper(
        interfaces::io::IRequest& req,
        const PathIds& ids,
        const std::string& body,
        std::string_view operation,
        std::string_view flow,
        std::string_view counter
    ) const
    {
        ProcessResult result;

        // get_network_token_resource_object (incoming.rs:872-878): deserialization
        // failure is InternalServerError.
        if (!serde::Ser::decode_generic("application/json", body)) {
            result.failure = error(
                ErrorId::INTERNAL,
                "Network Token Requestor Webhook deserialization failed"
            );
            return result;
        }

        // Network tokenization service must be configured (incoming.rs:880-886).
        const auto secret = config(
            "network_tokenization_service", "webhook_source_verification_key"
        );
        if (!secret || secret->empty()) {
            result.failure = error(
                ErrorId::INTERNAL, "Network Tokenization Service not configured"
            );
            return result;
        }

        // Authorization::verify_webhook_source (network_tokenization_incoming.rs:212-237).
        const std::string authorization = app::http::HeaderReader{}.get(req, "authorization");
        if (authorization != *secret) {
            result.failure = error(ErrorId::WEBHOOK_AUTH_FAILED, "Webhook authentication failed");
            return result;
        }

        // NetworkTokenWebhookResponse parse (incoming.rs:893-896).
        const bool has_card_reference = extract_string_field(body, "card_reference").has_value() ||
            extract_string_field(body, "card_reference_id").has_value() ||
            extract_string_field(body, "reference").has_value();
        if (!has_card_reference) {
            result.failure = error(
                ErrorId::WEBHOOK_UNPROCESSABLE, "There was an issue processing the webhook body"
            );
            return result;
        }

        // callback_mapper substitute (network_tokenization_incoming.rs:54-70):
        // the local store has no callback mapper table, so ids are read from
        // the webhook payload with the x-merchant-id header as fallback.
        const std::string merchant_id = extract_string_field(body, "merchant_id")
            .value_or(ids.merchant_id);
        const std::string payment_method_id = extract_string_field(body, "payment_method_id")
            .value_or(extract_string_field(body, "payment_method").value_or(""));
        const std::string customer_id = extract_string_field(body, "customer_id").value_or("");
        if (merchant_id.empty()) {
            result.failure = error(
                ErrorId::WEBHOOK_UNPROCESSABLE, "There was an issue processing the webhook body"
            );
            return result;
        }

        m_context->metrics().counter("WEBHOOK_SOURCE_VERIFIED_COUNT").increment();
        record_metrics(flow, counter, merchant_id);

        ObjectRef ref;
        ref.id = payment_method_id;
        ref.type = "PaymentMethod";
        const auto event_id = persist_event(
            merchant_id, "network_token_requestor", "network_token_metadata_update",
            "incoming:network_token", body
        );
        if (!event_id || event_id->empty()) {
            result.failure = error(
                ErrorId::WEBHOOK_PROCESSING_FAILURE,
                "There was an issue processing the webhook"
            );
            return result;
        }

        publish_metadata(
            operation, "NetworkTokenWebhook", "network_token_requestor",
            "", "", payment_method_id, body, "PaymentMethod", ref,
            "network_token_metadata_update", "incoming:network_token", true
        );
        (void)customer_id;

        // Upstream returns WebhookResponse::StatusOk (incoming.rs:928-933).
        result.ack = get_webhook_api_response("network_token_requestor");
        return result;
    }

    // ---- wire helpers ----------------------------------------------------------

    [[nodiscard]] static std::string now_value()
    {
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        return std::to_string(seconds);
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        const ApiError& error_value
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", error_value.to_json());
        res.set_status(error_value.status());
        res.set_body(std::move(bytes));
        send();
    }

    void send_ack(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        const WebhookAck& ack
    ) const
    {
        if (ack.kind() == AckKind::STATUS_OK) {
            // Upstream http_response_ok() = HttpResponse::Ok().finish()
            // (router/src/services/api.rs:730-732).
            res.set_status(Status::OK);
            res.set_body(std::vector<std::byte>{});
            send();
            return;
        }
        if (ack.kind() == AckKind::TEXT_PLAIN) {
            auto bytes = serde::Ser::serialize_raw("text/plain", ack.body());
            res.set_status(Status::OK);
            res.set_body(std::move(bytes));
            send();
            return;
        }
        auto bytes = serde::Ser::serialize_raw("application/json", ack.body());
        res.set_status(Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::webhooks
