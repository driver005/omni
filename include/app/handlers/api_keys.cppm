module;
export module app_handlers:api_keys;

import std;
import interfaces;
import core_router;
import core_serde;
import core_database;
import core_cache;
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
import app_workflow;
import app_services;

export namespace app::handlers::api_keys {

// Local equivalents of the upstream Cargo features that shape this module
// (routes/api_keys.rs, core/api_keys.rs, app.rs:2483-2515).
inline constexpr bool kV1 = true;   // /api_keys/{merchant_id}
inline constexpr bool kV2 = true;   // /v2/api-keys
inline constexpr bool kEmail = true; // expiry-reminder process-tracker workflow

inline constexpr std::size_t kApiKeyLength = 64;  // consts.rs:53 API_KEY_LENGTH
inline constexpr std::size_t kHashKeyLen = 32;    // PlaintextApiKey::HASH_KEY_LEN
inline constexpr std::size_t kPrefixLen = 12;     // PlaintextApiKey::PREFIX_LEN
inline constexpr std::string_view kDecisionAdd = "ADD";
inline constexpr std::string_view kDecisionRevoke = "REVOKE";
inline constexpr std::string_view kExpiryRunner = "ApiKeyExpiryWorkflow";
inline constexpr std::string_view kExpiryName = "API_KEY_EXPIRY";
inline constexpr std::string_view kExpiryTag = "API_KEY";

/**
 * @brief Local in-process stand-in for common_utils::crypto::
 * generate_cryptographically_secure_random_string (crypto.rs:676-680), which
 * samples the [A-Za-z0-9] alphabet from OsRng. The external RNG boundary is the
 * only substitute; length and shape are upstream-identical.
 */
inline std::string generate_cryptographically_secure_random_string(std::size_t length)
{
    constexpr std::string_view alphabet{
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
    };
    std::random_device entropy;
    std::mt19937_64 generator{entropy()};
    std::uniform_int_distribution<std::size_t> pick{0, alphabet.size() - 1};
    std::string out{};
    out.reserve(length);
    for (std::size_t i = 0; i < length; ++i) {
        out.push_back(alphabet[pick(generator)]);
    }
    return out;
}

/** @brief Local equivalent of router_env::env::prefix_for_env (env.rs:58-67). */
inline std::string_view prefix_for_env(std::string_view env)
{
    if (env == "development" || env == "dev") { return "dev"; }
    if (env == "integration" || env == "integ") { return "integ"; }
    if (env == "production" || env == "prd") { return "prd"; }
    return "snd";
}

/**
 * @brief Local stand-in for blake3::keyed_hash(...).to_hex() (core/api_keys.rs:78-108).
 * BLAKE3 is an external crypto primitive; four domain-separated KeyedHasher
 * digests reproduce the 64-lowercase-hex width persisted as `HashedApiKey`.
 */
inline std::string blake3_keyed_hash(std::string_view key, std::string_view input)
{
    const app::crypto::KeyedHasher hasher{std::string{key}};
    std::string out{};
    out.reserve(64);
    for (int block = 0; block < 4; ++block) {
        out += hasher.hash_with_domain(
            std::string{"api_key_block_"} + std::to_string(block), input
        );
    }
    return out;
}

/**
 * @brief Local newtypes mirroring PlaintextApiKey / HashedApiKey
 * (core/api_keys.rs:47-109) so plaintext never reaches a persisted record.
 */
struct HashedApiKey
{
    std::string value{};
    [[nodiscard]] const std::string& peek() const noexcept { return value; }
};

struct PlaintextApiKey
{
    app::crypto::SecretString secret{};

    [[nodiscard]] static PlaintextApiKey new_(std::size_t length, std::string_view env)
    {
        return PlaintextApiKey{app::crypto::SecretString{
            std::string{env} + "_" + generate_cryptographically_secure_random_string(length)
        }};
    }

    /** @brief ApiKeyId::generate_key_id(env) -> generate_ref_id_with_default_length. */
    [[nodiscard]] static std::string new_key_id(std::string_view env)
    {
        return std::string{env} + "_" + generate_cryptographically_secure_random_string(16);
    }

    [[nodiscard]] std::string prefix() const
    {
        const std::string& value = secret.value();
        return value.substr(0, std::min(kPrefixLen, value.size()));
    }

    [[nodiscard]] const std::string& peek() const noexcept { return secret.value(); }

    [[nodiscard]] HashedApiKey keyed_hash(std::string_view key) const
    {
        return HashedApiKey{blake3_keyed_hash(key, peek())};
    }
};

class ApiKeysRouteHandlers
{
public:
    void handle_api_key_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "api_key_create", permission_for("api_key_create", req))) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::api_keys::CreateApiKeyRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const std::string merchant_id = merchant_from_request(req);
        if (merchant_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::UNAUTHORIZED, "missing merchant id"));
            return;
        }
        auto result = create_api_key(merchant_id, *parsed);
        if (!result) {
            send_result_error(res, send, result.error());
            return;
        }
        send_json(res, send, serde::Ser::encode_json(*result), true);
    }

    void handle_api_key_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "api_key_retrieve", permission_for("api_key_retrieve", req))) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string merchant_id = merchant_from_request(req);
        const std::string key_id = key_id_from_request(req);
        if (key_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing key id"));
            return;
        }
        auto result = retrieve_api_key(merchant_id, key_id);
        if (!result) {
            send_result_error(res, send, result.error());
            return;
        }
        m_context->metrics().record_flow("ApiKeyRetrieve");
        m_context->metrics().counter("API_KEY_RETRIEVED").increment();
        send_json(res, send, serde::Ser::encode_json(*result), true);
    }

    void handle_api_key_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "api_key_update", permission_for("api_key_update", req))) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::api_keys::UpdateApiKeyRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const std::string merchant_id = merchant_from_request(req);
        const std::string key_id = key_id_from_request(req);
        if (key_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing key id"));
            return;
        }
        auto result = update_api_key(merchant_id, key_id, *parsed);
        if (!result) {
            send_result_error(res, send, result.error());
            return;
        }
        m_context->metrics().record_flow("ApiKeyUpdate");
        m_context->metrics().counter("API_KEY_UPDATED").increment();
        send_json(res, send, serde::Ser::encode_json(*result), true);
    }

    void handle_api_key_revoke(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "api_key_revoke", permission_for("api_key_revoke", req))) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string merchant_id = merchant_from_request(req);
        const std::string key_id = key_id_from_request(req);
        if (key_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing key id"));
            return;
        }
        auto result = revoke_api_key(merchant_id, key_id);
        if (!result) {
            send_result_error(res, send, result.error());
            return;
        }
        m_context->metrics().record_flow("ApiKeyRevoke");
        m_context->metrics().counter("API_KEY_REVOKED").increment();
        send_json(res, send, serde::Ser::encode_json(*result), true);
    }

    void handle_api_key_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "api_key_list", permission_for("api_key_list", req))) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string merchant_id = merchant_from_request(req);
        const auto query = app::http::RequestQuery{}(req);

        app::dto::api_keys::ListApiKeyConstraints constraints;
        if (const auto raw = query.get("limit")) {
            std::int64_t limit{};
            const auto converted = std::from_chars(raw->data(), raw->data() + raw->size(), limit);
            if (converted.ec == std::errc{} && converted.ptr == raw->data() + raw->size()) {
                constraints.set_limit(limit);
            }
        }
        if (const auto raw = query.get("skip")) {
            std::int64_t skip{};
            const auto converted = std::from_chars(raw->data(), raw->data() + raw->size(), skip);
            if (converted.ec == std::errc{} && converted.ptr == raw->data() + raw->size()) {
                constraints.set_skip(skip);
            }
        }

        auto result = list_api_keys(
            merchant_id,
            constraints.get_limit().value_or(-1),
            constraints.get_skip().value_or(-1)
        );
        if (!result) {
            send_result_error(res, send, result.error());
            return;
        }
        m_context->metrics().record_flow("ApiKeyList");
        m_context->metrics().counter("API_KEY_LISTED").increment();
        send_json(res, send, serde::Ser::encode_json(*result), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        if constexpr (kV1) {
            // app.rs:2504-2513 — web::scope("/api_keys/{merchant_id}")
            Router{router, "api_keys"}
                .add_route(
                    Route{":merchant_id"}
                        .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                            handle_api_key_create(req, res, std::move(send));
                        })
                        .summary("Hyperswitch api_key_create")
                )
                .add_router(Router{router, ":merchant_id"}.add_route(
                    Route{"list"}
                        .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                            handle_api_key_list(req, res, std::move(send));
                        })
                        .summary("Hyperswitch api_key_list")
                ))
                .add_router(Router{router, ":merchant_id"}.add_route(
                    Route{":key_id"}
                        .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                            handle_api_key_retrieve(req, res, std::move(send));
                        })
                        .summary("Hyperswitch api_key_retrieve")
                ))
                .add_router(Router{router, ":merchant_id"}.add_route(
                    Route{":key_id"}
                        .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                            handle_api_key_update(req, res, std::move(send));
                        })
                        .summary("Hyperswitch api_key_update")
                ))
                .add_router(Router{router, ":merchant_id"}.add_route(
                    Route{":key_id"}
                        .delt([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                            handle_api_key_revoke(req, res, std::move(send));
                        })
                        .summary("Hyperswitch api_key_revoke")
                ));
        }

        if constexpr (kV2) {
            // app.rs:2488-2497 — web::scope("/v2/api-keys"), resource("") etc.
            Router{router, "v2/api-keys"}
                .add_route(
                    Route{""}
                        .post([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                            handle_api_key_create(req, res, std::move(send));
                        })
                        .summary("Hyperswitch api_key_create (v2)")
                )
                .add_route(
                    Route{"list"}
                        .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                            handle_api_key_list(req, res, std::move(send));
                        })
                        .summary("Hyperswitch api_key_list (v2)")
                )
                .add_route(
                    Route{":key_id"}
                        .get([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                            handle_api_key_retrieve(req, res, std::move(send));
                        })
                        .summary("Hyperswitch api_key_retrieve (v2)")
                )
                .add_route(
                    Route{":key_id"}
                        .put([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                            handle_api_key_update(req, res, std::move(send));
                        })
                        .summary("Hyperswitch api_key_update (v2)")
                )
                .add_route(
                    Route{":key_id"}
                        .delt([this](interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) {
                            handle_api_key_revoke(req, res, std::move(send));
                        })
                        .summary("Hyperswitch api_key_revoke (v2)")
                );
        }
    }

    explicit ApiKeysRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    using Response = app::dto::api_keys::CreateApiKeyResponse;
    using Retrieve = app::dto::api_keys::RetrieveApiKeyResponse;
    using Revoke = app::dto::api_keys::RevokeApiKeyResponse;
    using Expiration = app::dto::api_keys::ApiKeyExpiration;
    using ApiKeyRecord = app::store::api_keys::ApiKeyRecord;
    using Error = core::error::AppError;

    [[nodiscard]] Error make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    /** @brief Exact ApiErrorResponse::ApiKeyNotFound mapping
     * (api_error_response.rs:495-497): NotFound, ApiError::new("HE", 2, ...). */
    [[nodiscard]] Error api_key_not_found_error() const
    {
        return make_error(core::error::AppErrorKind::NOT_FOUND, "API Key does not exist in our records");
    }

    [[nodiscard]] static std::string json_escape(std::string_view value)
    {
        std::string out{};
        out.reserve(value.size() + 2);
        for (const char c : value) {
            switch (c) {
                case '"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default: out.push_back(c); break;
            }
        }
        return out;
    }

    [[nodiscard]] bool is_v2(interfaces::io::IRequest& req) const
    {
        return app::http::RequestPath{}(req).at(0).value_or("") == "v2";
    }

    [[nodiscard]] std::string merchant_from_request(interfaces::io::IRequest& req) const
    {
        if (is_v2(req)) {
            return app::http::merchant_id_from_header{}(req);
        }
        return std::string{app::http::RequestPath{}(req).at(1).value_or("")};
    }

    [[nodiscard]] std::string key_id_from_request(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() >= 3) {
            return std::string{segments.at(2).value_or("")};
        }
        return {};
    }

    /**
     * @brief Port of the per-operation Permission from routes/api_keys.rs.
     * v2 update intentionally uses read permission (routes/api_keys.rs:235).
     */
    [[nodiscard]] app::auth::AppPermission permission_for(
        std::string_view operation, interfaces::io::IRequest& req
    ) const
    {
        const bool write = operation == "api_key_create" || operation == "api_key_revoke" ||
                           (operation == "api_key_update" && !is_v2(req));
        return write ? app::auth::AppPermission::API_KEY_WRITE
                     : app::auth::AppPermission::API_KEY_READ;
    }

    /**
     * @brief Port of auth_type() (services/authentication.rs:6464-6475): a JWT
     * bearer token selects the JWT merchant guard; otherwise the default
     * platform-admin (v1) / header-merchant (v2) guard is used. Guard flags are
     * the upstream `allow_connected`/`allow_platform` values.
     */
    [[nodiscard]] std::optional<Error> authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation,
        app::auth::AppPermission permission
    ) const
    {
        const auto registered = m_context->auth_rules().rule_for(operation);
        if (registered.scheme() == app::auth::AuthScheme::FORBIDDEN) {
            return make_error(core::error::AppErrorKind::FORBIDDEN, "operation is not registered in the auth rule registry");
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const bool jwt = !auth.token().is_empty();
        const bool v2 = is_v2(req);

        app::auth::RouteAuthRule rule;
        if (jwt) {
            rule = app::auth::RouteAuthRuleBuilder{}
                       .scheme(app::auth::AuthScheme::JWT)
                       .permission(permission)
                       .allow_connected(true)
                       .allow_platform(true)
                       .build();
        } else if (v2) {
            rule = app::auth::RouteAuthRuleBuilder{}
                       .scheme(app::auth::AuthScheme::HEADER_MERCHANT)
                       .permission(permission)
                       .allow_connected(true)
                       .allow_platform(false)
                       .build();
        } else {
            rule = app::auth::RouteAuthRuleBuilder{}
                       .scheme(app::auth::AuthScheme::PLATFORM_ADMIN)
                       .permission(permission)
                       .allow_connected(true)
                       .allow_platform(true)
                       .build();
        }
        return m_context->authorizer().authorize_route(auth, rule);
    }

    [[nodiscard]] std::int64_t now_epoch() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    /** @brief Civil date from days since 1970-01-01; deterministic UTC formatting. */
    [[nodiscard]] std::string iso_utc(std::int64_t epoch_seconds) const
    {
        constexpr std::int64_t seconds_per_day{86400};
        std::int64_t days = epoch_seconds / seconds_per_day;
        std::int64_t seconds_of_day = epoch_seconds % seconds_per_day;
        if (seconds_of_day < 0) {
            seconds_of_day += seconds_per_day;
            --days;
        }
        days += 719468;
        const std::int64_t era = (days >= 0 ? days : days - 146096) / 146097;
        const std::int64_t doe = days - era * 146097;
        const std::int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        std::int64_t year = yoe + era * 400;
        const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const std::int64_t mp = (5 * doy + 2) / 153;
        const std::int64_t day = doy - (153 * mp + 2) / 5 + 1;
        const std::int64_t month = mp < 10 ? mp + 3 : mp - 9;
        year += (month <= 2) ? 1 : 0;
        return std::format(
            "{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}Z",
            year, month, day,
            seconds_of_day / 3600,
            (seconds_of_day % 3600) / 60,
            seconds_of_day % 60
        );
    }

    /** @brief Inverse of iso_utc; accepts an optional fractional-seconds tail. */
    [[nodiscard]] std::optional<std::int64_t> parse_iso_utc(std::string_view text) const
    {
        if (text.size() < 20 || text[4] != '-' || text[7] != '-' || text[10] != 'T' ||
            text[13] != ':' || text[16] != ':') {
            return std::nullopt;
        }
        const auto number = [](std::string_view part) -> std::optional<int> {
            int value{};
            const auto result = std::from_chars(part.data(), part.data() + part.size(), value);
            if (result.ec != std::errc{} || result.ptr != part.data() + part.size()) { return std::nullopt; }
            return value;
        };
        const auto year = number(text.substr(0, 4));
        const auto month = number(text.substr(5, 2));
        const auto day = number(text.substr(8, 2));
        const auto hour = number(text.substr(11, 2));
        const auto minute = number(text.substr(14, 2));
        const auto second = number(text.substr(17, 2));
        if (!year || !month || !day || !hour || !minute || !second) { return std::nullopt; }

        int y = *year - (*month <= 2 ? 1 : 0);
        const int era = (y >= 0 ? y : y - 399) / 400;
        const int yoe = y - era * 400;
        const int mp = *month + (*month > 2 ? -3 : 9);
        const int doy = (153 * mp + 2) / 5 + *day - 1;
        const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
        const std::int64_t epoch_days = static_cast<std::int64_t>(era) * 146097 + doe - 719468;
        return epoch_days * 86400 + static_cast<std::int64_t>(*hour) * 3600 +
               static_cast<std::int64_t>(*minute) * 60 + *second;
    }

    [[nodiscard]] std::string env_prefix() const
    {
        const auto configured = m_context->config_store().get_value("api_keys", "env");
        return std::string{prefix_for_env(configured.value_or("sandbox"))};
    }

    /** @brief Port of settings::ApiKeys::get_hash_key (core/api_keys.rs:29-45). */
    [[nodiscard]] std::expected<app::crypto::SecretString, Error> get_hash_key() const
    {
        constexpr std::string_view default_hex =
            "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f";
        const auto resolved = m_context->secret_resolver().resolve("api_key_hash", "default");
        const std::string hex = resolved.value_or(std::string{default_hex});

        const auto nibble = [](char c) -> int {
            if (c >= '0' && c <= '9') { return c - '0'; }
            if (c >= 'a' && c <= 'f') { return c - 'a' + 10; }
            if (c >= 'A' && c <= 'F') { return c - 'A' + 10; }
            return -1;
        };
        if (hex.size() % 2 != 0) {
            return std::unexpected{make_error(core::error::AppErrorKind::INTERNAL_ERROR,
                                              "API key hash key has invalid hexadecimal data")};
        }
        std::string bytes{};
        bytes.reserve(hex.size() / 2);
        for (std::size_t i = 0; i < hex.size(); i += 2) {
            const int high = nibble(hex[i]);
            const int low = nibble(hex[i + 1]);
            if (high < 0 || low < 0) {
                return std::unexpected{make_error(core::error::AppErrorKind::INTERNAL_ERROR,
                                                  "API key hash key has invalid hexadecimal data")};
            }
            bytes.push_back(static_cast<char>((high << 4) | low));
        }
        if (bytes.size() != kHashKeyLen) {
            return std::unexpected{make_error(core::error::AppErrorKind::INTERNAL_ERROR,
                                              "The API hashing key has incorrect length")};
        }
        return app::crypto::SecretString{std::move(bytes)};
    }

    [[nodiscard]] std::string expiration_string(const Expiration& expiration) const
    {
        return expiration.is_never() ? std::string{"never"} : expiration.value();
    }

    [[nodiscard]] Expiration expiration_enum(std::string_view value) const
    {
        return value == "never" ? Expiration::never() : Expiration::at(std::string{value});
    }

    /** @brief Option<PrimitiveDateTime> conversion of ApiKeyExpiration (api_models:181-188). */
    [[nodiscard]] std::optional<std::string> expires_at(const Expiration& expiration) const
    {
        return expiration.is_never() ? std::nullopt : std::optional<std::string>{expiration.value()};
    }

    /** @brief authentication::decision::convert_expiry (decision.rs:181-192). */
    [[nodiscard]] std::uint64_t convert_expiry(const std::optional<std::string>& expires) const
    {
        if (!expires) { return 0U; }
        const auto target = parse_iso_utc(*expires);
        if (!target) { return 0U; }
        const std::int64_t delta = *target - now_epoch();
        return delta <= 0 ? 0U : static_cast<std::uint64_t>(delta);
    }

    // ── Email expiry workflow (core/api_keys.rs:191-258,371-433,499-551) ───────

    [[nodiscard]] std::vector<std::int64_t> expiry_reminder_days() const
    {
        std::vector<std::int64_t> days{7, 3, 1}; // configs/defaults.rs:146
        const auto configured = m_context->config_store().get_value("api_keys", "expiry_reminder_days");
        if (!configured || configured->empty()) { return days; }
        std::vector<std::int64_t> parsed{};
        for (const auto part : std::views::split(*configured, ',')) {
            std::int64_t value{};
            const std::string text{std::ranges::begin(part), std::ranges::end(part)};
            const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
            if (result.ec == std::errc{} && result.ptr == text.data() + text.size() && value > 0) {
                parsed.push_back(value);
            }
        }
        if (!parsed.empty()) { days = std::move(parsed); }
        return days;
    }

    [[nodiscard]] std::string generate_task_id_for_api_key_expiry_workflow(std::string_view key_id) const
    {
        return std::format("{}_{}_{}", kExpiryRunner, kExpiryName, key_id);
    }

    [[nodiscard]] std::int64_t find_expiry_tracker(std::string_view key_id) const
    {
        const auto stored = m_context->config_store().get_value(
            "api_key_expiry", generate_task_id_for_api_key_expiry_workflow(key_id)
        );
        if (!stored) { return 0; }
        std::int64_t tracker_id{};
        const auto result = std::from_chars(stored->data(), stored->data() + stored->size(), tracker_id);
        if (result.ec != std::errc{} || result.ptr != stored->data() + stored->size()) { return 0; }
        return tracker_id;
    }

    [[nodiscard]] std::string api_key_expiry_tracking_json(
        const ApiKeyRecord& key, const std::vector<std::int64_t>& days, const std::string& expires
    ) const
    {
        std::string out = std::format(
            R"({{"key_id":"{}","merchant_id":"{}","api_key_name":"{}","prefix":"{}","api_key_expiry":"{}","expiry_reminder_days":[)",
            json_escape(key.get_key_id()), json_escape(key.get_merchant_id()),
            json_escape(key.get_name()), json_escape(key.get_prefix()), json_escape(expires)
        );
        for (std::size_t i = 0; i < days.size(); ++i) {
            if (i != 0) { out.push_back(','); }
            out += std::to_string(days[i]);
        }
        out += "]}";
        return out;
    }

    /** @brief core::api_keys::add_api_key_expiry_task (197-258). */
    [[nodiscard]] std::optional<Error> add_api_key_expiry_task(
        const ApiKeyRecord& key, const std::optional<std::string>& expires
    ) const
    {
        if (!expires) { return std::nullopt; }
        const auto expiry_epoch = parse_iso_utc(*expires);
        if (!expiry_epoch) {
            return make_error(core::error::AppErrorKind::INTERNAL_ERROR,
                              "Failed to obtain initial process tracker schedule time");
        }
        const auto days = expiry_reminder_days();
        if (days.empty()) {
            return make_error(core::error::AppErrorKind::INTERNAL_ERROR,
                              "Failed to obtain initial process tracker schedule time");
        }
        const std::int64_t schedule_epoch = *expiry_epoch - days.front() * 86400;
        if (schedule_epoch <= now_epoch()) { return std::nullopt; } // first email in the past

        const std::string task_id = generate_task_id_for_api_key_expiry_workflow(key.get_key_id());
        const std::string payload = api_key_expiry_tracking_json(key, days, *expires);
        const std::int64_t tracker_id = m_context->process_trackers().schedule(
            std::string{kExpiryName}, payload, iso_utc(schedule_epoch)
        );
        m_context->config_store().put("api_key_expiry", task_id, std::to_string(tracker_id));
        m_context->metrics().counter("TASKS_ADDED_COUNT").increment();
        return std::nullopt;
    }

    /** @brief core::api_keys::update_api_key_expiry_task (376-433) + the
     * find/revoke/select branch of update_api_key (314-366). */
    [[nodiscard]] std::optional<Error> update_api_key_expiry_task(
        const ApiKeyRecord& key, const std::optional<std::string>& expires
    ) const
    {
        const std::int64_t existing = find_expiry_tracker(key.get_key_id());
        if (existing != 0) {
            if (expires) {
                (void)m_context->process_trackers().mark_terminal(existing, "superseded");
                m_context->config_store().remove(
                    "api_key_expiry", generate_task_id_for_api_key_expiry_workflow(key.get_key_id())
                );
                return add_api_key_expiry_task(key, expires);
            }
            return revoke_api_key_expiry_task(key.get_key_id());
        }
        if (expires) { return add_api_key_expiry_task(key, expires); }
        return std::nullopt;
    }

    /** @brief core::api_keys::revoke_api_key_expiry_task (503-520): StatusUpdate
     * to Finish with business_status REVOKED (local: mark_terminal "REVOKED"). */
    [[nodiscard]] std::optional<Error> revoke_api_key_expiry_task(std::string_view key_id) const
    {
        const std::int64_t tracker_id = find_expiry_tracker(key_id);
        if (tracker_id == 0) { return std::nullopt; }
        (void)m_context->process_trackers().mark_terminal(tracker_id, std::string{kExpiryTag} + "_REVOKED");
        m_context->config_store().remove(
            "api_key_expiry", generate_task_id_for_api_key_expiry_workflow(key_id)
        );
        return std::nullopt;
    }

    // ── Auth decision cache sync (authentication::decision.rs:11-223) ───────────

    struct DecisionConfig
    {
        std::string base_url{};
        std::string tag{};
    };

    [[nodiscard]] DecisionConfig decision_config() const
    {
        return DecisionConfig{
            m_context->config_store().get_value("decision", "base_url").value_or("http://127.0.0.1:9090"),
            m_context->config_store().get_value("tenant", "schema").value_or("public")
        };
    }

    /** @brief RuleRequest / AuthRuleType / Identifiers wire shape (decision.rs:24-73). */
    [[nodiscard]] std::string rule_request_json(
        const DecisionConfig& config,
        std::string_view hashed_api_key,
        std::string_view merchant_id,
        std::string_view key_id,
        const std::optional<std::uint64_t>& expiry
    ) const
    {
        const std::string expiry_part = expiry ? std::to_string(*expiry) : std::string{"null"};
        return std::format(
            R"({{"tag":"{}","type":"api_key","api_key":"{}","identifiers":{{"type":"api_key","merchant_id":"{}","key_id":"{}"}},"expiry":{}}})",
            json_escape(config.tag), json_escape(hashed_api_key),
            json_escape(merchant_id), json_escape(key_id), expiry_part
        );
    }

    [[nodiscard]] std::string rule_delete_json(
        const DecisionConfig& config, std::string_view hashed_api_key
    ) const
    {
        return std::format(
            R"({{"tag":"{}","type":"api_key","api_key":"{}"}})",
            json_escape(config.tag), json_escape(hashed_api_key)
        );
    }

    /** @brief spawn_tracked_job (decision.rs:194-223): detached tracked work with
     * API_KEY_REQUEST_INITIATED / ..._COMPLETED counters. */
    void spawn_tracked_job(std::string_view request_type, std::string payload) const
    {
        m_context->metrics().counter("API_KEY_REQUEST_INITIATED").increment();
        m_context->process_trackers().schedule(
            std::string{"api_key_decision_"} + std::string{request_type},
            std::move(payload),
            {}
        );
        m_context->metrics().counter("API_KEY_REQUEST_COMPLETED").increment();
    }

    /** @brief authentication::decision::add_api_key (decision.rs:78-104).
     * The external decision-service HTTP call is substituted in-process by
     * persisting the exact RuleRequest body in the shared cache under the
     * hashed-key decision key; the tracked job is recorded identically. */
    void add_api_key_decision(
        std::string_view hashed_api_key,
        std::string_view merchant_id,
        std::string_view key_id,
        const std::optional<std::string>& expires
    ) const
    {
        const auto config = decision_config();
        std::optional<std::uint64_t> expiry{};
        if (expires) { expiry = convert_expiry(expires); }
        const std::string body = rule_request_json(config, hashed_api_key, merchant_id, key_id, expiry);
        const std::string cache_key{std::string{"api_keys.decision."} + std::string{hashed_api_key}};
        m_context->cache().put(cache_key, core::cache::to_cache_value(body));
        spawn_tracked_job(kDecisionAdd, body);
    }

    /** @brief authentication::decision::revoke_api_key (decision.rs:163-179). */
    void revoke_api_key_decision(std::string_view hashed_api_key) const
    {
        const auto config = decision_config();
        const std::string body = rule_delete_json(config, hashed_api_key);
        const std::string cache_key{std::string{"api_keys.decision."} + std::string{hashed_api_key}};
        (void)m_context->cache().erase(cache_key);
        spawn_tracked_job(kDecisionRevoke, body);
    }

    // ── Persistence mirror of the upstream ApiKey expiry column ────────────────

    void upsert_expiry_record(
        const ApiKeyRecord& key, const std::optional<std::string>& expires
    ) const
    {
        auto repository = m_context->repository();
        const std::string expires_value = expires.value_or("");
        if (auto existing = repository.find_api_key_expiry_by_string(key.get_key_id())) {
            existing->set_expiration(key.get_expiration());
            existing->set_expires_at(expires_value);
            existing->set_revoked(key.get_revoked());
            repository.update_api_key_expiry(*existing);
            return;
        }
        app::store::api_key_expiries::ApiKeyExpiryRecord fresh;
        fresh.set_key_id(key.get_key_id());
        fresh.set_merchant_id(key.get_merchant_id());
        fresh.set_expiration(key.get_expiration());
        fresh.set_expires_at(expires_value);
        fresh.set_revoked(key.get_revoked());
        fresh.set_created_at(key.get_created_at());
        repository.save_api_key_expiry(std::move(fresh));
    }

    // ── core/api_keys.rs named functions, ported as local helpers ───────────────

    /** @brief core::api_keys::create_api_key (112-189). */
    [[nodiscard]] std::expected<Response, Error> create_api_key(
        const std::string& merchant_id, const app::dto::api_keys::CreateApiKeyRequest& input
    ) const
    {
        auto hash_key = get_hash_key();
        if (!hash_key) { return std::unexpected{hash_key.error()}; }

        const std::string env = env_prefix();
        const PlaintextApiKey plaintext = PlaintextApiKey::new_(kApiKeyLength, env);
        const std::string key_id = PlaintextApiKey::new_key_id(env);
        const HashedApiKey hashed = plaintext.keyed_hash(hash_key->value());
        const std::int64_t created_epoch = now_epoch();
        const std::string created_at = iso_utc(created_epoch);
        const std::optional<std::string> expires = expires_at(input.get_expiration());

        ApiKeyRecord record;
        record.set_key_id(key_id);
        record.set_merchant_id(merchant_id);
        record.set_name(input.get_name());
        record.set_description(input.get_description().value_or(""));
        record.set_prefix(plaintext.prefix());
        record.set_api_key(hashed.peek());
        record.set_expiration(expiration_string(input.get_expiration()));
        record.set_created_at(created_at);
        record.set_revoked(false);

        auto inserted = m_context->repository().create_api_key(std::move(record));
        if (!inserted) { return std::unexpected{inserted.error()}; }
        const ApiKeyRecord saved = *inserted;

        upsert_expiry_record(saved, expires);

        // decision ADD tracked job (core/api_keys.rs:148-160).
        add_api_key_decision(saved.get_api_key(), saved.get_merchant_id(), saved.get_key_id(), expires);

        // metrics::API_KEY_CREATED.add(1, merchant attribute) (core:162-165).
        m_context->metrics().record_flow("ApiKeyCreate");
        m_context->metrics().counter("API_KEY_CREATED").increment();

        if constexpr (kEmail) {
            if (expires) {
                if (auto expiry_error = add_api_key_expiry_task(saved, expires)) {
                    return std::unexpected{*expiry_error};
                }
            }
        }

        Response response;
        app::dto::api_keys::ApiKeyIdValue response_key_id;
        response_key_id.set_value(saved.get_key_id());
        response.set_key_id(std::move(response_key_id));
        app::dto::api_keys::MerchantIdValue response_merchant_id;
        response_merchant_id.set_value(saved.get_merchant_id());
        response.set_merchant_id(std::move(response_merchant_id));
        response.set_name(saved.get_name());
        response.set_description(
            saved.get_description().empty()
                ? std::nullopt
                : std::optional<std::string>{saved.get_description()}
        );
        response.set_api_key(plaintext.peek());
        app::dto::api_keys::PrimitiveDateTimeValue created;
        created.set_value(saved.get_created_at());
        response.set_created(std::move(created));
        response.set_expiration(input.get_expiration());
        return response;
    }

    /** @brief core::api_keys::retrieve_api_key (261-275). */
    [[nodiscard]] std::expected<Retrieve, Error> retrieve_api_key(
        const std::string& merchant_id, const std::string& key_id
    ) const
    {
        const auto found = find_api_key(merchant_id, key_id);
        if (!found) { return std::unexpected{api_key_not_found_error()}; }
        return to_retrieve_response(*found);
    }

    /** @brief core::api_keys::update_api_key (278-369). */
    [[nodiscard]] std::expected<Retrieve, Error> update_api_key(
        const std::string& merchant_id,
        const std::string& key_id,
        const app::dto::api_keys::UpdateApiKeyRequest& input
    ) const
    {
        const auto found = find_api_key(merchant_id, key_id);
        if (!found) { return std::unexpected{api_key_not_found_error()}; }

        ApiKeyRecord record = *found;
        if (input.get_name()) { record.set_name(*input.get_name()); }
        if (input.get_description()) { record.set_description(*input.get_description()); }
        const bool expiration_changed = input.get_expiration().has_value();
        if (expiration_changed) { record.set_expiration(expiration_string(*input.get_expiration())); }
        record.set_key_id(key_id);
        record.set_merchant_id(merchant_id);
        if (!m_context->repository().update_api_key(record)) {
            return std::unexpected{api_key_not_found_error()};
        }

        const Expiration effective = expiration_changed
            ? *input.get_expiration()
            : expiration_enum(record.get_expiration());
        const std::optional<std::string> expires = expires_at(effective);
        upsert_expiry_record(record, expires);

        // decision ADD on update (core/api_keys.rs:300-312).
        add_api_key_decision(record.get_api_key(), merchant_id, key_id, expires);

        if constexpr (kEmail) {
            if (auto expiry_error = update_api_key_expiry_task(record, expires)) {
                return std::unexpected{*expiry_error};
            }
        }
        return to_retrieve_response(record);
    }

    /** @brief core::api_keys::revoke_api_key (436-497). */
    [[nodiscard]] std::expected<Revoke, Error> revoke_api_key(
        const std::string& merchant_id, const std::string& key_id
    ) const
    {
        const auto found = find_api_key(merchant_id, key_id);
        if (!found) { return std::unexpected{api_key_not_found_error()}; }

        ApiKeyRecord record = *found;
        record.set_revoked(true);
        if (!m_context->repository().update_api_key(record)) {
            return std::unexpected{api_key_not_found_error()};
        }

        upsert_expiry_record(record, std::nullopt);

        // decision REVOKE (core/api_keys.rs:453-464).
        revoke_api_key_decision(record.get_api_key());

        if constexpr (kEmail) {
            if (auto expiry_error = revoke_api_key_expiry_task(key_id)) {
                return std::unexpected{*expiry_error};
            }
        }

        Revoke response;
        app::dto::api_keys::MerchantIdValue response_merchant_id;
        response_merchant_id.set_value(record.get_merchant_id());
        response.set_merchant_id(std::move(response_merchant_id));
        app::dto::api_keys::ApiKeyIdValue response_key_id;
        response_key_id.set_value(record.get_key_id());
        response.set_key_id(std::move(response_key_id));
        response.set_revoked(record.get_revoked());
        return response;
    }

    /** @brief core::api_keys::list_api_keys (523-541): merchant-scoped query with
     * limit/offset pushed into the database query, not an in-memory filter. */
    [[nodiscard]] std::expected<std::vector<Retrieve>, Error> list_api_keys(
        const std::string& merchant_id, std::int64_t limit, std::int64_t offset
    ) const
    {
        core::db::QuerySpec spec = m_context->database().query<ApiKeyRecord>().spec();
        spec.predicates.push_back(core::db::Predicate{
            .column = "merchant_id",
            .comparison = core::db::Compare::EQ,
            .value = core::db::DbValue{merchant_id}
        });
        if (limit >= 0) { spec.limit_count = static_cast<std::size_t>(limit); }
        if (offset >= 0) { spec.offset_count = static_cast<std::size_t>(offset); }

        const std::vector<ApiKeyRecord> records = m_context->database().collect<ApiKeyRecord>(spec);
        std::vector<Retrieve> responses{};
        responses.reserve(records.size());
        for (const auto& record : records) {
            responses.push_back(to_retrieve_response(record));
        }
        return responses;
    }

    [[nodiscard]] std::optional<ApiKeyRecord> find_api_key(
        const std::string& merchant_id, const std::string& key_id
    ) const
    {
        if (key_id.empty()) { return std::nullopt; }
        auto repository = m_context->repository();
        if (!merchant_id.empty()) {
            return repository.find_api_key_by_merchant_id_and_key_id(merchant_id, key_id);
        }
        return repository.find_api_key_by_string(key_id);
    }

    [[nodiscard]] Retrieve to_retrieve_response(const ApiKeyRecord& record) const
    {
        Retrieve response;
        app::dto::api_keys::ApiKeyIdValue key_id;
        key_id.set_value(record.get_key_id());
        response.set_key_id(std::move(key_id));
        app::dto::api_keys::MerchantIdValue merchant_id;
        merchant_id.set_value(record.get_merchant_id());
        response.set_merchant_id(std::move(merchant_id));
        response.set_name(record.get_name());
        response.set_description(
            record.get_description().empty()
                ? std::nullopt
                : std::optional<std::string>{record.get_description()}
        );
        response.set_prefix(record.get_prefix());
        app::dto::api_keys::PrimitiveDateTimeValue created;
        created.set_value(record.get_created_at());
        response.set_created(std::move(created));
        response.set_expiration(expiration_enum(record.get_expiration()));
        return response;
    }

    void send_result_error(
        interfaces::io::IResponse& res, std::function<void()> send, const Error& error
    ) const
    {
        if (error.get_message() == "API Key does not exist in our records") {
            // Exact upstream AER::NotFound(ApiError::new("HE", 2, ...)) body.
            constexpr std::string_view body =
                R"({"type":"api","message":"API Key does not exist in our records","code":"HE_02"})";
            res.set_status(interfaces::io::types::Status::NOT_FOUND);
            res.set_body(serde::Ser::serialize_raw("application/json", body));
            send();
            return;
        }
        send_error(res, send, error);
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        Error error
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

} // namespace app::handlers::api_keys
