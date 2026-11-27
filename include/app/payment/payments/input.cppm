export module app_payment:payments_input;

import std;
import core_behavior;
import interfaces;
import core_error;
import core_serde;
import app_context;
import app_dto;
import app_model;
import app_auth;
import app_http;
import app_store;
import app_crypto;
import :context;

export namespace app::handlers::payments {

/**
 *  Upstream ApiErrorResponse variants are represented as AppError values: the
 *  upstream code (`IR_04`, `HE_00`, ...) is carried in `kind_name`, the real
 *  message in `message`, and `kind` is chosen so `get_status()` reproduces the
 *  upstream HTTP status. The handler serializes this value directly.
 */
using PaymentsApiError = core::error::AppError;

/**  Shared in-process mirror of upstream profile-scoped payment intent keys. */
class PaymentsScopeState
{
public:
    void remember(std::string_view merchant, std::string_view id, std::string profile)
    {
        m_values[std::format("{}|{}", merchant, id)] = std::move(profile);
    }

    [[nodiscard]] std::optional<std::string>
    lookup(std::string_view merchant, std::string_view id) const
    {
        const auto it = m_values.find(std::format("{}|{}", merchant, id));
        return it == m_values.end() ? std::nullopt : std::optional<std::string>{it->second};
    }

private:
    std::unordered_map<std::string, std::string> m_values{};
};

/**  HTTP-free view of everything the payments runtime helpers consume. */
struct PaymentsRequestData
{
    std::string body{};
    std::string payment_id{};
    std::string merchant_id{};
    std::string merchant_id_path{};
    std::string profile_id{};
    std::vector<std::string> path_segments{};
    std::unordered_map<std::string, std::string> query{};
};

/**
 *  @brief Parse/auth/path/query support for the payments handlers.
 *
 *  No business logic lives here: request data becomes the plain
 *  PaymentsRequestData consumed by the domain runtime, and auth failures become
 *  a typed AppError the handler serializes directly.
 */
class PaymentsInputSupport
{
public:
    explicit PaymentsInputSupport(
        ::app::context::AppContext& context,
        PaymentsScopeState& scope
    ) noexcept :
        m_context{context},
        m_scope{scope}
    {
    }

    // ───────────────────────── Path / query helpers ─────────────────────────
    [[nodiscard]] std::string payment_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i = 0; i < segments.count(); ++i) {
            const auto segment = segments.at(i);
            if (segment && segment->starts_with("pay_")) {
                return std::string{*segment};
            }
        }
        return {};
    }

    [[nodiscard]] std::string merchant_id_from_path(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i = 0; i + 2 < segments.count(); ++i) {
            const auto current = segments.at(i);
            const auto after = segments.at(i + 1);
            if (current && *current == "payments" && after && after->starts_with("pay_")) {
                return std::string{*segments.at(i + 2)};
            }
        }
        return {};
    }

    [[nodiscard]] std::string
    last_path_segment(interfaces::io::IRequest& req, std::size_t offset = 0) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        if (segments.count() <= offset) {
            return {};
        }
        return std::string{*segments.at(segments.count() - 1 - offset)};
    }

    [[nodiscard]] std::string query_value(
        interfaces::io::IRequest& req,
        std::string_view key,
        std::string_view fallback = {}
    ) const
    {
        return ::app::http::RequestQuery{}(req).get(key).value_or(std::string{fallback});
    }

    [[nodiscard]] std::string query_uint(
        interfaces::io::IRequest& req,
        std::string_view key,
        std::string_view fallback = "0"
    ) const
    {
        const std::string raw = query_value(req, key, fallback);
        std::string digits{};
        for (const char ch: raw) {
            if (ch >= '0' && ch <= '9') {
                digits.push_back(ch);
            }
        }
        return digits.empty() ? std::string{fallback} : digits;
    }

    /**  Build a typed JSON document for path-derived runtime body fields. */
    [[nodiscard]] std::string
    path_body(std::initializer_list<std::pair<std::string_view, std::string_view>> fields) const
    {
        serde::Value::Object object{};
        for (const auto& [key, value]: fields) {
            object[std::string{key}] = serde::Value{std::string{value}};
        }
        const auto bytes =
            serde::Ser::serialize("application/json", serde::Value{std::move(object)});
        return std::string{reinterpret_cast<const char*>(bytes.data()), bytes.size()};
    }

    template<class T>
    [[nodiscard]] std::optional<T> parse_query_as(
        interfaces::io::IRequest& req,
        std::initializer_list<std::string_view> keys
    ) const
    {
        std::string json{"{"};
        bool first{true};
        for (const auto key: keys) {
            const std::string value = query_value(req, key);
            if (value.empty()) {
                continue;
            }
            if (!first) {
                json.push_back(',');
            }
            first = false;
            json += serde::Ser::encode_json(std::string{key});
            json.push_back(':');
            if (value == "true" || value == "false") {
                json += value;
            } else {
                bool integral{!value.empty()};
                for (const char ch: value) {
                    if (ch < '0' || ch > '9') {
                        integral = false;
                        break;
                    }
                }
                json += integral ? value : serde::Ser::encode_json(value);
            }
        }
        json.push_back('}');
        const auto decoded = serde::Ser::deserialize<T>("application/json", json);
        return decoded ? std::optional<T>{*decoded} : std::nullopt;
    }

    [[nodiscard]] bool
    query_bool(interfaces::io::IRequest& req, std::string_view key, bool fallback = false) const
    {
        const std::string raw = query_value(req, key);
        if (raw == "true" || raw == "1") {
            return true;
        }
        if (raw == "false" || raw == "0") {
            return false;
        }
        return fallback;
    }

    /**  Extract the entire HTTP-free request view consumed by the runtime. */
    [[nodiscard]] PaymentsRequestData request_data(interfaces::io::IRequest& req) const
    {
        PaymentsRequestData data{};
        data.body = req.body_as_string();
        const auto segments = ::app::http::RequestPath{}(req);
        for (std::size_t i = 0; i < segments.count(); ++i) {
            if (const auto segment = segments.at(i)) {
                data.path_segments.push_back(std::string{*segment});
            }
        }
        data.payment_id = payment_id_from_path(req);
        data.merchant_id_path = merchant_id_from_path(req);
        data.merchant_id = ::app::http::merchant_id_from_header{}(req);
        data.profile_id = profile_for(req);
        const auto query = ::app::http::RequestQuery{}(req);
        for (const std::string_view key: kQueryKeys) {
            if (const auto value = query.get(key)) {
                data.query[std::string{key}] = *value;
            }
        }
        return data;
    }

    // ───────────────────────── Error wire translation ─────────────────────────
    [[nodiscard]] PaymentsApiError api_error(
        core::error::AppErrorKind kind,
        std::string_view message,
        std::string_view payment_id = {}
    ) const
    {
        using Kind = core::error::AppErrorKind;
        switch (kind) {
            case Kind::EMPTY_REQUEST:
                return api_error_variant(
                    ApiVariant::BadRequest,
                    std::format("Missing required param: {}", message),
                    "IR_04",
                    payment_id
                );
            case Kind::INVALID_JSON:
                return api_error_variant(
                    ApiVariant::BadRequest,
                    std::format("Invalid value provided: {}", message),
                    "IR_06",
                    payment_id
                );
            case Kind::NOT_FOUND:
                return api_error_variant(
                    ApiVariant::NotFound,
                    "Payment does not exist in our records",
                    "HE_02",
                    payment_id
                );
            case Kind::UNAUTHORIZED:
                return api_error_variant(ApiVariant::Unauthorized, message, "IR_08", payment_id);
            case Kind::FORBIDDEN:
                return api_error_variant(
                    ApiVariant::ForbiddenCommonResource,
                    std::format("Access forbidden. {}", message),
                    "IR_22",
                    payment_id
                );
            case Kind::INTERNAL:
            case Kind::INTERNAL_ERROR:
                return api_error_variant(
                    ApiVariant::InternalServerError,
                    "Something went wrong",
                    "HE_00",
                    payment_id
                );
            case Kind::ALREADY_EXISTS:
                return api_error_variant(ApiVariant::Conflict, message, "IR_12", payment_id);
            case Kind::PRECONDITION_FAILED:
                return api_error_variant(ApiVariant::Conflict, message, "IR_14", payment_id);
            case Kind::INVALID_REQUEST:
                return api_error_variant(
                    ApiVariant::BadRequest,
                    std::format("Invalid value provided: {}", message),
                    "IR_06",
                    payment_id
                );
            case Kind::DUPLICATE_REQUEST:
                return api_error_variant(
                    ApiVariant::Unprocessable,
                    "Resource is busy. Please try again later.",
                    "HE_00",
                    payment_id
                );
            case Kind::CONNECTOR_ERROR:
                return api_error_variant(ApiVariant::ConnectorError, message, "CE_00", payment_id);
            case Kind::VALIDATION_FAILED:
                return api_error_variant(
                    ApiVariant::Unprocessable,
                    std::format("This Payment could not be processed: {}", message),
                    "IR_14",
                    payment_id
                );
            case Kind::RATE_LIMITED:
                return api_error_variant(ApiVariant::TooManyRequests, message, "HE_00", payment_id);
            case Kind::TIMEOUT:
                return api_error_variant(ApiVariant::Unprocessable, message, "HE_00", payment_id);
        }
        return api_error_variant(
            ApiVariant::InternalServerError,
            "Something went wrong",
            "HE_00",
            payment_id
        );
    }

    [[nodiscard]] PaymentsApiError
    api_error(const core::error::AppError& error, std::string_view payment_id = {}) const
    {
        return api_error(error.get_kind(), error.get_message(), payment_id);
    }

    [[nodiscard]] std::string metadata_value(std::string_view metadata, std::string_view key) const
    {
        const std::string needle = std::format(R"("{}":")", key);
        const auto pos = metadata.find(needle);
        if (pos == std::string_view::npos) {
            return {};
        }
        const auto start = pos + needle.size();
        const auto end = metadata.find('"', start);
        return end == std::string_view::npos ? std::string{}
                                             : std::string{metadata.substr(start, end - start)};
    }

    // ───────────────────────── Authorization ─────────────────────────
    [[nodiscard]] std::optional<PaymentsApiError>
    authorize(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth = m_context.authorizer().authorize_route(ctx, rule)) {
            return api_error(*auth);
        }
        if (!apply_extractor(req, ctx, auth_spec_for(operation), operation)) {
            return unauthorized(payment_id_from_path(req));
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<PaymentsApiError> authorize_mapped(
        interfaces::io::IRequest& req,
        std::string_view registry_operation,
        std::string_view extractor_operation
    ) const
    {
        const auto rule = m_context.auth_rules().rule_for(registry_operation);
        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth = m_context.authorizer().authorize_route(ctx, rule)) {
            return api_error(*auth);
        }
        if (!apply_extractor(req, ctx, auth_spec_for(extractor_operation), extractor_operation)) {
            return unauthorized(payment_id_from_path(req));
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<PaymentsApiError>
    authorize_extractor_only(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        if (!apply_extractor(req, ctx, auth_spec_for(operation), operation)) {
            return unauthorized(payment_id_from_path(req));
        }
        return std::nullopt;
    }

private:
    enum class ApiVariant
    {
        Unauthorized,
        ForbiddenCommonResource,
        ForbiddenPrivateResource,
        Conflict,
        Gone,
        TooManyRequests,
        Unprocessable,
        InternalServerError,
        NotImplemented,
        ConnectorError,
        NotFound,
        MethodNotAllowed,
        BadRequest,
        DomainError,
    };

    enum class ExtractorKind
    {
        V2ApiKey,
        V2Client,
        ApiOrClientOrJwt,
        ApiOrClient,
        ApiOrJwt,
        Jwt,
        Sdk,
        SdkOrApiKey,
        ApiKey,
        InternalMerchantOrJwt,
        PlatformSelf,
        CallbackToken,
    };

    struct AuthSpec
    {
        ExtractorKind kind{ExtractorKind::ApiKey};
        std::string permission{};
        bool allow_connected{false};
        bool allow_platform{false};
    };

    static constexpr std::string_view kQueryKeys[] = {
        "status",
        "connector",
        "payment_method",
        "payment_method_type",
        "offset",
        "limit",
        "start",
        "end",
        "profile_id",
        "payment_id",
        "customer_id",
        "merchant_id",
        "merchant_reference_id",
        "return_url",
        "force_sync",
        "expand_attempts",
        "client_secret",
        "amount",
        "recurring_enabled",
        "installment_payment_enabled",
    };

    bool in_set(std::string_view op, std::initializer_list<std::string_view> names) const
    {
        for (const auto name: names) {
            if (op == name) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::string frame_field(std::string_view field) const
    {
        return std::format("{}:{}", field.size(), field);
    }

    [[nodiscard]] bool is_uuid_v4(std::string_view token) const noexcept
    {
        if (token.size() != 36 || token[8] != '-' || token[13] != '-' || token[18] != '-' ||
            token[23] != '-' || token[14] != '4') {
            return false;
        }
        for (std::size_t i = 0; i < token.size(); ++i) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                continue;
            }
            const char ch = token[i];
            const bool hex = (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f');
            if (!hex) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::string signature_key(
        std::string_view route_name,
        std::string_view payment_id,
        std::string_view merchant_id,
        std::string_view hash_tail,
        std::string_view token
    ) const
    {
        const std::string input = std::format(
            "{}{}{}{}{}",
            frame_field(route_name),
            frame_field(payment_id),
            frame_field(merchant_id),
            frame_field(hash_tail),
            token
        );
        const std::string secret = m_context.config_store()
                                       .get_value("callback_token_secret", merchant_id)
                                       .value_or(std::string{"local_callback_secret"});
        ::app::crypto::KeyedHasher hasher{secret};
        return std::format(
            "payment_callback_token:{}",
            hasher.hash_with_domain("callback_token", input)
        );
    }

    [[nodiscard]] std::string profile_for(interfaces::io::IRequest& req) const
    {
        const auto ctx = ::app::auth::AuthContextBuilder{}(req);
        if (!ctx.profile_id().empty()) {
            return ctx.profile_id();
        }
        const std::string header = ::app::http::HeaderReader{}.get(req, "x-profile-id");
        return header.empty() ? query_value(req, "profile_id") : header;
    }

    [[nodiscard]] std::optional<::app::store::payments::PaymentIntentRecord>
    find_payment_record(interfaces::io::IRequest& req, std::string_view payment_id) const
    {
        if (payment_id.empty()) {
            return std::nullopt;
        }
        const auto record = m_context.repository().find_payment_by_string(payment_id);
        if (!record) {
            return std::nullopt;
        }
        const std::string merchant = ::app::http::merchant_id_from_header{}(req);
        if (!merchant.empty() && record->get_merchant_id() != merchant) {
            return std::nullopt;
        }
        const std::string profile = profile_for(req);
        if (!profile.empty()) {
            const auto it = m_scope.lookup(record->get_merchant_id(), record->get_payment_id());
            if (it.has_value() && *it != profile) {
                return std::nullopt;
            }
        }
        return record;
    }

    [[nodiscard]] bool
    client_secret_matches(interfaces::io::IRequest& req, const ::app::auth::AuthContext& ctx) const
    {
        const auto secret = ctx.claim(::app::auth::kClaimClientSecret);
        if (!secret || secret->empty()) {
            return false;
        }
        const auto record = find_payment_record(req, payment_id_from_path(req));
        if (!record) {
            return true;
        }
        const std::string expected = metadata_value(record->get_metadata(), "client_secret");
        return expected.empty() || expected == *secret;
    }

    [[nodiscard]] bool client_secret_query_matches(interfaces::io::IRequest& req) const
    {
        const std::string secret = query_value(req, "client_secret");
        if (secret.empty()) {
            return false;
        }
        const auto record = find_payment_record(req, payment_id_from_path(req));
        if (!record) {
            return true;
        }
        const std::string expected = metadata_value(record->get_metadata(), "client_secret");
        return expected.empty() || expected == secret;
    }

    [[nodiscard]] bool verify_callback_token(interfaces::io::IRequest& req) const
    {
        const std::string merchant = ::app::http::merchant_id_from_header{}(req);
        const std::string mode = m_context.config_store()
                                     .get_value("payment_callback_mode", merchant)
                                     .value_or(std::string{"legacy"});
        const bool issues_tokens = mode == "enabled" || mode == "forced";
        const auto segments = ::app::http::RequestPath{}(req);
        std::string token{};
        std::size_t token_index = segments.count();
        for (std::size_t i = 0; i + 1 < segments.count(); ++i) {
            if (segments.at(i) && *segments.at(i) == "token") {
                token = std::string{*segments.at(i + 1)};
                token_index = i + 1;
                break;
            }
        }
        if (!issues_tokens) {
            return true;
        }
        if (token.empty() || !is_uuid_v4(token) || token_index + 2 > segments.count()) {
            return false;
        }
        std::string route_name{"payment_callback_redirect_response"};
        std::string hash_tail{};
        if (token_index + 1 < segments.count()) {
            const std::string kind = std::string{*segments.at(token_index + 1)};
            const std::string first = token_index + 2 < segments.count()
                                          ? std::string{*segments.at(token_index + 2)}
                                          : std::string{};
            const std::string second = token_index + 3 < segments.count()
                                           ? std::string{*segments.at(token_index + 3)}
                                           : std::string{};
            const bool with_creds = !second.empty();
            if (kind == "start") {
                route_name = "payment_callback_start";
                hash_tail = first;
            } else if (kind == "response") {
                route_name = with_creds ? "payment_callback_redirect_response_with_credentials"
                                        : "payment_callback_redirect_response";
                hash_tail = with_creds
                                ? std::format("{}{}", frame_field(first), frame_field(second))
                                : first;
            } else if (kind == "complete") {
                route_name = with_creds ? "payment_callback_redirect_complete_with_credentials"
                                        : "payment_callback_redirect_complete";
                hash_tail = with_creds
                                ? std::format("{}{}", frame_field(first), frame_field(second))
                                : first;
            } else if (kind == "authorize") {
                route_name = "payment_callback_authorize";
                hash_tail = first;
            }
        }
        const std::string key =
            signature_key(route_name, payment_id_from_path(req), merchant, hash_tail, token);
        const auto cached = m_context.cache().get(key);
        const bool valid = cached.hit && cached.value_or<std::string>({}) == "UNUSED";
        m_context.cache().erase(key);
        return valid;
    }

    [[nodiscard]] AuthSpec auth_spec_for(std::string_view op) const
    {
        AuthSpec spec{};
        if (in_set(
                op,
                {"payments_start",
                 "payments_redirect_response",
                 "payments_redirect_response_with_creds_identifier",
                 "payments_complete_authorize_redirect",
                 "payments_complete_authorize_redirect_with_creds_identifier",
                 "post_3ds_payments_authorize"}
            )) {
            spec.kind = ExtractorKind::CallbackToken;
            return spec;
        }
        if (op == "payments_post_session_tokens") {
            spec.kind = ExtractorKind::Sdk;
            return spec;
        }
        if (op == "payments_dynamic_tax_calculation") {
            spec.kind = ExtractorKind::SdkOrApiKey;
            return spec;
        }
        if (in_set(
                op,
                {"payments_get_intent", "revenue_recovery_get_intent", "payments_retrieve"}
            )) {
            spec.kind = ExtractorKind::ApiOrClientOrJwt;
            spec.permission = "ProfileRevenueRecoveryRead";
            return spec;
        }
        if (in_set(op, {"payment_confirm_intent", "payments_update", "payments_confirm"})) {
            spec.kind = ExtractorKind::ApiOrClient;
            return spec;
        }
        if (in_set(op, {"recovery_payments_create", "payments_update_intent"})) {
            spec.kind = ExtractorKind::V2ApiKey;
            return spec;
        }
        if (in_set(
                op,
                {"payments_create", "payments_create_and_confirm_intent", "payment_link_create"}
            )) {
            spec.kind = ExtractorKind::InternalMerchantOrJwt;
            spec.permission = "ProfilePaymentWrite";
            spec.allow_connected = true;
            return spec;
        }
        if (in_set(op, {"payments_list_for_platform", "payments_list_for_platform_filters"})) {
            spec.kind = ExtractorKind::PlatformSelf;
            spec.allow_platform = true;
            return spec;
        }
        if (in_set(
                op,
                {"get_payment_filters_profile",
                 "get_payments_aggregates",
                 "get_payments_aggregates_profile",
                 "list_payment_methods",
                 "payments_apply_pm_data",
                 "profile_payments_list",
                 "profile_payments_list_by_filter",
                 "proxy_confirm_intent",
                 "confirm_intent_with_external_vault_proxy"}
            )) {
            spec.kind = ExtractorKind::Jwt;
            spec.permission = "ProfilePaymentRead";
            return spec;
        }
        if (op == "list_payment_methods_for_payments_client") {
            spec.kind = ExtractorKind::ApiOrClientOrJwt;
            spec.permission = "ProfilePaymentRead";
            spec.allow_connected = true;
            spec.allow_platform = true;
            return spec;
        }
        if (in_set(
                op,
                {"get_payment_list_from_opensearch", "get_profile_payment_list_from_opensearch"}
            )) {
            spec.kind = ExtractorKind::Jwt;
            spec.permission = "MerchantPaymentRead";
            spec.allow_connected = true;
            spec.allow_platform = true;
            return spec;
        }
        if (in_set(
                op,
                {"payments_create_intent",
                 "get_payment_filters",
                 "payments_list",
                 "revenue_recovery_invoices_list",
                 "revenue_recovery_invoices_list_profile",
                 "payment_get_intent_using_merchant_reference_id",
                 "list_payment_attempts",
                 "payment_status",
                 "payments_capture",
                 "payments_cancel",
                 "payments_list_by_filter",
                 "get_filters_for_payments",
                 "payments_cancel_post_capture",
                 "payments_approve",
                 "payments_reject",
                 "payments_connector_session"}
            )) {
            spec.kind = ExtractorKind::ApiOrJwt;
            spec.permission = "ProfilePaymentWrite";
            return spec;
        }
        spec.kind = ExtractorKind::ApiKey;
        return spec;
    }

    [[nodiscard]] bool apply_extractor(
        interfaces::io::IRequest& req,
        const ::app::auth::AuthContext& ctx,
        const AuthSpec& spec,
        std::string_view operation
    ) const
    {
        (void)operation;
        const bool api_key = ctx.claim(::app::auth::kClaimApiKey).has_value();
        const bool internal = ctx.claim(::app::auth::kClaimInternalApiKey).has_value();
        const bool sdk = ctx.claim(::app::auth::kClaimSdkKey).has_value();
        const auto publishable = ctx.claim(::app::auth::kClaimPublishableKey);
        const auto client_secret = ctx.claim(::app::auth::kClaimClientSecret);
        const bool jwt = !ctx.token().is_empty();
        const bool connected = ctx.claim(::app::auth::kClaimConnectedMerchantId).has_value();
        const bool platform = !ctx.org_id().empty();
        switch (spec.kind) {
            case ExtractorKind::V2ApiKey:
                return api_key && (!connected || spec.allow_connected) &&
                       (!platform || spec.allow_platform);
            case ExtractorKind::V2Client:
                return client_secret.has_value() || publishable.has_value();
            case ExtractorKind::ApiOrClientOrJwt:
                return api_key || sdk || jwt || client_secret_matches(req, ctx) ||
                       client_secret_query_matches(req) ||
                       (publishable.has_value() && publishable->starts_with("pk_"));
            case ExtractorKind::ApiOrClient:
                return api_key || client_secret_matches(req, ctx) ||
                       client_secret_query_matches(req);
            case ExtractorKind::ApiOrJwt:
                return api_key || jwt;
            case ExtractorKind::Jwt:
                return jwt;
            case ExtractorKind::Sdk:
                return sdk || (publishable.has_value() && publishable->starts_with("pk_"));
            case ExtractorKind::SdkOrApiKey:
                return sdk || api_key ||
                       (publishable.has_value() && publishable->starts_with("pk_"));
            case ExtractorKind::ApiKey:
                return api_key;
            case ExtractorKind::InternalMerchantOrJwt:
                return internal || jwt;
            case ExtractorKind::PlatformSelf:
                return platform && (api_key || jwt);
            case ExtractorKind::CallbackToken:
                return verify_callback_token(req);
        }
        return false;
    }

    [[nodiscard]] PaymentsApiError unauthorized(std::string_view payment_id) const
    {
        return api_error_variant(
            ApiVariant::Unauthorized,
            "Authentication information is missing or invalid",
            "IR_08",
            payment_id
        );
    }

    [[nodiscard]] PaymentsApiError api_error_variant(
        ApiVariant variant,
        std::string_view message,
        std::string_view code,
        std::string_view payment_id
    ) const
    {
        using Kind = core::error::AppErrorKind;
        Kind kind{Kind::INVALID_REQUEST};
        switch (variant) {
            case ApiVariant::Unauthorized:
                kind = Kind::UNAUTHORIZED;
                break;
            case ApiVariant::ForbiddenCommonResource:
                kind = Kind::FORBIDDEN;
                break;
            case ApiVariant::ForbiddenPrivateResource:
                kind = Kind::NOT_FOUND;
                break;
            case ApiVariant::Conflict:
                kind = Kind::ALREADY_EXISTS;
                break;
            case ApiVariant::Gone:
                kind = Kind::NOT_FOUND;
                break;
            case ApiVariant::TooManyRequests:
                kind = Kind::RATE_LIMITED;
                break;
            case ApiVariant::Unprocessable:
                kind = Kind::VALIDATION_FAILED;
                break;
            case ApiVariant::InternalServerError:
                kind = Kind::INTERNAL;
                break;
            case ApiVariant::NotImplemented:
                kind = Kind::INTERNAL;
                break;
            case ApiVariant::ConnectorError:
                kind = Kind::CONNECTOR_ERROR;
                break;
            case ApiVariant::NotFound:
                kind = Kind::NOT_FOUND;
                break;
            case ApiVariant::MethodNotAllowed:
                kind = Kind::INVALID_REQUEST;
                break;
            case ApiVariant::BadRequest:
                kind = Kind::INVALID_REQUEST;
                break;
            case ApiVariant::DomainError:
                kind = Kind::INVALID_REQUEST;
                break;
        }
        PaymentsApiError error = core::error::AppErrorBuilder{}(kind, std::string{message});
        error.set_kind_name(std::string{code});
        if (!payment_id.empty()) {
            error.set_message(std::format("{} (payment id: {})", message, payment_id));
        }
        return error;
    }

    ::app::context::AppContext& m_context;
    PaymentsScopeState& m_scope;
};

/** @brief Request policy for the payments domain: wraps the domain auth rules. */
class PaymentsPolicy
{
public:
    PaymentsPolicy(::app::context::AppContext& /*context*/, PaymentsInputSupport& support) noexcept
        :
        m_support{support}
    {
    }

    [[nodiscard]] core::behavior::CallContext context(interfaces::io::IRequest& req) const
    {
        return ::app::payment::detail::make_call_context(req);
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view operation,
        const core::behavior::AuthSpec& spec
    ) const
    {
        (void)spec;
        if (spec.allow_platform_self_operation) {
            return m_support.authorize_extractor_only(req, operation);
        }
        if (!spec.permission.empty() && spec.permission != operation) {
            return m_support.authorize_mapped(req, operation, spec.permission);
        }
        return m_support.authorize(req, operation);
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        core::behavior::Scope result;
        result.merchant_id = std::string{auth.merchant_id()};
        result.profile_id = std::string{auth.profile_id()};
        result.org_id = std::string{auth.org_id()};
        const auto connected = auth.claim(::app::auth::kClaimConnectedMerchantId);
        result.connected_merchant_id = connected ? std::string{*connected} : std::string{};
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope&) const
    {
        return std::nullopt;
    }


private:
    PaymentsInputSupport& m_support;
};

using PaymentsRequestSupport = core::behavior::RequestSupport<PaymentsPolicy>;
using PaymentsPolicySpec = core::behavior::AuthSpec;

[[nodiscard]] PaymentsPolicySpec spec_for(std::string_view operation) noexcept
{
    return PaymentsPolicySpec{.permission = operation};
}

[[nodiscard]] PaymentsPolicySpec extractor_only_spec_for(std::string_view operation) noexcept
{
    return PaymentsPolicySpec{.permission = operation, .allow_platform_self_operation = true};
}
} // namespace app::handlers::payments
