export module app_handlers_subscription:input;

import std;
import interfaces;
import core_serde;
import core_error;
import app_auth;
import app_http;
import app_context;

export namespace app::handlers::subscription {

using Json = serde::Value;
using JsonObject = Json::Object;

inline constexpr std::string_view kWritePermission{"PROFILE_SUBSCRIPTION_WRITE"};
inline constexpr std::string_view kReadPermission{"PROFILE_SUBSCRIPTION_READ"};
inline constexpr std::int64_t kMaxListCount{10}; // core.rs:29 SUBSCRIPTIONS_MAX_LIST_COUNT
inline constexpr std::int64_t kSessionExpirySeconds{15 * 60}; // common_utils/consts.rs:85

struct ParsedPayload
{
    Json doc{};
    const JsonObject* root{nullptr};
    std::string raw{};
};

/**
 * @brief Request -> Input support for the subscription handlers.
 *
 * Every error returned here is a core::error::AppError: its message holds the real
 * human-readable text, its kind_name holds the upstream code (IR_xx, HE_xx, ...),
 * and its kind selects the HTTP status via get_status(). handler.cppm serializes the
 * AppError itself with Ser::serialize; no hand-built error JSON text.
 */
class SubscriptionInputSupport
{
public:
    // ── AppError helpers ────────────────────────────────────────────────────────
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

    [[nodiscard]] core::error::AppError json_body_error(std::string_view message) const
    {
        // utils.rs:78-90 custom_json_error_handler -> invalid_request / IR_06.
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INVALID_REQUEST,
            std::string{message}
        );
        error.set_kind_name("IR_06");
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

    [[nodiscard]] core::error::AppError invalid_data_value(std::string_view name) const
    {
        return make_failure(
            core::error::AppErrorKind::INVALID_REQUEST,
            "invalid_request",
            "IR_07",
            std::format("Invalid value provided: {}", name)
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

    [[nodiscard]] core::error::AppError unauthorized() const
    {
        return make_failure(
            core::error::AppErrorKind::UNAUTHORIZED,
            "invalid_request",
            "IR_01",
            "API key not provided or invalid API key used"
        );
    }

    // ── JSON input helpers (rfl::Generic tree) ──────────────────────────────────
    [[nodiscard]] std::optional<Json> parse_json(std::string_view body) const
    {
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            return std::nullopt;
        }
        return std::move(*parsed);
    }

    [[nodiscard]] std::expected<ParsedPayload, core::error::AppError>
    parse_json_body(interfaces::io::IRequest& req) const
    {
        const auto body = req.body_as_string();
        auto parsed = parse_json(body);
        if (!parsed) {
            return std::unexpected{
                json_body_error("Json payload is empty or deserialization failed")
            };
        }
        const auto* root = std::get_if<JsonObject>(&parsed->get());
        if (root == nullptr) {
            return std::unexpected{json_body_error("Json payload must be an object")};
        }
        return ParsedPayload{std::move(*parsed), root, body};
    }

    [[nodiscard]] std::optional<ParsedPayload>
    parse_json_body_optional(interfaces::io::IRequest& req) const
    {
        const auto body = req.body_as_string();
        auto parsed = parse_json(body);
        if (!parsed) {
            return std::nullopt;
        }
        const auto* root = std::get_if<JsonObject>(&parsed->get());
        if (root == nullptr) {
            return std::nullopt;
        }
        return ParsedPayload{std::move(*parsed), root, body};
    }

    [[nodiscard]] Json json_from_query(const app::http::QueryParams& query) const
    {
        JsonObject object{};
        for (const auto* key: {"client_secret", "plan_id", "item_price_id", "coupon_code"}) {
            if (const auto value = query.get(key)) {
                object.insert(std::string{key}, Json{*value});
            }
        }
        return Json{std::move(object)};
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

    [[nodiscard]] const JsonObject*
    nested_object(const JsonObject* object, std::string_view key) const
    {
        if (object == nullptr) {
            return nullptr;
        }
        const auto field = object->get(std::string{key});
        if (!field) {
            return nullptr;
        }
        return std::get_if<JsonObject>(&field.value().get());
    }

    [[nodiscard]] bool has_payment_method_data_or_token(const JsonObject* details) const
    {
        if (details == nullptr) {
            return false;
        }
        return json_string(details, "payment_method_data").has_value() ||
               json_string(details, "payment_token").has_value() ||
               nested_object(details, "payment_method_data") != nullptr;
    }

    [[nodiscard]] bool has_payment_method_data_or_id(const JsonObject* details) const
    {
        if (details == nullptr) {
            return false;
        }
        return json_string(details, "payment_method_data").has_value() ||
               json_string(details, "payment_method_id").has_value() ||
               nested_object(details, "payment_method_data") != nullptr;
    }

    // ── Auth helpers (upstream authentication.rs semantics) ─────────────────────
    [[nodiscard]] std::optional<std::string>
    header(interfaces::io::IRequest& req, std::string_view name) const
    {
        const auto value = app::http::HeaderReader{}.get(req, name);
        return value.empty() ? std::nullopt : std::optional<std::string>{value};
    }

    [[nodiscard]] bool has_bearer_token(const app::auth::AuthContext& auth) const
    {
        return !auth.token().is_empty();
    }

    [[nodiscard]] bool
    permission_granted(const app::auth::AuthContext& auth, std::string_view required) const
    {
        const auto single = auth.claim("permission");
        const auto list = auth.claim("permissions");
        if (!single && !list) {
            return true;
        } // JWT without explicit claims: upstream grants by role.
        const auto grants = [required](std::string_view value)
        {
            std::string_view rest = value;
            while (!rest.empty()) {
                const auto comma = rest.find(',');
                const auto token = rest.substr(0, comma);
                if (token == required || token == "ADMIN") {
                    return true;
                }
                if (comma == std::string_view::npos) {
                    break;
                }
                rest = rest.substr(comma + 1);
            }
            return false;
        };
        return (single && grants(*single)) || (list && grants(*list));
    }

    // auth_type(HeaderAuth(ApiKeyAuth), JWTAuth{permission}, headers) (authentication.rs:6464-6475)
    [[nodiscard]] std::optional<core::error::AppError> authorize_api_key_or_jwt(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& auth,
        std::string_view permission
    ) const
    {
        if (has_bearer_token(auth)) {
            if (!permission_granted(auth, permission)) {
                return make_failure(
                    core::error::AppErrorKind::FORBIDDEN,
                    "invalid_request",
                    "IR_20",
                    "Access forbidden, insufficient permission"
                );
            }
            return std::nullopt;
        }
        return authorize_api_key(req, auth);
    }

    // HeaderAuth(ApiKeyAuth{allow_connected=false, allow_platform=false})
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_api_key(interfaces::io::IRequest& req, const app::auth::AuthContext& auth) const
    {
        (void)req;
        const auto api_key = auth.claim(app::auth::kClaimApiKey);
        if (!api_key || api_key->empty()) {
            return unauthorized();
        }
        if (auth.merchant_id().empty()) {
            return make_failure(
                core::error::AppErrorKind::INVALID_REQUEST,
                "invalid_request",
                "IR_04",
                "Missing required param: x-merchant-id"
            );
        }
        return std::nullopt;
    }

    // check_sdk_auth_and_get_auth + auth_type (routes/subscription.rs:214-235,284-303,432-457)
    [[nodiscard]] std::optional<core::error::AppError> authorize_sdk(
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

    [[nodiscard]] std::string
    effective_client_secret(const app::auth::AuthContext& auth, const JsonObject* payload) const
    {
        const auto claim = auth.claim(app::auth::kClaimClientSecret);
        if (claim && !claim->empty()) {
            return std::string{*claim};
        }
        return json_string(payload, "client_secret").value_or("");
    }

    [[nodiscard]] std::string
    effective_client_secret(const app::auth::AuthContext& auth, const Json& payload) const
    {
        return effective_client_secret(auth, std::get_if<JsonObject>(&payload.get()));
    }

    // ── X-Profile-Id extraction (routes/subscription.rs:21-47) ──────────────────
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    extract_profile_id(interfaces::io::IRequest& req) const
    {
        std::string raw = app::http::HeaderReader{}.get(req, "X-Profile-Id");
        if (raw.empty()) {
            raw = app::http::HeaderReader{}.get(req, "x-profile-id");
        }
        if (raw.empty()) {
            return std::unexpected{missing_field("X-Profile-Id")};
        }
        const auto trimmed = trim(raw);
        if (trimmed.empty()) {
            return std::unexpected{missing_field("X-Profile-Id")};
        }
        if (!valid_profile_id(trimmed)) {
            return std::unexpected{invalid_data_value("X-Profile-Id")};
        }
        return trimmed;
    }

    [[nodiscard]] std::string trim(std::string_view value) const
    {
        std::size_t begin = 0U;
        std::size_t end = value.size();
        while (begin < end && std::isspace(static_cast<unsigned char>(value[begin]))) {
            ++begin;
        }
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1U]))) {
            --end;
        }
        return std::string{value.substr(begin, end - begin)};
    }

    [[nodiscard]] bool valid_profile_id(std::string_view value) const
    {
        // ProfileId::try_from -> LengthId -> AlphaNumericId (id_type.rs:161-180)
        if (value.empty() || value.size() > 64U) {
            return false;
        }
        for (const char ch: value) {
            const auto c = static_cast<unsigned char>(ch);
            if (!std::isalnum(c) && ch != '_' && ch != '-') {
                return false;
            }
        }
        return true;
    }

    // ── Identity / request helpers ──────────────────────────────────────────────
    [[nodiscard]] std::string
    merchant_id(interfaces::io::IRequest& req, const app::auth::AuthContext& auth) const
    {
        if (!auth.merchant_id().empty()) {
            return auth.merchant_id();
        }
        return app::http::merchant_id_from_header{}(req);
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

    [[nodiscard]] std::int64_t query_number(
        const app::http::QueryParams& query,
        std::string_view key,
        std::int64_t fallback
    ) const
    {
        const auto raw = query.get(key);
        if (!raw) {
            return fallback;
        }
        std::int64_t value{};
        const auto converted = std::from_chars(raw->data(), raw->data() + raw->size(), value);
        return converted.ec == std::errc{} && converted.ptr == raw->data() + raw->size() ? value
                                                                                         : fallback;
    }

    [[nodiscard]] std::uint64_t query_unsigned(
        const app::http::QueryParams& query,
        std::string_view key,
        std::uint64_t fallback
    ) const
    {
        const auto raw = query.get(key);
        if (!raw) {
            return fallback;
        }
        std::uint64_t value{};
        const auto converted = std::from_chars(raw->data(), raw->data() + raw->size(), value);
        return converted.ec == std::errc{} && converted.ptr == raw->data() + raw->size() ? value
                                                                                         : fallback;
    }
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old inline core block consumed.

struct CreateSubscriptionInput
{
    std::string profile_id{};
    std::string merchant_id{};
    std::string customer_id{};
    std::string item_price_id{};
    ParsedPayload payload{};
};

struct PauseSubscriptionInput
{
    std::string profile_id{};
    std::string merchant_id{};
    std::string subscription_id{};
    std::optional<ParsedPayload> payload{};
};

struct ResumeSubscriptionInput
{
    std::string profile_id{};
    std::string merchant_id{};
    std::string subscription_id{};
    std::optional<ParsedPayload> payload{};
};

struct CancelSubscriptionInput
{
    std::string profile_id{};
    std::string merchant_id{};
    std::string subscription_id{};
    std::optional<ParsedPayload> payload{};
};

struct ConfirmSubscriptionInput
{
    std::string profile_id{};
    std::string merchant_id{};
    std::string subscription_id{};
    std::string client_secret{};
    ParsedPayload payload{};
};

struct GetSubscriptionItemsInput
{
    std::string profile_id{};
    std::string merchant_id{};
    std::string client_secret{};
    std::uint32_t limit{};
    std::uint32_t offset{};
    std::string item_type{};
};

struct GetSubscriptionInput
{
    std::string profile_id{};
    std::string merchant_id{};
    std::string subscription_id{};
};

struct CreateAndConfirmSubscriptionInput
{
    std::string profile_id{};
    std::string merchant_id{};
    std::string customer_id{};
    std::string item_price_id{};
    ParsedPayload payload{};
};

struct GetEstimateInput
{
    std::string profile_id{};
    std::string merchant_id{};
    std::string client_secret{};
    Json payload{};
};

struct UpdateSubscriptionInput
{
    std::string profile_id{};
    std::string merchant_id{};
    std::string subscription_id{};
    std::string plan_id{};
    std::string item_price_id{};
};

struct ListSubscriptionsInput
{
    std::string profile_id{};
    std::string merchant_id{};
    std::int64_t limit{};
    std::int64_t offset{};
};

} // namespace app::handlers::subscription
