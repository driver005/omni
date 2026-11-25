export module app_handlers_superposition_sdk_config:input;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_auth;
import app_http;
import app_crypto;

export namespace app::handlers::superposition_sdk_config {

/**
 * @brief Upstream ApiErrorResponse variants used by these two routes, mapped onto the
 * local core::error model. The variant name is carried on the wire as `error`; the
 * upstream HTTP status is carried as a status override (VALIDATION_FAILED maps to 422
 * locally but 400 upstream, so the override is mandatory for those variants).
 */
class SdkConfigErrorSupport
{
public:
    [[nodiscard]] core::error::AppError unauthorized() const
    {
        return wire_error(core::error::AppErrorKind::UNAUTHORIZED, "Unauthorized", "Unauthorized");
    }

    [[nodiscard]] core::error::AppError missing_required_field(std::string_view field) const
    {
        return wire_error(
            core::error::AppErrorKind::VALIDATION_FAILED,
            std::format("Missing required field: {}", field),
            "MissingRequiredField"
        );
    }

    [[nodiscard]] core::error::AppError invalid_data_value(std::string_view field) const
    {
        return wire_error(
            core::error::AppErrorKind::VALIDATION_FAILED,
            std::format("Invalid data value: {}", field),
            "InvalidDataValue"
        );
    }

    [[nodiscard]] core::error::AppError payment_not_found() const
    {
        return wire_error(
            core::error::AppErrorKind::NOT_FOUND,
            "PaymentNotFound",
            "PaymentNotFound"
        );
    }

    [[nodiscard]] core::error::AppError profile_not_found(std::string_view id) const
    {
        return wire_error(
            core::error::AppErrorKind::NOT_FOUND,
            std::format("Profile not found: {}", id),
            "ProfileNotFound"
        );
    }

    [[nodiscard]] core::error::AppError customer_not_found() const
    {
        return wire_error(
            core::error::AppErrorKind::NOT_FOUND,
            "CustomerNotFound",
            "CustomerNotFound"
        );
    }

    [[nodiscard]] core::error::AppError merchant_account_not_found() const
    {
        return wire_error(
            core::error::AppErrorKind::NOT_FOUND,
            "MerchantAccountNotFound",
            "MerchantAccountNotFound"
        );
    }

    [[nodiscard]] core::error::AppError generic_not_found(std::string_view message) const
    {
        return wire_error(
            core::error::AppErrorKind::NOT_FOUND,
            std::string{message},
            "GenericNotFoundError"
        );
    }

    [[nodiscard]] core::error::AppError internal_server_error() const
    {
        return wire_error(
            core::error::AppErrorKind::INTERNAL_ERROR,
            "InternalServerError",
            "InternalServerError"
        );
    }

    // Errors whose wire status differs from AppErrorStatus(kind) carry the upstream
    // variant name in kind_name; translate it back to the status override here.
    [[nodiscard]] std::optional<interfaces::io::types::Status>
    status_override_for(std::string_view kind_name) const noexcept
    {
        using interfaces::io::types::Status;
        if (kind_name == "Unauthorized") {
            return Status::UNAUTHORIZED;
        }
        if (kind_name == "MissingRequiredField" || kind_name == "InvalidDataValue") {
            return Status::BAD_REQUEST;
        }
        if (kind_name == "PaymentNotFound" || kind_name == "ProfileNotFound" ||
            kind_name == "CustomerNotFound" || kind_name == "MerchantAccountNotFound" ||
            kind_name == "GenericNotFoundError") {
            return Status::NOT_FOUND;
        }
        if (kind_name == "InternalServerError") {
            return Status::INTERNAL_SERVER_ERROR;
        }
        return std::nullopt;
    }

private:
    [[nodiscard]] core::error::AppError wire_error(
        core::error::AppErrorKind kind,
        std::string message,
        std::string_view kind_name
    ) const
    {
        auto error = core::error::AppErrorBuilder{}(kind, std::move(message));
        error.set_kind_name(std::string{kind_name});
        return error;
    }
};

struct SdkAuth
{
    std::string profile_id{};
    std::string payment_id{};
    std::string publishable_key{};
    std::string client_secret{};
    bool decoded{false};
};

struct PaymentIdResolution
{
    std::string payment_id{};
    std::string client_secret{};
};

struct Resolution
{
    std::optional<PaymentIdResolution> value{};
    std::optional<core::error::AppError> error{};
};

/**
 * @brief Parse/auth/path/query support for the superposition_sdk_config handlers.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * structs consumed by the domain runtime. Errors are returned, never written.
 */
class SuperpositionSdkConfigInputSupport
{
public:
    explicit SuperpositionSdkConfigInputSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // SdkAuthorization::decode (domain_models/sdk_auth.rs:134-236). BASE64_ENGINE is standard
    // base64 with padding (common_utils/consts.rs:124); the local codec is Base64Url, so the
    // alphabet is translated and padding dropped before decoding.
    [[nodiscard]] std::optional<SdkAuth> decode_sdk_authorization(std::string_view header) const
    {
        if (header.empty()) {
            return std::nullopt;
        }
        constexpr std::string_view bearer_prefix{"Bearer "};
        if (header.size() > bearer_prefix.size() &&
            std::equal(
                bearer_prefix.begin(),
                bearer_prefix.end(),
                header.begin(),
                [](char lhs, char rhs)
                {
                    return std::tolower(static_cast<unsigned char>(lhs)) ==
                           std::tolower(static_cast<unsigned char>(rhs));
                }
            )) {
            return std::nullopt;
        }

        std::string normalized{};
        normalized.reserve(header.size());
        for (const char ch: header) {
            if (ch == '+') {
                normalized.push_back('-');
            } else if (ch == '/') {
                normalized.push_back('_');
            } else if (ch != '=') {
                normalized.push_back(ch);
            }
        }
        std::string decoded = app::crypto::Base64Url{}.decode(normalized);
        if (decoded.find('=') == std::string::npos) {
            decoded = std::string{header};
        }

        SdkAuth result{};
        std::size_t start{0};
        while (start <= decoded.size()) {
            const std::size_t comma = decoded.find(',', start);
            const std::string_view pair = std::string_view{decoded}.substr(
                start,
                comma == std::string::npos ? std::string::npos : comma - start
            );
            const std::size_t equals = pair.find('=');
            if (equals != std::string_view::npos) {
                const std::string key = trim_copy(pair.substr(0, equals));
                const std::string value = trim_copy(pair.substr(equals + 1));
                if (!key.empty() && !value.empty()) {
                    result.decoded = true;
                    if (key == "profile_id") {
                        result.profile_id = value;
                    } else if (key == "payment_id") {
                        result.payment_id = value;
                    } else if (key == "publishable_key" || key == "publishable-key") {
                        result.publishable_key = value;
                    } else if (key == "client_secret" || key == "client-secret") {
                        result.client_secret = value;
                    }
                }
            }
            if (comma == std::string::npos) {
                break;
            }
            start = comma + 1;
        }
        return result;
    }

    // Upstream routes::get_payment_id_from_headers_or_payload (routes:19-49). The Authorization
    // header switches the whole flow to SDK auth; a decode failure is Unauthorized and a missing
    // payment_id is MissingRequiredField. Without the header, client_secret comes from the query
    // (never the body) and the id is derived by the payments helper.
    [[nodiscard]] Resolution get_payment_id_from_headers_or_payload(
        std::string_view auth_header,
        const std::optional<SdkAuth>& sdk_auth,
        const app::http::QueryParams& query
    ) const
    {
        Resolution out{};
        if (!auth_header.empty()) {
            if (!sdk_auth || !sdk_auth->decoded) {
                out.error = m_errors.unauthorized();
                return out;
            }
            if (sdk_auth->payment_id.empty()) {
                out.error = m_errors.missing_required_field("payment_id");
                return out;
            }
            out.value = PaymentIdResolution{sdk_auth->payment_id, sdk_auth->client_secret};
            return out;
        }

        const auto secret = query.get("client_secret");
        if (!secret || secret->empty()) {
            out.error = m_errors.missing_required_field("client_secret");
            return out;
        }
        const auto payment_id = payment_id_from_client_secret(*secret);
        if (!payment_id || payment_id->empty()) {
            out.error = m_errors.invalid_data_value("payment_id");
            return out;
        }
        out.value = PaymentIdResolution{*payment_id, *secret};
        return out;
    }

    // Upstream check_sdk_auth_and_get_auth (authentication.rs:6037-6066) plus the SDK-header
    // claim decoding of SdkAuthorization::decode (domain_models/sdk_auth.rs:134-236).
    [[nodiscard]] std::expected<app::auth::AuthContext, core::error::AppError> authorize_request(
        interfaces::io::IRequest& req,
        std::string_view operation,
        std::string_view client_secret,
        const std::optional<SdkAuth>& sdk_auth,
        bool has_authorization_header
    ) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        auto ctx = app::auth::AuthContextBuilder{}(req);

        // SdkAuthorization::decode yields a base64-wrapped comma-separated key=value map.
        // AuthContextBuilder only reads the plain form, so decode the wrapped form here and feed
        // the same claims into the authorizer.
        if (sdk_auth && sdk_auth->decoded) {
            if (!sdk_auth->publishable_key.empty()) {
                ctx.add_claim(
                    std::string{app::auth::kClaimPublishableKey},
                    sdk_auth->publishable_key
                );
            }
            if (!sdk_auth->client_secret.empty()) {
                ctx.add_claim(std::string{app::auth::kClaimClientSecret}, sdk_auth->client_secret);
            }
            if (!sdk_auth->profile_id.empty()) {
                ctx.add_claim("x-profile-id", sdk_auth->profile_id);
                ctx.set_profile_id(sdk_auth->profile_id);
            }
        }
        // Upstream check_client_secret_and_get_auth reads client_secret from the payload; the
        // local builder does not read the query string, so inject the resolved query value.
        if (!client_secret.empty()) {
            ctx.add_claim(std::string{app::auth::kClaimClientSecret}, std::string{client_secret});
        }

        // The registry exposes these SDK routes through bearer-based schemes even though
        // upstream authenticates with SDK/publishable headers. Feed the presented header
        // credential as the flat scheme's credential; the header claims are still required
        // by authorize_route for SDK/PUBLISHABLE_KEY rules.
        if (ctx.token().is_empty()) {
            const auto sdk_key = ctx.claim(app::auth::kClaimSdkKey);
            const auto secret_claim = ctx.claim(app::auth::kClaimClientSecret);
            const auto publishable_key = ctx.claim(app::auth::kClaimPublishableKey);
            if (sdk_key && !sdk_key->empty()) {
                ctx.set_token(app::auth::BearerToken{std::string{*sdk_key}});
            } else if (secret_claim && !secret_claim->empty()) {
                ctx.set_token(app::auth::BearerToken{std::string{*secret_claim}});
            } else if (publishable_key && !publishable_key->empty()) {
                ctx.set_token(app::auth::BearerToken{std::string{*publishable_key}});
            }
        }

        const auto auth = m_context.authorizer().authorize_route(ctx, rule);
        if (auth) {
            return std::unexpected{*auth};
        }

        // Upstream HeaderAuth(PublishableKeyAuth) for the profile route (routes:119-122);
        // SdkAuthorizationAuth for the header-present path of get_sdk_config (routes:54-60).
        if (operation == "get_profile_sdk_config" ||
            (operation == "get_sdk_config" && has_authorization_header)) {
            const auto publishable_key = ctx.claim(app::auth::kClaimPublishableKey);
            if (!publishable_key.has_value() || publishable_key->empty()) {
                return std::unexpected{m_errors.unauthorized()};
            }
        }
        return ctx;
    }

    [[nodiscard]] std::string provider_merchant_id(interfaces::io::IRequest& req) const
    {
        return app::http::HeaderReader{}.get(req, "x-provider-merchant-id");
    }

    [[nodiscard]] std::string profile_id_from_path(interfaces::io::IRequest& req) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        for (std::size_t i = 0; i < segments.count(); ++i) {
            if (segments.at(i).value_or("") == "configs" && i + 2 < segments.count()) {
                return std::string{segments.at(i + 2).value_or("")};
            }
        }
        for (std::size_t i = 1; i < segments.count(); ++i) {
            if (segments.at(i).value_or("") == "sdk_config.json") {
                return std::string{segments.at(i - 1).value_or("")};
            }
        }
        return segments.count() > 0 ? std::string{segments.at(segments.count() - 1).value_or("")}
                                    : std::string{};
    }

private:
    [[nodiscard]] std::string trim_copy(std::string_view value) const
    {
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
            value.remove_prefix(1);
        }
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
            value.remove_suffix(1);
        }
        return std::string{value};
    }

    // Mirrors payments::helpers::get_payment_id_from_client_secret (helpers.rs:5172-5177):
    // rsplit_once("_secret_"). Kept local because the helper is not exported by a core module.
    [[nodiscard]] std::optional<std::string>
    payment_id_from_client_secret(std::string_view client_secret) const
    {
        constexpr std::string_view separator{"_secret_"};
        const auto split = client_secret.rfind(separator);
        if (split == std::string_view::npos || split == 0 ||
            split + separator.size() >= client_secret.size()) {
            return std::nullopt;
        }
        return std::string{client_secret.substr(0, split)};
    }

    app::context::AppContext& m_context;
    SdkConfigErrorSupport m_errors{};
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old core call received.

struct GetSdkConfigInput
{
    std::string payment_id{};
    std::string sdk_profile_id{};
    std::string provider_merchant_id{};
    app::auth::AuthContext auth{};
};

struct GetProfileSdkConfigInput
{
    std::string profile_id{};
    std::string provider_merchant_id{};
    app::auth::AuthContext auth{};
};

} // namespace app::handlers::superposition_sdk_config
