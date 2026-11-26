export module app_payment:pm_auth_input;

import std;
import interfaces;
import core_serde;
import core_error;
import app_auth;
import app_http;
import app_dto;
import app_context;

export namespace app::handlers::pm_auth {

struct AuthResult
{
    std::optional<core::error::AppError> error{};
    ::app::auth::AuthContext context{};
};

/** @brief Request parsing / auth / header support for the pm_auth handlers.
 *
 *  Keep free of business logic: it only turns request data into the plain Input
 *  structs consumed by the domain runtime.
 */
class PmAuthInputSupport
{
public:
    explicit PmAuthInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /** @brief get_connector_auth_type + BodyKey extraction is business logic and lives in
     *  runtime/common.cppm; this class only mirrors the route-layer extractors.
     *
     *  AuthContextBuilder (app/auth/context.cppm) extracts the SDK publishable key, SDK key
     *  and client-secret claims from the SDK Authorization header key=value pairs and from
     *  x-client-secret / the request body, mirroring the upstream extractors.
     */
    [[nodiscard]] AuthResult authorize_with_scheme(
        interfaces::io::IRequest& req,
        std::string_view body,
        std::string_view operation,
        ::app::auth::AuthScheme scheme
    ) const
    {
        AuthResult result{};
        result.context = ::app::auth::AuthContextBuilder{}(req, body);
        const auto rule = ::app::auth::RouteAuthRuleBuilder{}
                              .scheme(scheme)
                              .permission(m_context.auth_rules().rule_for(operation).permission())
                              .build();
        result.error = m_context.authorizer().authorize_route(result.context, rule);
        return result;
    }

    /** @brief v1 check_sdk_auth_and_get_auth (routes/pm_auth.rs:25-32,91-98). */
    [[nodiscard]] AuthResult sdk_auth_and_get_auth(
        interfaces::io::IRequest& req,
        std::string_view body,
        std::string_view operation
    ) const
    {
        return authorize_with_scheme(req, body, operation, ::app::auth::AuthScheme::SDK);
    }

    /** @brief v2 check_client_secret_and_get_auth (routes/pm_auth.rs:36-43,102-109). */
    [[nodiscard]] AuthResult client_secret_and_get_auth(
        interfaces::io::IRequest& req,
        std::string_view body,
        std::string_view operation
    ) const
    {
        return authorize_with_scheme(req, body, operation, ::app::auth::AuthScheme::CLIENT_SECRET);
    }

    [[nodiscard]] AuthResult authorize_request(
        interfaces::io::IRequest& req,
        std::string_view body,
        std::string_view operation
    ) const
    {
        // Upstream builds ApiKeyAuth::default() (routes/pm_auth.rs:20,86) and selects exactly
        // one extractor by feature: v1 check_sdk_auth_and_get_auth (25-32,91-98), v2
        // check_client_secret_and_get_auth (36-43,102-109). Try both, then keep the registry
        // rule and its alternate schemes as the local compatibility fallback.
        if (auto sdk = sdk_auth_and_get_auth(req, body, operation); !sdk.error) {
            return sdk;
        }
        if (auto client_secret = client_secret_and_get_auth(req, body, operation);
            !client_secret.error) {
            return client_secret;
        }

        AuthResult result{};
        result.context = ::app::auth::AuthContextBuilder{}(req, body);
        const auto rule = m_context.auth_rules().rule_for(operation);
        result.error = m_context.authorizer().authorize_route(result.context, rule);
        if (!result.error) {
            return result;
        }
        const auto try_scheme = [this, &result, &rule](::app::auth::AuthScheme scheme)
        {
            const auto alternate = ::app::auth::RouteAuthRuleBuilder{}
                                       .scheme(scheme)
                                       .permission(rule.permission())
                                       .build();
            return !m_context.authorizer().authorize_route(result.context, alternate).has_value();
        };
        if (try_scheme(::app::auth::AuthScheme::SDK) ||
            try_scheme(::app::auth::AuthScheme::CLIENT_SECRET) ||
            try_scheme(::app::auth::AuthScheme::API_KEY) ||
            try_scheme(::app::auth::AuthScheme::V2_API_KEY)) {
            result.error.reset();
        }
        return result;
    }

    [[nodiscard]] std::expected<std::string, core::error::AppError>
    merchant_from_request(interfaces::io::IRequest& req) const
    {
        const std::string merchant_id = ::app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED,
                "missing merchant id header"
            )};
        }
        if (!m_context.repository().find_merchant_account_by_string(merchant_id)) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "merchant account not found"
            )};
        }
        return merchant_id;
    }

    /** @brief Flatten the upstream {field: {"value": ...}} wrappers into the DTO shape. */
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    normalize_link_token_body(std::string_view body) const
    {
        const auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            )};
        }

        std::string payment_id;
        std::string payment_method;
        std::string payment_method_type;
        std::string language;
        std::string client_secret;
        if (const auto* object = std::get_if<interfaces::Value::Object>(&generic->get())) {
            for (const auto& [name, field]: *object) {
                std::string text = field_text(field);
                if (name == "payment_id") {
                    payment_id = text;
                } else if (name == "payment_method") {
                    payment_method = text;
                } else if (name == "payment_method_type") {
                    payment_method_type = text;
                } else if (name == "language") {
                    language = text;
                } else if (name == "client_secret") {
                    client_secret = text;
                }
            }
        }

        std::string normalized =
            "{\"payment_id\":{\"value\":" + serde::Ser::encode_json(payment_id) +
            "},\"payment_method\":{\"value\":" + serde::Ser::encode_json(payment_method) +
            "},\"payment_method_type\":{\"value\":" + serde::Ser::encode_json(payment_method_type);
        if (!language.empty()) {
            normalized += "},\"language\":" + serde::Ser::encode_json(language);
        } else {
            normalized += "}";
        }
        if (!client_secret.empty()) {
            normalized += ",\"client_secret\":" + serde::Ser::encode_json(client_secret);
        }
        normalized += "}";
        return normalized;
    }

    [[nodiscard]] std::expected<std::string, core::error::AppError>
    normalize_exchange_body(std::string_view body) const
    {
        const auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON,
                "invalid JSON"
            )};
        }

        std::string public_token;
        std::string payment_id;
        std::string payment_method;
        std::string payment_method_type;
        std::string client_secret;
        if (const auto* object = std::get_if<interfaces::Value::Object>(&generic->get())) {
            for (const auto& [name, field]: *object) {
                std::string text = field_text(field);
                if (name == "public_token") {
                    public_token = text;
                } else if (name == "payment_id") {
                    payment_id = text;
                } else if (name == "payment_method") {
                    payment_method = text;
                } else if (name == "payment_method_type") {
                    payment_method_type = text;
                } else if (name == "client_secret") {
                    client_secret = text;
                }
            }
        }

        std::string normalized =
            "{\"public_token\":" + serde::Ser::encode_json(public_token) +
            ",\"payment_id\":{\"value\":" + serde::Ser::encode_json(payment_id) +
            "},\"payment_method\":{\"value\":" + serde::Ser::encode_json(payment_method) +
            "},\"payment_method_type\":{\"value\":" + serde::Ser::encode_json(payment_method_type);
        if (!client_secret.empty()) {
            normalized += "},\"client_secret\":" + serde::Ser::encode_json(client_secret);
        } else {
            normalized += "}";
        }
        normalized += "}";
        return normalized;
    }

private:
    [[nodiscard]] std::string field_text(const interfaces::Value& field) const
    {
        std::string text;
        if (const auto* scalar = std::get_if<std::string>(&field.get())) {
            text = *scalar;
        } else if (const auto* wrapped = std::get_if<interfaces::Value::Object>(&field.get())) {
            for (const auto& [inner_name, inner]: *wrapped) {
                if (inner_name == "value") {
                    if (const auto* inner_text = std::get_if<std::string>(&inner.get())) {
                        text = *inner_text;
                    }
                }
            }
        }
        return text;
    }

    ::app::context::AppContext& m_context;
};

// ───────────────────────── Plain inputs ─────────────────────────
// Each struct carries exactly what the old core call received.

struct LinkTokenCreateInput
{
    ::app::dto::pm_auth::LinkTokenCreateRequest request{};
    std::string merchant_id{};
    AuthResult auth{};
    std::string client_platform{};
    std::string app_id{};
    std::string redirect_uri{};
};

struct ExchangeTokenInput
{
    ::app::dto::pm_auth::ExchangeTokenCreateRequest request{};
    std::string merchant_id{};
    AuthResult auth{};
    std::string body{};
};

} // namespace app::handlers::pm_auth
