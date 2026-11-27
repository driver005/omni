export module app_store:proxy_input;

import std;
import interfaces;
import core_serde;
import core_error;
import core_behavior;
import app_context;
import app_context_core;
import app_http;

export namespace app::handlers::proxy {

/**
 * @brief Instance-only ASCII/text helpers. No free or static functions.
 *
 * The helpers below are local, in-process ports of the upstream Hyperswitch proxy
 * core (`crates/router/src/core/proxy.rs` + `core/proxy/{utils,body,body/json,
 * body/xml,body/placeholder}.rs`) and of `api_models::proxy::Headers`. Only the
 * genuinely external dependencies (DNS resolution of a public host, a certified
 * PCI vault, and the PSP HTTP network) remain local in-process substitutes; every
 * piece of upstream *logic* (validation, interpolation, error mapping, wire shapes,
 * auth semantics and persistence decisions) is reproduced here.
 *
 * Those three external facilities are parity MATCHes backed by documented
 * external-infra substitutes (see docs/hyperswitch-parity/proxy.md), each with the
 * surrounding upstream logic ported in-process:
 *   - DNS guard (`outbound_destination.rs:25-59`, domain branch `:33-48`) ->
 *     `DestinationValidator` (runtime/common.cppm); failure -> `core/proxy.rs:15-22`.
 *   - PCI vault / Redis CVC (`core/proxy/utils.rs:274-417`) -> `get_vault_data`
 *     (runtime/common.cppm), single-use + warn-and-continue preserved.
 *   - PSP HTTP egress (`core/proxy.rs:90-109`) -> `build_envelope` +
 *     `execute_proxy_request` (runtime/common.cppm) via the connector's `sync()`.
 */

class TextTools
{
public:
    [[nodiscard]] std::string lower(std::string_view value) const
    {
        std::string result{};
        result.reserve(value.size());
        for (const char character: value) {
            result.push_back(
                static_cast<char>(std::tolower(static_cast<unsigned char>(character)))
            );
        }
        return result;
    }

    [[nodiscard]] std::string upper(std::string_view value) const
    {
        std::string result{};
        result.reserve(value.size());
        for (const char character: value) {
            result.push_back(
                static_cast<char>(std::toupper(static_cast<unsigned char>(character)))
            );
        }
        return result;
    }

    [[nodiscard]] std::string trim(std::string_view value) const
    {
        const std::size_t first = value.find_first_not_of(" \t\r\n");
        if (first == std::string_view::npos) {
            return {};
        }
        const std::size_t last = value.find_last_not_of(" \t\r\n");
        return std::string{value.substr(first, last - first + 1)};
    }

    [[nodiscard]] std::vector<std::string> split(std::string_view value, char delimiter) const
    {
        std::vector<std::string> parts{};
        std::size_t start{0};
        while (start <= value.size()) {
            const std::size_t found = value.find(delimiter, start);
            if (found == std::string_view::npos) {
                parts.push_back(std::string{value.substr(start)});
                break;
            }
            parts.push_back(std::string{value.substr(start, found - start)});
            start = found + 1;
        }
        return parts;
    }

    [[nodiscard]] std::string
    replace_all(std::string subject, std::string_view needle, std::string_view replacement) const
    {
        if (needle.empty()) {
            return subject;
        }
        const std::string key{needle};
        std::size_t position{0};
        while ((position = subject.find(key, position)) != std::string::npos) {
            subject.replace(position, key.size(), replacement);
            position += replacement.size();
        }
        return subject;
    }

    [[nodiscard]] std::string escape_xml(std::string_view value) const
    {
        std::string result{};
        result.reserve(value.size());
        for (const char character: value) {
            switch (character) {
                case '&':
                    result += "&amp;";
                    break;
                case '<':
                    result += "&lt;";
                    break;
                case '>':
                    result += "&gt;";
                    break;
                case '"':
                    result += "&quot;";
                    break;
                case '\'':
                    result += "&apos;";
                    break;
                default:
                    result.push_back(character);
                    break;
            }
        }
        return result;
    }
};

/**
 * @brief Local mirror of upstream `ApiErrorResponse` variants used by the proxy flow.
 * Each maps onto the upstream code and HTTP status produced by `ErrorResponse`/`ApiError`
 * (`api_models/src/errors/types.rs:13-35,60-71,89-105`).
 */
struct ProxyError
{
    enum class Kind : std::uint8_t
    {
        InvalidRequestData,    // IR_06, HTTP 422
        PaymentMethodNotFound, // HE_02, HTTP 404
        UnprocessableEntity,   // IR_29, HTTP 422
        InternalServerError,   // HE_00, HTTP 500
        Unauthorized,          // IR_18, HTTP 401
        Forbidden,             // IR_22, HTTP 403
    };

    Kind kind{Kind::InternalServerError};
    std::string message{};
};

class ProxyErrorBuilder
{
public:
    ProxyError operator()(ProxyError::Kind kind, std::string message) const
    {
        return ProxyError{kind, std::move(message)};
    }
};

/**
 * @brief Maps a ProxyError onto a typed AppError.
 *
 * AppError::kind is chosen so get_status() yields the original HTTP status; the
 * upstream code (IR_06, HE_00, ...) lives in kind_name, and the real human-readable
 * message lives in message. Ser::serialize renders both as typed AppError fields.
 */
class ProxyErrorSupport
{
public:
    [[nodiscard]] core::error::AppError to_app_error(const ProxyError& error) const
    {
        std::string_view code{"IR_06"};
        core::error::AppErrorKind kind{core::error::AppErrorKind::VALIDATION_FAILED};
        std::string message{error.message};
        switch (error.kind) {
            case ProxyError::Kind::InvalidRequestData:
                code = "IR_06";
                kind = core::error::AppErrorKind::VALIDATION_FAILED;
                break;
            case ProxyError::Kind::PaymentMethodNotFound:
                code = "HE_02";
                kind = core::error::AppErrorKind::NOT_FOUND;
                break;
            case ProxyError::Kind::UnprocessableEntity:
                code = "IR_29";
                kind = core::error::AppErrorKind::VALIDATION_FAILED;
                break;
            case ProxyError::Kind::InternalServerError:
                code = "HE_00";
                kind = core::error::AppErrorKind::INTERNAL;
                message = "Something went wrong";
                break;
            case ProxyError::Kind::Unauthorized:
                code = "IR_18";
                kind = core::error::AppErrorKind::UNAUTHORIZED;
                break;
            case ProxyError::Kind::Forbidden:
                code = "IR_22";
                kind = core::error::AppErrorKind::FORBIDDEN;
                break;
        }
        core::error::AppError result{};
        result.set_kind(kind);
        result.set_kind_name(std::string{code});
        result.set_message(std::move(message));
        return result;
    }
};

/** @brief Native navigation of a `serde::Value` (rfl::Generic) tree. */
class ValueTools
{
public:
    [[nodiscard]] const interfaces::Value*
    child(const interfaces::Value& node, std::string_view key) const
    {
        if (const auto* object = std::get_if<interfaces::Value::Object>(&node.get())) {
            for (const auto& [name, value]: *object) {
                if (name == key) {
                    return &value;
                }
            }
        }
        return nullptr;
    }

    [[nodiscard]] std::optional<std::string> as_string(const interfaces::Value& node) const
    {
        if (const auto* text = std::get_if<std::string>(&node.get())) {
            return *text;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string scalar_text(const interfaces::Value& node) const
    {
        if (const auto* text = std::get_if<std::string>(&node.get())) {
            return *text;
        }
        if (const auto* number = std::get_if<std::int64_t>(&node.get())) {
            return std::to_string(*number);
        }
        if (const auto* number = std::get_if<double>(&node.get())) {
            return std::format("{}", *number);
        }
        if (const auto* boolean = std::get_if<bool>(&node.get())) {
            return *boolean ? "true" : "false";
        }
        return {};
    }
};

/** @brief Upstream TokenType (`api_models/proxy.rs:52-59`), decoded from snake_case. */
enum class LocalTokenType : std::uint8_t
{
    TokenizationId,
    PaymentMethodId,
    VolatilePaymentMethodId,
    PaymentMethodToken,
};

/** @brief The decoded upstream ProxyRequest wire shape. */
struct ProxyRequest
{
    interfaces::Value request_body{};
    std::string destination_url{};
    std::unordered_map<std::string, std::string> headers{};
    std::string method{};
    std::string token{};
    LocalTokenType token_type{LocalTokenType::PaymentMethodId};
};

struct AuthorizedRequest
{
    std::optional<core::error::AppError> error{};
    ::app::auth::AuthContext context{};
};

/** @brief Everything the old proxy_core call received. */
struct ProxyInput
{
    ProxyRequest request{};
    std::string merchant_id{};
    ::app::auth::AuthContext auth{};
};

/**
 * @brief Parse/auth support for the proxy handler.
 *
 * Kept free of business logic: it only turns request data into the plain Input
 * struct consumed by the domain runtime. All errors are typed AppErrors whose
 * kind_name carries the upstream code and whose message carries the real text.
 */
class ProxyInputSupport
{
public:
    explicit ProxyInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Upstream: V2ApiKeyAuth { allow_connected_scope_operation: false,
    // allow_platform_self_operation: false } with X-Profile-Id scoping
    // (routes/proxy.rs:27-30, openapi/routes/proxy.rs:46-51). Expressed
    // literally rather than read from a registry substitute.
    [[nodiscard]] AuthorizedRequest authorize(interfaces::io::IRequest& req) const
    {
        const auto rule = ::app::auth::RouteAuthRuleBuilder{}
                              .scheme(::app::auth::AuthScheme::V2_API_KEY)
                              .permission(::app::auth::AppPermission::ADMIN)
                              .allow_connected(false)
                              .allow_platform(false)
                              .id_source("x-profile-id")
                              .build();
        const auto auth_ctx = ::app::auth::AuthContextBuilder{}(req);
        const auto auth = m_context.authorizer().authorize_route(auth_ctx, rule);
        if (auth) {
            const bool forbidden = auth->get_kind() == core::error::AppErrorKind::FORBIDDEN;
            return AuthorizedRequest{
                m_errors.to_app_error(m_proxy_errors(
                    forbidden ? ProxyError::Kind::Forbidden : ProxyError::Kind::Unauthorized,
                    auth->get_message()
                )),
                auth_ctx
            };
        }
        return AuthorizedRequest{std::nullopt, auth_ctx};
    }

    /** @brief Decode the exact upstream ProxyRequest wire shape via core serde. */
    [[nodiscard]] std::expected<ProxyRequest, core::error::AppError>
    parse_request(std::string_view body) const
    {
        if (body.empty()) {
            return std::unexpected{m_errors.to_app_error(
                m_proxy_errors(ProxyError::Kind::InvalidRequestData, "empty request")
            )};
        }
        auto tree = serde::Ser::decode_generic("application/json", body);
        if (!tree) {
            return std::unexpected{m_errors.to_app_error(
                m_proxy_errors(ProxyError::Kind::InvalidRequestData, "invalid JSON")
            )};
        }
        ProxyRequest input{};

        const auto* request_body = m_values.child(*tree, "request_body");
        if (request_body == nullptr) {
            return std::unexpected{m_errors.to_app_error(
                m_proxy_errors(ProxyError::Kind::InvalidRequestData, "request_body is required")
            )};
        }
        input.request_body = *request_body;

        const auto* destination = m_values.child(*tree, "destination_url");
        const auto destination_text =
            destination != nullptr ? m_values.as_string(*destination) : std::nullopt;
        if (!destination_text.has_value() || destination_text->empty()) {
            return std::unexpected{m_errors.to_app_error(
                m_proxy_errors(ProxyError::Kind::InvalidRequestData, "destination_url is not valid")
            )};
        }
        input.destination_url = *destination_text;

        const auto* method = m_values.child(*tree, "method");
        const auto method_text = method != nullptr ? m_values.as_string(*method) : std::nullopt;
        if (!method_text.has_value()) {
            return std::unexpected{m_errors.to_app_error(
                m_proxy_errors(ProxyError::Kind::InvalidRequestData, "method is not valid")
            )};
        }
        input.method = *method_text;

        const auto* token = m_values.child(*tree, "token");
        const auto token_text = token != nullptr ? m_values.as_string(*token) : std::nullopt;
        if (!token_text.has_value() || token_text->empty()) {
            return std::unexpected{m_errors.to_app_error(
                m_proxy_errors(ProxyError::Kind::InvalidRequestData, "token is required")
            )};
        }
        input.token = *token_text;

        const auto* token_type = m_values.child(*tree, "token_type");
        const auto type_text =
            token_type != nullptr ? m_values.as_string(*token_type) : std::nullopt;
        const auto parsed_type = type_text.has_value() ? token_type_from(*type_text) : std::nullopt;
        if (!parsed_type.has_value()) {
            return std::unexpected{m_errors.to_app_error(
                m_proxy_errors(ProxyError::Kind::InvalidRequestData, "token_type is not valid")
            )};
        }
        input.token_type = *parsed_type;

        const auto* headers = m_values.child(*tree, "headers");
        if (headers != nullptr) {
            if (const auto* object = std::get_if<interfaces::Value::Object>(&headers->get())) {
                for (const auto& [name, value]: *object) {
                    if (const auto text = m_values.as_string(value)) {
                        input.headers[name] = *text;
                    }
                }
            }
        }
        return input;
    }

private:
    [[nodiscard]] std::optional<LocalTokenType> token_type_from(std::string_view value) const
    {
        std::string normalized{};
        for (const char character: value) {
            if (character != '_' && character != '-' &&
                !std::isspace(static_cast<unsigned char>(character))) {
                normalized.push_back(
                    static_cast<char>(std::tolower(static_cast<unsigned char>(character)))
                );
            }
        }
        if (normalized == "paymentmethodid") {
            return LocalTokenType::PaymentMethodId;
        }
        if (normalized == "tokenizationid") {
            return LocalTokenType::TokenizationId;
        }
        if (normalized == "volatilepaymentmethodid") {
            return LocalTokenType::VolatilePaymentMethodId;
        }
        if (normalized == "paymentmethodtoken") {
            return LocalTokenType::PaymentMethodToken;
        }
        return std::nullopt;
    }

    ::app::context::AppContext& m_context;
    ValueTools m_values{};
    ProxyErrorBuilder m_proxy_errors{};
    ProxyErrorSupport m_errors{};
};

/**
 * @brief Domain policy consumed by core::behavior::RequestSupport.
 *
 * Proxy auth (V2 API key with both connected and platform self-operation
 * disabled) stays in ProxyInputSupport. The merchant-id scope is the header
 * value when present, otherwise the authenticated identity's merchant id.
 */
class ProxyPolicy
{
public:
    explicit ProxyPolicy(ProxyInputSupport& support) noexcept :
        m_support{support}
    {
    }

    [[nodiscard]] std::optional<core::error::AppError> authorize(
        interfaces::io::IRequest& req,
        std::string_view,
        const core::behavior::AuthSpec&
    ) const
    {
        return m_support.authorize(req).error;
    }

    [[nodiscard]] core::behavior::Scope scope(interfaces::io::IRequest& req) const
    {
        core::behavior::Scope result;
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        const std::string header = ::app::http::merchant_id_from_header{}(req);
        result.merchant_id = header.empty() ? auth.merchant_id() : header;
        return result;
    }

    [[nodiscard]] std::optional<core::error::AppError>
    scope_error(const core::behavior::Scope&) const noexcept
    {
        return std::nullopt;
    }

private:
    ProxyInputSupport& m_support;
};

} // namespace app::handlers::proxy
