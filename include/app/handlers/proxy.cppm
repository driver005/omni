module;
export module app_handlers:proxy;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import core_cache;
import core_error;
import app_store;
import app_connector;

namespace app::handlers::proxy {

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
 */

class TextTools
{
public:
    [[nodiscard]] std::string lower(std::string_view value) const
    {
        std::string result{};
        result.reserve(value.size());
        for (const char character : value) {
            result.push_back(static_cast<char>(
                std::tolower(static_cast<unsigned char>(character))));
        }
        return result;
    }

    [[nodiscard]] std::string upper(std::string_view value) const
    {
        std::string result{};
        result.reserve(value.size());
        for (const char character : value) {
            result.push_back(static_cast<char>(
                std::toupper(static_cast<unsigned char>(character))));
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

    [[nodiscard]] std::string replace_all(
        std::string subject, std::string_view needle, std::string_view replacement) const
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
        for (const char character : value) {
            switch (character) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            default: result.push_back(character); break;
            }
        }
        return result;
    }
};

/**
 * @brief Local mirror of upstream `ApiErrorResponse` variants used by the proxy flow.
 * Each maps onto the exact upstream wire shape produced by `ErrorResponse`/`ApiError`
 * (`api_models/src/errors/types.rs:13-35,60-71,89-105`) and HTTP status.
 */
struct ProxyError
{
    enum class Kind : std::uint8_t
    {
        InvalidRequestData,     // IR_06, HTTP 422
        PaymentMethodNotFound,  // HE_02, HTTP 404
        UnprocessableEntity,    // IR_29, HTTP 422
        InternalServerError,    // HE_00, HTTP 500
        Unauthorized,           // IR_18, HTTP 401
        Forbidden,              // IR_22, HTTP 403
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

/** @brief Native navigation of a `serde::Value` (rfl::Generic) tree. */
class ValueTools
{
public:
    [[nodiscard]] const interfaces::Value*
    child(const interfaces::Value& node, std::string_view key) const
    {
        if (const auto* object = std::get_if<interfaces::Value::Object>(&node.get())) {
            for (const auto& [name, value] : *object) {
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

struct ProxyInput
{
    interfaces::Value request_body{};
    std::string destination_url{};
    std::unordered_map<std::string, std::string> headers{};
    std::string method{};
    std::string token{};
    LocalTokenType token_type{LocalTokenType::PaymentMethodId};
};

/**
 * @brief Port of `ProxyRequestWrapper::get_content_type` (`core/proxy/utils.rs:176-206`)
 * and `ConnectorContentType` (`common_utils/src/request.rs:67-111`). Content-Type is
 * parsed once; conflicting duplicates and invalid MIME values are rejected.
 */
class ContentTypeInspector
{
public:
    enum class Kind : std::uint8_t
    {
        Json,
        Xml,
        Unsupported,
    };

    struct Content
    {
        Kind kind{Kind::Json};
        std::string header{"application/json"};
        bool utf8{true};
        bool declared{false};
    };

    std::expected<Content, ProxyError>
    operator()(const std::unordered_map<std::string, std::string>& headers) const
    {
        std::optional<std::string> first{};
        for (const auto& [name, value] : headers) {
            if (m_text.lower(name) != "content-type") {
                continue;
            }
            if (first.has_value() && *first != value) {
                return std::unexpected{m_errors{}(
                    ProxyError::Kind::InvalidRequestData,
                    "Conflicting connector Content-Type headers")};
            }
            first = value;
        }

        Content content{};
        if (!first.has_value()) {
            return content; // JSON default (upstream: None => application/json).
        }
        content.declared = true;
        content.header = *first; // Upstream `header()` returns the value as received.

        const std::size_t semicolon = first->find(';');
        const std::string essence = m_text.trim(m_text.lower(
            semicolon == std::string::npos ? *first : first->substr(0, semicolon)));
        if (!valid_mime(essence)) {
            return std::unexpected{m_errors{}(
                ProxyError::Kind::InvalidRequestData,
                "Invalid connector Content-Type header")};
        }
        if (essence == "application/json" || essence.ends_with("+json")) {
            content.kind = Kind::Json;
        } else if (essence == "application/xml" || essence == "text/xml"
                   || essence.ends_with("+xml")) {
            content.kind = Kind::Xml;
        } else {
            content.kind = Kind::Unsupported;
        }
        content.utf8 = utf8_params(semicolon == std::string::npos
            ? std::string_view{}
            : std::string_view{*first}.substr(semicolon + 1));
        return content;
    }

private:
    [[nodiscard]] bool valid_mime(std::string_view essence) const
    {
        const std::size_t slash = essence.find('/');
        if (slash == std::string_view::npos || slash == 0 || slash + 1 >= essence.size()
            || essence.find('/', slash + 1) != std::string_view::npos) {
            return false;
        }
        const auto valid_token = [](char character)
        {
            return std::isalnum(static_cast<unsigned char>(character)) != 0
                || character == '-' || character == '.' || character == '+';
        };
        return std::ranges::all_of(essence, valid_token);
    }

    [[nodiscard]] bool utf8_params(std::string_view params) const
    {
        const auto pieces = m_text.split(std::string{params}, ';');
        for (const auto& piece : pieces) {
            const std::size_t equals = piece.find('=');
            if (equals == std::string::npos) {
                continue;
            }
            if (m_text.trim(m_text.lower(piece.substr(0, equals))) != "charset") {
                continue;
            }
            const std::string value = m_text.trim(m_text.lower(piece.substr(equals + 1)));
            if (value != "utf-8" && value != "utf8" && value != "\"utf-8\""
                && value != "\"utf8\"") {
                return false;
            }
        }
        return true;
    }

    TextTools m_text{};
    ProxyErrorBuilder m_errors{};
};

/**
 * @brief Port of `outbound_destination::validate_destination`
 * (`external_services/.../outbound_destination.rs:25-59`) and `SafeOutboundUrl`
 * parsing (`common_utils/src/outbound_url.rs:26-103`). DNS resolution of a domain
 * name is the documented external-infra substitute; literal IP routability,
 * scheme validation and the bypass list are reproduced exactly.
 */
class DestinationValidator
{
public:
    std::optional<std::string> operator()(
        std::string_view url, std::string_view bypass_csv) const
    {
        std::string_view rest{url};
        if (rest.starts_with("https://")) {
            rest.remove_prefix(8);
        } else if (rest.starts_with("http://")) {
            rest.remove_prefix(7);
        } else {
            return std::string{"destination_url is not valid"};
        }

        const std::size_t end = rest.find_first_of("/?#");
        std::string_view authority = rest.substr(0, end);
        if (const std::size_t at = authority.rfind('@'); at != std::string_view::npos) {
            authority = authority.substr(at + 1);
        }
        std::string_view host = authority;
        if (const std::size_t colon = host.rfind(':');
            colon != std::string_view::npos && host.find(']') == std::string_view::npos) {
            host = host.substr(0, colon);
        }
        if (host.empty()) {
            return std::string{"destination_url is not valid"};
        }

        const std::string host_lower = m_text.lower(host);
        if (is_bypassed(host_lower, bypass_csv)) {
            return std::string{"destination_url is not valid"};
        }
        if (is_literal(host_lower) && !is_global_literal(host_lower)) {
            return std::string{"destination_url is not valid"};
        }
        // Host::Domain: upstream resolves via DNS and requires public addresses.
        // Without DNS (external infra) this step is a deterministic local pass.
        return std::nullopt;
    }

private:
    [[nodiscard]] bool is_bypassed(
        std::string_view host, std::string_view bypass_csv) const
    {
        for (const std::string& raw : m_text.split(bypass_csv, ',')) {
            std::string entry = m_text.trim(raw);
            while (!entry.empty() && entry.front() == '.') {
                entry.erase(entry.begin());
            }
            while (!entry.empty() && entry.back() == '.') {
                entry.pop_back();
            }
            if (entry.empty()) {
                continue;
            }
            const std::string lower = m_text.lower(entry);
            if (lower == "*" || lower == std::string{host}) {
                return true;
            }
            if (host.size() > lower.size() && host.ends_with(lower)
                && host[host.size() - lower.size() - 1] == '.') {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool is_literal(std::string_view host) const
    {
        if (host.size() >= 2 && host.front() == '[' && host.back() == ']') {
            return true;
        }
        return !host.empty() && host.front() >= '0' && host.front() <= '9';
    }

    [[nodiscard]] bool is_global_literal(std::string_view host) const
    {
        std::string_view value = host;
        if (value.size() >= 2 && value.front() == '[' && value.back() == ']') {
            value = value.substr(1, value.size() - 2);
        }
        if (value == "::1" || value == "::" || value.starts_with("fc")
            || value.starts_with("fd") || value.starts_with("fe80:")
            || value.starts_with("::ffff:127.")) {
            return false;
        }

        std::array<int, 4> octets{};
        std::size_t index{0};
        std::size_t start{0};
        while (index < 4) {
            const std::size_t dot = value.find('.', start);
            const std::string_view part = value.substr(
                start, dot == std::string_view::npos ? std::string_view::npos : dot - start);
            if (part.empty() || part.size() > 3) {
                return true; // not an IPv4 literal we can judge -> treat as public
            }
            int number{0};
            for (const char character : part) {
                if (character < '0' || character > '9') {
                    return true;
                }
                number = number * 10 + (character - '0');
            }
            if (number > 255) {
                return true;
            }
            octets[index] = number;
            ++index;
            if (dot == std::string_view::npos) {
                break;
            }
            start = dot + 1;
        }
        if (index != 4) {
            return true;
        }

        const int first = octets[0];
        const int second = octets[1];
        const int third = octets[2];
        if (first == 0 || first == 10 || first == 127 || first >= 224) {
            return false;
        }
        if (first == 169 && second == 254) {
            return false;
        }
        if (first == 172 && second >= 16 && second <= 31) {
            return false;
        }
        if (first == 192 && second == 168) {
            return false;
        }
        if (first == 100 && second >= 64 && second <= 127) {
            return false;
        }
        if (first == 192 && second == 0 && (third == 0 || third == 2)) {
            return false;
        }
        return !(first == 198 && (second == 18 || second == 19));
    }

    TextTools m_text{};
};

/** @brief Port of `core/proxy/body.rs` ProxyBodyError and its Display strings. */
class ProxyBodyErrorTools
{
public:
    [[nodiscard]] std::string message(std::string_view code, std::string_view field) const
    {
        if (code == "InvalidPlaceholder") {
            return "Malformed vault placeholder";
        }
        if (code == "InvalidField") {
            return "Missing or non-scalar vault field: " + std::string{field};
        }
        if (code == "UnsupportedContentType") {
            return "Unsupported connector Content-Type; only JSON and XML are supported";
        }
        if (code == "InvalidContentType") {
            return "Connector body does not match its Content-Type";
        }
        if (code == "InvalidXml") {
            return "Invalid XML template";
        }
        return "Failed to serialize connector body";
    }
};

struct BodyError
{
    std::string code{};
    std::string field{};
};

/**
 * @brief Port of `core/proxy/body/placeholder.rs`: `{{$field}}` parsing, vault
 * field lookup, scalar coercion and substitution shared by JSON and XML bodies.
 */
class Placeholder
{
public:
    struct Parsed
    {
        std::string field{};
        std::size_t consumed{0};
    };

    [[nodiscard]] const interfaces::Value*
    find_field(const interfaces::Value& vault, std::string_view field) const
    {
        if (const auto* object = std::get_if<interfaces::Value::Object>(&vault.get())) {
            for (const auto& [name, value] : *object) {
                if (name == field) {
                    return &value;
                }
            }
            for (const auto& [name, value] : *object) {
                if (const auto* found = find_field(value, field)) {
                    return found;
                }
            }
        } else if (const auto* array = std::get_if<interfaces::Value::Array>(&vault.get())) {
            for (const auto& value : *array) {
                if (const auto* found = find_field(value, field)) {
                    return found;
                }
            }
        }
        return nullptr;
    }

    [[nodiscard]] std::expected<std::string, BodyError>
    scalar(const interfaces::Value& vault, std::string_view field) const
    {
        const auto* found = find_field(vault, field);
        if (found == nullptr) {
            return std::unexpected{BodyError{"InvalidField", std::string{field}}};
        }
        if (std::holds_alternative<std::string>(found->get())
            || std::holds_alternative<std::int64_t>(found->get())
            || std::holds_alternative<double>(found->get())
            || std::holds_alternative<bool>(found->get())) {
            return m_values.scalar_text(*found);
        }
        return std::unexpected{BodyError{"InvalidField", std::string{field}}};
    }

    [[nodiscard]] std::expected<interfaces::Value, BodyError>
    value(const interfaces::Value& vault, std::string_view field) const
    {
        const auto* found = find_field(vault, field);
        if (found == nullptr) {
            return std::unexpected{BodyError{"InvalidField", std::string{field}}};
        }
        return *found;
    }

    [[nodiscard]] std::expected<std::optional<Parsed>, BodyError>
    parse_placeholder(std::string_view input) const
    {
        if (!input.starts_with("{{")) {
            return std::optional<Parsed>{};
        }
        const std::string_view rest = input.substr(2);
        std::size_t whitespace{0};
        while (whitespace < rest.size()
               && std::isspace(static_cast<unsigned char>(rest[whitespace]))) {
            ++whitespace;
        }
        if (whitespace >= rest.size() || rest[whitespace] != '$') {
            return std::optional<Parsed>{};
        }
        const std::size_t end = rest.find("}}");
        if (end == std::string_view::npos) {
            return std::unexpected{BodyError{"InvalidPlaceholder", {}}};
        }
        std::string_view raw = m_text.trim(rest.substr(0, end));
        if (!raw.starts_with('$')) {
            return std::unexpected{BodyError{"InvalidPlaceholder", {}}};
        }
        const std::string field{raw.substr(1)};
        if (field.empty()) {
            return std::unexpected{BodyError{"InvalidPlaceholder", {}}};
        }
        for (const char character : field) {
            const bool valid = std::isalnum(static_cast<unsigned char>(character)) != 0
                || character == '_';
            if (!valid) {
                return std::unexpected{BodyError{"InvalidPlaceholder", {}}};
            }
        }
        return Parsed{field, end + 4};
    }

    [[nodiscard]] std::expected<std::string, BodyError> substitute(
        std::string_view template_text,
        const interfaces::Value& vault,
        bool xml_escape
    ) const
    {
        std::string output{};
        output.reserve(template_text.size());
        std::string_view rest{template_text};
        while (true) {
            const std::size_t start = rest.find("{{");
            if (start == std::string_view::npos) {
                output.append(rest);
                break;
            }
            output.append(rest.substr(0, start));
            rest = rest.substr(start);
            auto parsed = parse_placeholder(rest);
            if (!parsed) {
                return std::unexpected{std::move(parsed.error())};
            }
            if (parsed->has_value()) {
                auto resolved = scalar(vault, parsed->field);
                if (!resolved) {
                    return std::unexpected{std::move(resolved.error())};
                }
                output += xml_escape ? m_text.escape_xml(*resolved) : *resolved;
                rest.remove_prefix(parsed->consumed);
            } else {
                output += "{{";
                rest.remove_prefix(2);
            }
        }
        return output;
    }

    [[nodiscard]] std::expected<void, BodyError>
    validate_placeholders(std::string_view template_text) const
    {
        std::size_t from{0};
        while (true) {
            const std::size_t start = template_text.find("{{", from);
            if (start == std::string_view::npos) {
                return {};
            }
            auto parsed = parse_placeholder(template_text.substr(start));
            if (!parsed) {
                return std::unexpected{std::move(parsed.error())};
            }
            from = start + 2;
        }
    }

    [[nodiscard]] std::expected<bool, BodyError>
    has_reference(std::string_view template_text) const
    {
        std::size_t from{0};
        while (true) {
            const std::size_t start = template_text.find("{{", from);
            if (start == std::string_view::npos) {
                return false;
            }
            auto parsed = parse_placeholder(template_text.substr(start));
            if (!parsed) {
                return std::unexpected{std::move(parsed.error())};
            }
            if (parsed->has_value()) {
                return true;
            }
            from = start + 2;
        }
    }

private:
    TextTools m_text{};
    ValueTools m_values{};
};

/**
 * @brief Port of `core/proxy/body/json.rs`: recursive interpolation of JSON string
 * values (keys are never interpolated; a whole-value placeholder keeps its JSON type).
 */
class JsonBody
{
public:
    [[nodiscard]] std::expected<void, BodyError>
    validate(const interfaces::Value& item) const
    {
        if (std::holds_alternative<std::string>(item.get())) {
            return m_placeholder.validate_placeholders(std::get<std::string>(item.get()));
        }
        if (const auto* array = std::get_if<interfaces::Value::Array>(&item.get())) {
            for (const auto& value : *array) {
                if (auto result = validate(value); !result) {
                    return std::unexpected{std::move(result.error())};
                }
            }
        } else if (const auto* object = std::get_if<interfaces::Value::Object>(&item.get())) {
            for (const auto& [name, value] : *object) {
                if (auto result = validate(value); !result) {
                    return std::unexpected{std::move(result.error())};
                }
            }
        }
        return {};
    }

    [[nodiscard]] std::expected<std::string, BodyError>
    render(const interfaces::Value& body, const interfaces::Value& vault) const
    {
        auto resolved = resolve(body, vault);
        if (!resolved) {
            return std::unexpected{std::move(resolved.error())};
        }
        return serde::Ser::encode_json(*resolved);
    }

private:
    [[nodiscard]] std::expected<interfaces::Value, BodyError>
    resolve(const interfaces::Value& item, const interfaces::Value& vault) const
    {
        if (const auto* text = std::get_if<std::string>(&item.get())) {
            auto parsed = m_placeholder.parse_placeholder(*text);
            if (!parsed) {
                return std::unexpected{std::move(parsed.error())};
            }
            if (parsed->has_value() && parsed->consumed == text->size()) {
                return m_placeholder.value(vault, parsed->field);
            }
            auto replaced = m_placeholder.substitute(*text, vault, false);
            if (!replaced) {
                return std::unexpected{std::move(replaced.error())};
            }
            return interfaces::Value{std::move(*replaced)};
        }
        if (const auto* array = std::get_if<interfaces::Value::Array>(&item.get())) {
            interfaces::Value::Array result{};
            result.reserve(array->size());
            for (const auto& value : *array) {
                auto child = resolve(value, vault);
                if (!child) {
                    return std::unexpected{std::move(child.error())};
                }
                result.push_back(std::move(*child));
            }
            return interfaces::Value{std::move(result)};
        }
        if (const auto* object = std::get_if<interfaces::Value::Object>(&item.get())) {
            interfaces::Value::Object result{};
            for (const auto& [name, value] : *object) {
                auto child = resolve(value, vault);
                if (!child) {
                    return std::unexpected{std::move(child.error())};
                }
                result[name] = std::move(*child);
            }
            return interfaces::Value{std::move(result)};
        }
        return item; // number / bool / null: copied unchanged.
    }

    Placeholder m_placeholder{};
};

/**
 * @brief Port of `core/proxy/body/xml.rs`: validate a well-formed UTF-8 XML template
 * whose placeholders sit only in text/attribute/CDATA, then interpolate with escaping.
 * A deterministic local scanner replaces `roxmltree`/`quick_xml` (in-process library
 * substitute); placeholder escape rules and error strings match upstream.
 */
class XmlBody
{
public:
    [[nodiscard]] std::expected<void, BodyError>
    validate(std::string_view template_text) const
    {
        if (auto result = m_placeholder.validate_placeholders(template_text); !result) {
            return std::unexpected{std::move(result.error())};
        }
        if (!well_formed(template_text)) {
            return std::unexpected{BodyError{"InvalidXml", {}}};
        }
        const std::size_t root = root_start(template_text);
        const std::string prolog = root == std::string_view::npos
            ? std::string{template_text}
            : std::string{template_text.substr(0, root)};
        if (has_reference_text(prolog)) {
            return std::unexpected{BodyError{"InvalidXml", {}}};
        }
        std::size_t from{0};
        while (true) {
            const std::size_t comment = template_text.find("<!--", from);
            if (comment == std::string_view::npos) {
                break;
            }
            const std::size_t close = template_text.find("-->", comment + 4);
            if (close == std::string_view::npos
                || has_reference_text(template_text.substr(comment, close - comment))) {
                return std::unexpected{BodyError{"InvalidXml", {}}};
            }
            from = close + 3;
        }
        if (!declares_utf8(template_text)) {
            return std::unexpected{BodyError{"InvalidXml", {}}};
        }
        return {};
    }

    [[nodiscard]] std::expected<std::string, BodyError>
    render(std::string_view template_text, const interfaces::Value& vault) const
    {
        std::string output{};
        output.reserve(template_text.size());
        std::size_t cursor{0};
        while (cursor < template_text.size()) {
            const std::size_t open = template_text.find('<', cursor);
            if (open == std::string_view::npos) {
                auto text = m_placeholder.substitute(template_text.substr(cursor), vault, true);
                if (!text) {
                    return std::unexpected{std::move(text.error())};
                }
                output += *text;
                break;
            }
            if (open > cursor) {
                auto text = m_placeholder.substitute(
                    template_text.substr(cursor, open - cursor), vault, true);
                if (!text) {
                    return std::unexpected{std::move(text.error())};
                }
                output += *text;
            }

            if (template_text.substr(open, 9) == "<![CDATA[") {
                const std::size_t close = template_text.find("]]>", open + 9);
                if (close == std::string_view::npos) {
                    return std::unexpected{BodyError{"InvalidXml", {}}};
                }
                auto data = m_placeholder.substitute(
                    template_text.substr(open + 9, close - open - 9), vault, false);
                if (!data) {
                    return std::unexpected{std::move(data.error())};
                }
                const std::string cdata = m_text.replace_all(*data, "]]>", "]]]]><![CDATA[>");
                output += "<![CDATA[" + cdata + "]]>";
                cursor = close + 3;
                continue;
            }
            if (template_text.substr(open, 4) == "<!--") {
                const std::size_t close = template_text.find("-->", open + 4);
                if (close == std::string_view::npos) {
                    return std::unexpected{BodyError{"InvalidXml", {}}};
                }
                output.append(template_text.substr(open, close + 3 - open));
                cursor = close + 3;
                continue;
            }
            if (template_text.substr(open, 2) == "<?") {
                const std::size_t close = template_text.find("?>", open + 2);
                if (close == std::string_view::npos) {
                    return std::unexpected{BodyError{"InvalidXml", {}}};
                }
                output.append(template_text.substr(open, close + 2 - open));
                cursor = close + 2;
                continue;
            }
            const std::size_t close = template_text.find('>', open);
            if (close == std::string_view::npos) {
                return std::unexpected{BodyError{"InvalidXml", {}}};
            }
            auto tag = render_tag(template_text.substr(open + 1, close - open - 1), vault);
            if (!tag) {
                return std::unexpected{std::move(tag.error())};
            }
            output += '<' + *tag + '>';
            cursor = close + 1;
        }
        return output;
    }

private:
    [[nodiscard]] std::expected<std::string, BodyError>
    render_tag(std::string_view tag, const interfaces::Value& vault) const
    {
        std::string output{};
        bool in_quote{false};
        std::size_t value_start{0};
        for (std::size_t index = 0; index < tag.size(); ++index) {
            const char character = tag[index];
            if (character == '"') {
                if (!in_quote) {
                    output.append(tag.substr(value_start, index - value_start));
                    value_start = index;
                    in_quote = true;
                } else {
                    auto value = m_placeholder.substitute(
                        tag.substr(value_start, index + 1 - value_start), vault, true);
                    if (!value) {
                        return std::unexpected{std::move(value.error())};
                    }
                    output += *value;
                    in_quote = false;
                    value_start = index + 1;
                }
            }
        }
        output.append(tag.substr(value_start));
        return output;
    }

    [[nodiscard]] bool has_reference_text(std::string_view text) const
    {
        auto result = m_placeholder.has_reference(text);
        return result.has_value() && *result;
    }

    [[nodiscard]] bool declares_utf8(std::string_view template_text) const
    {
        if (!template_text.starts_with("<?xml")) {
            return true;
        }
        const std::size_t close = template_text.find("?>");
        if (close == std::string_view::npos) {
            return false;
        }
        const std::string declaration{template_text.substr(0, close)};
        const std::size_t key = m_text.lower(declaration).find("encoding");
        if (key == std::string::npos) {
            return true;
        }
        const std::size_t equals = declaration.find('=', key);
        if (equals == std::string::npos) {
            return true;
        }
        std::string value = m_text.trim(declaration.substr(equals + 1));
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        } else if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
            value = value.substr(1, value.size() - 2);
        }
        const std::string lowered = m_text.lower(value);
        return lowered == "utf-8" || lowered == "utf8";
    }

    [[nodiscard]] std::size_t root_start(std::string_view template_text) const
    {
        std::size_t cursor{0};
        while (cursor < template_text.size()) {
            if (template_text[cursor] != '<') {
                ++cursor;
                continue;
            }
            if (template_text.substr(cursor, 2) == "<?"
                || template_text.substr(cursor, 4) == "<!--") {
                const std::string terminator = template_text.substr(cursor, 2) == "<?" ? "?>" : "-->";
                const std::size_t close = template_text.find(terminator, cursor + 2);
                cursor = close == std::string_view::npos
                    ? template_text.size()
                    : close + terminator.size();
                continue;
            }
            return cursor;
        }
        return std::string_view::npos;
    }

    [[nodiscard]] bool well_formed(std::string_view template_text) const
    {
        std::vector<std::string> stack{};
        std::size_t cursor{0};
        while (cursor < template_text.size()) {
            const std::size_t open = template_text.find('<', cursor);
            if (open == std::string_view::npos) {
                break;
            }
            if (template_text.substr(open, 9) == "<![CDATA[") {
                const std::size_t close = template_text.find("]]>", open + 9);
                if (close == std::string_view::npos) {
                    return false;
                }
                cursor = close + 3;
                continue;
            }
            if (template_text.substr(open, 4) == "<!--") {
                const std::size_t close = template_text.find("-->", open + 4);
                if (close == std::string_view::npos) {
                    return false;
                }
                cursor = close + 3;
                continue;
            }
            const std::size_t close = template_text.find('>', open);
            if (close == std::string_view::npos) {
                return false;
            }
            std::string_view inner = template_text.substr(open + 1, close - open - 1);
            if (inner.starts_with("?") || inner.starts_with("!")) {
                cursor = close + 1;
                continue;
            }
            if (inner.starts_with("/")) {
                const std::string name{m_text.trim(inner.substr(1))};
                if (stack.empty() || stack.back() != name) {
                    return false;
                }
                stack.pop_back();
            } else {
                const std::size_t space = inner.find_first_of(" \t\r\n/");
                const std::string name{inner.substr(0, space)};
                if (name.empty() || !inner.ends_with('/')) {
                    stack.push_back(name);
                }
            }
            cursor = close + 1;
        }
        return stack.empty();
    }

    TextTools m_text{};
    Placeholder m_placeholder{};
};

/**
 * @brief Port of `ProxyBody` + the `Init -> Validated -> Prepared` builder
 * (`core/proxy/body.rs:44-178`). Body structure and placeholder syntax are checked
 * before the vault is read, so an invalid request never consumes the single-use CVC.
 */
class ProxyBody
{
public:
    struct Rendered
    {
        std::string bytes{};
        std::string content_type{};
        std::unordered_map<std::string, std::string> headers{};
    };

    std::expected<ProxyBody, BodyError>
    make(const interfaces::Value& body, const ContentTypeInspector::Content& content) const
    {
        ProxyBody result{};
        result.m_content = content;
        switch (content.kind) {
        case ContentTypeInspector::Kind::Json:
            result.m_json = body;
            result.m_is_xml = false;
            return result;
        case ContentTypeInspector::Kind::Xml:
            if (const auto* text = std::get_if<std::string>(&body.get());
                text != nullptr && content.utf8) {
                result.m_xml = *text;
                result.m_is_xml = true;
                return result;
            }
            return std::unexpected{BodyError{"InvalidContentType", {}}};
        case ContentTypeInspector::Kind::Unsupported:
            return std::unexpected{BodyError{"UnsupportedContentType", {}}};
        }
        return std::unexpected{BodyError{"UnsupportedContentType", {}}};
    }

    [[nodiscard]] std::expected<void, BodyError> validate() const
    {
        return m_is_xml ? m_xml_body.validate(m_xml) : m_json_body.validate(m_json);
    }

    [[nodiscard]] std::expected<Rendered, BodyError>
    build(
        const interfaces::Value& vault,
        const std::unordered_map<std::string, std::string>& caller_headers
    ) const
    {
        Rendered rendered{};
        if (m_is_xml) {
            auto result = m_xml_body.render(m_xml, vault);
            if (!result) {
                return std::unexpected{std::move(result.error())};
            }
            rendered.bytes = std::move(*result);
        } else {
            auto result = m_json_body.render(m_json, vault);
            if (!result) {
                return std::unexpected{std::move(result.error())};
            }
            rendered.bytes = std::move(*result);
        }
        rendered.content_type = m_content.declared
            ? m_content.header
            : (m_is_xml ? "application/xml" : "application/json");

        // Upstream drops caller Content-Type/Content-Length/Transfer-Encoding.
        for (const auto& [name, value] : caller_headers) {
            const std::string lowered = m_text.lower(name);
            if (lowered == "content-type" || lowered == "content-length"
                || lowered == "transfer-encoding") {
                continue;
            }
            rendered.headers[lowered] = value;
        }
        rendered.headers["Content-Type"] = rendered.content_type;
        return rendered;
    }

private:
    bool m_is_xml{false};
    interfaces::Value m_json{};
    std::string m_xml{};
    ContentTypeInspector::Content m_content{};
    JsonBody m_json_body{};
    XmlBody m_xml_body{};
    TextTools m_text{};
};

/**
 * @brief Port of `api_models::proxy::Headers::from_header_map` (`api_models/proxy.rs:12-28`)
 * plus the local connector's header substitute.
 */
class ResponseHeaders
{
public:
    std::unordered_map<std::string, std::string>
    from_response(bool json, std::string_view reference_id) const
    {
        std::unordered_map<std::string, std::string> result{};
        result["content-type"] = json ? "application/json" : "application/xml";
        if (!reference_id.empty()) {
            result["x-proxy-reference"] = std::string{reference_id};
        }
        return result;
    }

    [[nodiscard]] std::string encode(
        const std::unordered_map<std::string, std::string>& headers) const
    {
        std::string text{"{"};
        bool first{true};
        for (const auto& [name, value] : headers) {
            if (!first) {
                text += ",";
            }
            text += serde::Ser::encode_json(name) + ":" + serde::Ser::encode_json(value);
            first = false;
        }
        text += "}";
        return text;
    }
};

/**
 * @brief Downstream status/body conversion. Upstream reads `res.status_code` and
 * `res.headers` from the actual HTTP response; the local connector exposes a status
 * string and a reference id instead, so a numeric parse/derivation is the documented
 * external-infra substitute for those two fields.
 */
class ConnectorResponseReader
{
public:
    [[nodiscard]] std::uint16_t status_code(const app::connector::ConnectorResponse& res) const
    {
        const std::string& status = res.get_status();
        if (!status.empty()
            && std::ranges::all_of(status, [](char c) { return c >= '0' && c <= '9'; })) {
            std::uint16_t parsed{};
            const auto value = std::from_chars(
                status.data(), status.data() + status.size(), parsed);
            if (value.ec == std::errc{} && value.ptr == status.data() + status.size()) {
                return parsed;
            }
        }
        const std::string lowered = m_text.lower(status);
        if (lowered.find("fail") != std::string::npos
            || lowered.find("error") != std::string::npos
            || lowered.find("declin") != std::string::npos) {
            return 502;
        }
        if (lowered.find("pending") != std::string::npos) {
            return 202;
        }
        return 200;
    }

    struct ResponseBody
    {
        std::string encoded{};
        bool json{false};
    };

    /**
     * @brief Port of upstream `response_body` (`core/proxy.rs:126-137`): JSON when the
     * bytes parse as JSON, otherwise UTF-8 text; other encodings are rejected.
     */
    [[nodiscard]] std::expected<ResponseBody, ProxyError>
    response_body(std::string_view raw) const
    {
        if (auto value = serde::Ser::decode_generic("application/json", raw)) {
            return ResponseBody{serde::Ser::encode_json(*value), true}; // bare JSON value
        }
        if (!is_utf8(raw)) {
            return std::unexpected{m_errors{}(
                ProxyError::Kind::InternalServerError,
                "Connector response is neither JSON nor UTF-8 text")};
        }
        return ResponseBody{serde::Ser::encode_json(std::string{raw}), false}; // bare text
    }

private:
    [[nodiscard]] bool is_utf8(std::string_view text) const
    {
        std::size_t index{0};
        while (index < text.size()) {
            const unsigned char byte = static_cast<unsigned char>(text[index]);
            std::size_t length{0};
            std::uint32_t codepoint{0};
            if (byte < 0x80) {
                ++index;
                continue;
            } else if ((byte & 0xE0) == 0xC0) {
                length = 2;
                codepoint = byte & 0x1F;
            } else if ((byte & 0xF0) == 0xE0) {
                length = 3;
                codepoint = byte & 0x0F;
            } else if ((byte & 0xF8) == 0xF0) {
                length = 4;
                codepoint = byte & 0x07;
            } else {
                return false;
            }
            if (index + length > text.size()) {
                return false;
            }
            for (std::size_t offset = 1; offset < length; ++offset) {
                const unsigned char next = static_cast<unsigned char>(text[index + offset]);
                if ((next & 0xC0) != 0x80) {
                    return false;
                }
                codepoint = (codepoint << 6) | (next & 0x3F);
            }
            if (length == 2 && codepoint < 0x80) {
                return false;
            }
            if (length == 3 && codepoint < 0x800) {
                return false;
            }
            if (length == 4 && codepoint < 0x10000) {
                return false;
            }
            if (codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
                return false;
            }
            index += length;
        }
        return true;
    }

    TextTools m_text{};
    ProxyErrorBuilder m_errors{};
};

} // namespace app::handlers::proxy

export namespace app::handlers::proxy {

class ProxyRouteHandlers
{
public:
    void handle_proxy(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream: V2ApiKeyAuth { allow_connected_scope_operation: false,
        // allow_platform_self_operation: false } with X-Profile-Id scoping
        // (routes/proxy.rs:27-30, openapi/routes/proxy.rs:46-51). Expressed
        // literally rather than read from a registry substitute.
        const auto rule = app::auth::RouteAuthRuleBuilder{}
                              .scheme(app::auth::AuthScheme::V2_API_KEY)
                              .permission(app::auth::AppPermission::ADMIN)
                              .allow_connected(false)
                              .allow_platform(false)
                              .id_source("x-profile-id")
                              .build();
        const auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        const auto auth = m_context->authorizer().authorize_route(auth_ctx, rule);
        if (auth) {
            send_auth_error(res, send, *auth);
            return;
        }

        // Bind the forward to the authenticated identity, never to the request body.
        const auto header_merchant_id = app::http::merchant_id_from_header{}(req);
        const auto merchant_id = header_merchant_id.empty()
            ? auth_ctx.merchant_id()
            : header_merchant_id;

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, m_errors{}(
                ProxyError::Kind::InvalidRequestData, "empty request"));
            return;
        }
        auto input = parse_request(body);
        if (!input) {
            send_error(res, send, std::move(input.error()));
            return;
        }

        // proxy_core step 1: validate_destination() -> InvalidRequestData "destination_url is not valid".
        const auto bypass_hosts = m_context->config_store()
                                      .get_value("proxy", "bypass_proxy_hosts")
                                      .value_or(std::string{});
        if (DestinationValidator{}(input->destination_url, bypass_hosts)) {
            send_error(res, send, m_errors{}(
                ProxyError::Kind::InvalidRequestData, "destination_url is not valid"));
            return;
        }

        // Upstream Method is a five-value enum (common_utils/src/request.rs:37-57).
        const auto method = TextTools{}.upper(input->method);
        if (!is_supported_method(method)) {
            send_error(res, send, m_errors{}(
                ProxyError::Kind::InvalidRequestData, "method is not valid"));
            return;
        }

        std::string rendered_bytes{};
        std::unordered_map<std::string, std::string> outbound_headers{};
        const bool body_method = method != "GET" && method != "DELETE";

        std::optional<ProxyRecordValue> record{};
        if (body_method) {
            // Upstream body_request(): parse Content-Type, then construct and validate
            // the body before fetching the proxy record or reading the vault.
            auto content = ContentTypeInspector{}(input->headers);
            if (!content) {
                send_error(res, send, std::move(content.error()));
                return;
            }
            auto made = ProxyBody{}.make(input->request_body, *content);
            if (!made) {
                send_error(res, send, invalid_body(*made.error()));
                return;
            }
            auto proxy_body = std::move(*made);
            if (auto result = proxy_body.validate(); !result) {
                send_error(res, send, invalid_body(result.error()));
                return;
            }
            auto resolved = get_proxy_record(input->token_type, input->token, merchant_id);
            if (!resolved) {
                send_error(res, send, std::move(resolved.error()));
                return;
            }
            record = *std::move(resolved);
            auto vault = get_vault_data(*record);
            if (!vault) {
                send_error(res, send, std::move(vault.error()));
                return;
            }
            auto built = proxy_body.build(*vault, input->headers);
            if (!built) {
                send_error(res, send, invalid_body(built.error()));
                return;
            }
            rendered_bytes = std::move(built->bytes);
            outbound_headers = std::move(built->headers);
        } else {
            // Upstream GET/DELETE: fetch and validate the token but send no body and
            // never read the vault; attach default headers plus the caller's headers.
            auto resolved = get_proxy_record(input->token_type, input->token, merchant_id);
            if (!resolved) {
                send_error(res, send, std::move(resolved.error()));
                return;
            }
            record = *std::move(resolved);
            for (const auto& [name, value] : input->headers) {
                outbound_headers[TextTools{}.lower(name)] = value;
            }
        }

        // proxy_core step 3: execute_proxy_request() via call_connector_api("proxy").
        const auto envelope = build_envelope(
            input->destination_url, method, outbound_headers, rendered_bytes, body_method);
        auto response = execute_proxy_request(
            merchant_id,
            record->vault_id.empty() ? input->token : record->vault_id,
            envelope);
        if (!response) {
            send_error(res, send, std::move(response.error()));
            return;
        }

        // proxy_core step 4: ProxyResponse::try_from(ProxyResponseWrapper(res)).
        auto text = build_proxy_response(*response);
        if (!text) {
            send_error(res, send, std::move(text.error()));
            return;
        }

        m_context->metrics().record_flow("proxy");
        m_context->metrics().counter("proxy.proxy").increment();
        send_json(res, send, *text);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        // Upstream v2 scope is "/{payment_methods_prefix}/proxy" (default "v2",
        // app.rs:1043-1052, configs.rs:85-89); docs add /v2/proxy and /v1/proxy
        // (openapi/routes/proxy.rs:7,61). The bare "/proxy" has no upstream counterpart.
        const auto prefix = m_context->config_store()
                                .get_value("micro_services", "payment_methods_prefix")
                                .value_or(std::string{"v2"});
        std::vector<std::string> paths{"/v2/proxy", "/v1/proxy"};
        if (!prefix.empty() && prefix != "v2") {
            paths.push_back("/" + prefix + "/proxy");
        }
        for (const auto& path : paths) {
            Router{router, path}.add_route(
                Route{""}
                    .post([this](
                              interfaces::io::IRequest& req,
                              interfaces::io::IResponse& res,
                              std::function<void()> send
                          ) {
                        handle_proxy(req, res, std::move(send));
                    })
                    .summary("Hyperswitch proxy")
            );
        }
    }

    explicit ProxyRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    struct ProxyRecordValue
    {
        enum class Kind : std::uint8_t
        {
            PaymentMethod,
            VolatilePaymentMethod,
            Tokenization,
        };

        Kind kind{Kind::PaymentMethod};
        std::string merchant_id{};
        std::string profile_id{};
        std::string customer_id{};
        std::string vault_id{};
        std::string payment_method_id{};
    };

    [[nodiscard]] bool is_supported_method(std::string_view method) const
    {
        return method == "GET" || method == "POST" || method == "PUT"
            || method == "PATCH" || method == "DELETE";
    }

    [[nodiscard]] std::optional<LocalTokenType> token_type_from(std::string_view value) const
    {
        std::string normalized{};
        for (const char character : value) {
            if (character != '_' && character != '-' && !std::isspace(
                    static_cast<unsigned char>(character))) {
                normalized.push_back(static_cast<char>(
                    std::tolower(static_cast<unsigned char>(character))));
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

    /** @brief Decode the exact upstream ProxyRequest wire shape via core serde. */
    [[nodiscard]] std::expected<ProxyInput, ProxyError>
    parse_request(std::string_view body) const
    {
        auto tree = serde::Ser::decode_generic("application/json", body);
        if (!tree) {
            return std::unexpected{m_errors{}(
                ProxyError::Kind::InvalidRequestData, "invalid JSON")};
        }
        ValueTools values{};
        ProxyInput input{};

        const auto* request_body = values.child(*tree, "request_body");
        if (request_body == nullptr) {
            return std::unexpected{m_errors{}(
                ProxyError::Kind::InvalidRequestData, "request_body is required")};
        }
        input.request_body = *request_body;

        const auto* destination = values.child(*tree, "destination_url");
        const auto destination_text = destination != nullptr
            ? values.as_string(*destination)
            : std::nullopt;
        if (!destination_text.has_value() || destination_text->empty()) {
            return std::unexpected{m_errors{}(
                ProxyError::Kind::InvalidRequestData, "destination_url is not valid")};
        }
        input.destination_url = *destination_text;

        const auto* method = values.child(*tree, "method");
        const auto method_text = method != nullptr ? values.as_string(*method) : std::nullopt;
        if (!method_text.has_value()) {
            return std::unexpected{m_errors{}(
                ProxyError::Kind::InvalidRequestData, "method is not valid")};
        }
        input.method = *method_text;

        const auto* token = values.child(*tree, "token");
        const auto token_text = token != nullptr ? values.as_string(*token) : std::nullopt;
        if (!token_text.has_value() || token_text->empty()) {
            return std::unexpected{m_errors{}(
                ProxyError::Kind::InvalidRequestData, "token is required")};
        }
        input.token = *token_text;

        const auto* token_type = values.child(*tree, "token_type");
        const auto type_text = token_type != nullptr
            ? values.as_string(*token_type)
            : std::nullopt;
        const auto parsed_type = type_text.has_value()
            ? token_type_from(*type_text)
            : std::nullopt;
        if (!parsed_type.has_value()) {
            return std::unexpected{m_errors{}(
                ProxyError::Kind::InvalidRequestData, "token_type is not valid")};
        }
        input.token_type = *parsed_type;

        const auto* headers = values.child(*tree, "headers");
        if (headers != nullptr) {
            if (const auto* object = std::get_if<interfaces::Value::Object>(&headers->get())) {
                for (const auto& [name, value] : *object) {
                    if (const auto text = values.as_string(value)) {
                        input.headers[name] = *text;
                    }
                }
            }
        }
        return input;
    }

    /**
     * @brief Port of `ProxyRequestWrapper::get_proxy_record` + `fetch_proxy_record` +
     * `ProxyRecord::validate_ownership` (`core/proxy/utils.rs:34-165,217-232`).
     */
    [[nodiscard]] std::expected<ProxyRecordValue, ProxyError>
    get_proxy_record(LocalTokenType type, std::string_view token, std::string_view merchant_id) const
    {
        std::optional<ProxyRecordValue> record{};

        switch (type) {
        case LocalTokenType::PaymentMethodId:
        case LocalTokenType::PaymentMethodToken: {
            auto found = m_context->repository().find_payment_method_by_string(token);
            if (!found) {
                return std::unexpected{m_errors{}(
                    ProxyError::Kind::PaymentMethodNotFound,
                    "Payment method does not exist in our records")};
            }
            ProxyRecordValue value{};
            value.kind = ProxyRecordValue::Kind::PaymentMethod;
            value.merchant_id = found->get_merchant_id();
            value.profile_id = found->get_profile_id();
            value.customer_id = found->get_customer_id();
            value.vault_id = found->get_vault_id();
            value.payment_method_id = found->get_payment_method_id();
            record = std::move(value);
            break;
        }
        case LocalTokenType::TokenizationId: {
            auto found = m_context->repository().find_tokenization_by_string(token);
            if (!found) {
                return std::unexpected{m_errors{}(
                    ProxyError::Kind::InternalServerError,
                    "Error while fetching tokenization record from vault")};
            }
            ProxyRecordValue value{};
            value.kind = ProxyRecordValue::Kind::Tokenization;
            value.merchant_id = found->get_merchant_id();
            value.customer_id = found->get_customer_id();
            value.vault_id = found->get_vault_id();
            record = std::move(value);
            break;
        }
        case LocalTokenType::VolatilePaymentMethodId: {
            const auto cached = m_context->cache().get(std::string{token});
            if (!cached.hit) {
                return std::unexpected{m_errors{}(
                    ProxyError::Kind::UnprocessableEntity, "Token is invalid or expired")};
            }
            const auto packed = core::cache::value_as<std::string>(cached.value);
            const auto parts = TextTools{}.split(packed, '|');
            if (parts.size() < 3) {
                return std::unexpected{m_errors{}(
                    ProxyError::Kind::UnprocessableEntity, "Token is invalid or expired")};
            }
            ProxyRecordValue value{};
            value.kind = ProxyRecordValue::Kind::VolatilePaymentMethod;
            value.merchant_id = parts[0];
            value.profile_id = parts[1];
            value.vault_id = parts[2];
            value.customer_id = parts.size() > 3 ? parts[3] : std::string{};
            value.payment_method_id = parts.size() > 4 ? parts[4] : std::string{token};
            record = std::move(value);
            break;
        }
        }
        if (!record.has_value()) {
            return std::unexpected{m_errors{}(
                ProxyError::Kind::PaymentMethodNotFound,
                "Payment method does not exist in our records")};
        }
        if (record->merchant_id.empty() || record->merchant_id != merchant_id) {
            m_context->metrics().counter("proxy.ownership_denied").increment();
            return std::unexpected{m_errors{}(
                ProxyError::Kind::PaymentMethodNotFound,
                "Payment method does not exist in our records")};
        }
        return *record;
    }

    /**
     * @brief Port of `ProxyRecord::get_vault_data` (`core/proxy/utils.rs:274-417`).
     * The injected secret resolver stands in for the PCI vault; a failed single-use
     * CVC read only warns (it never fails the request), matching upstream.
     */
    [[nodiscard]] std::expected<interfaces::Value, ProxyError>
    get_vault_data(const ProxyRecordValue& record) const
    {
        const std::string vault_key = record.vault_id;
        auto card = interfaces::Value::Object{};
        bool found_any{false};
        bool has_cvc{false};
        const auto read = [&](std::string_view purpose) -> std::string
        {
            if (const auto secret = m_context->secret_resolver().resolve(
                    std::string{purpose}, vault_key)) {
                found_any = true;
                return *secret;
            }
            return {};
        };

        const std::string number = read("proxy.vault.number");
        const std::string month = read("proxy.vault.expiry_month");
        const std::string year = read("proxy.vault.expiry_year");
        const std::string cvc = read("proxy.vault.cvc");
        const std::string holder = read("proxy.vault.card_holder_name");
        card["card_number"] = interfaces::Value{number};
        card["card_exp_month"] = interfaces::Value{month};
        card["card_exp_year"] = interfaces::Value{year};
        card["card_holder_name"] = interfaces::Value{holder};

        if (!cvc.empty()) {
            const auto cvc_hash = app::crypto::KeyedHasher{"proxy.cvc"}.hash(
                vault_key + ":" + cvc);
            if (m_context->one_time_store().consume_totp(cvc_hash, 0)) {
                card["cvc"] = interfaces::Value{cvc};
                has_cvc = true;
            } else {
                // Upstream logs a warning and proceeds without the CVC.
                m_context->metrics().counter("proxy.cvc_consumed").increment();
            }
        }
        if (!found_any) {
            const auto kind = record.kind == ProxyRecordValue::Kind::VolatilePaymentMethod
                ? ProxyError::Kind::UnprocessableEntity
                : ProxyError::Kind::InternalServerError;
            const std::string message = kind == ProxyError::Kind::UnprocessableEntity
                ? std::string{"Token is invalid or expired"}
                : std::string{"Error while fetching data from vault"};
            return std::unexpected{m_errors{}(kind, message)};
        }

        auto vault = interfaces::Value::Object{};
        vault["card"] = interfaces::Value{std::move(card)};
        vault["number"] = interfaces::Value{number};
        vault["cvc"] = has_cvc ? interfaces::Value{cvc} : interfaces::Value{};
        return interfaces::Value{std::move(vault)};
    }

    [[nodiscard]] ProxyError invalid_body(const BodyError& error) const
    {
        return m_errors{}(
            ProxyError::Kind::InvalidRequestData,
            ProxyBodyErrorTools{}.message(error.code, error.field));
    }

    [[nodiscard]] std::string build_envelope(
        std::string_view url,
        std::string_view method,
        const std::unordered_map<std::string, std::string>& headers,
        std::string_view body,
        bool with_body
    ) const
    {
        ResponseHeaders header_tools{};
        const std::string encoded_headers = header_tools.encode(headers);
        return std::format(
            R"({{"url":{},"method":{},"headers":{},"body":{}}})",
            serde::Ser::encode_json(std::string{url}),
            serde::Ser::encode_json(std::string{method}),
            encoded_headers,
            serde::Ser::encode_json(std::string{with_body ? body : std::string_view{}})
        );
    }

    /** @brief Port of `execute_proxy_request` (`core/proxy.rs:90-109`). */
    [[nodiscard]] std::expected<app::connector::ConnectorResponse, ProxyError>
    execute_proxy_request(
        std::string_view merchant_id,
        std::string_view resource_id,
        std::string_view envelope
    ) const
    {
        auto outbound = app::connector::ConnectorRequestBuilder{}
                            .operation("proxy")
                            .merchant_id(std::string{merchant_id})
                            .resource_id(std::string{resource_id})
                            .raw_payload(std::string{envelope})
                            ();
        auto selected = m_context->connector_router().select_for(outbound);
        if (!selected) {
            m_context->metrics().counter("proxy.connector_unavailable").increment();
            return std::unexpected{m_errors{}(
                ProxyError::Kind::InternalServerError, "Failed to call the destination")};
        }
        const auto result = selected->sync(outbound);
        if (result.get_retryable() || result.get_status().empty()) {
            m_context->metrics().counter("proxy.connector_failed").increment();
            return std::unexpected{m_errors{}(
                ProxyError::Kind::InternalServerError, "Error while receiving response")};
        }
        return result;
    }

    /** @brief Port of `ProxyResponseWrapper::try_from` (`core/proxy.rs:111-137`). */
    [[nodiscard]] std::expected<std::string, ProxyError>
    build_proxy_response(const app::connector::ConnectorResponse& result) const
    {
        ConnectorResponseReader reader{};
        auto body = reader.response_body(result.get_raw_json());
        if (!body) {
            return std::unexpected{std::move(body.error())};
        }
        ResponseHeaders headers{};
        const auto map = headers.from_response(body->json, result.get_reference_id());
        return std::format(
            R"({{"response":{},"status_code":{},"response_headers":{}}})",
            body->encoded,
            reader.status_code(result),
            headers.encode(map)
        );
    }

    /** @brief Upstream error wire shape (api_models/src/errors/types.rs). */
    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        ProxyError error
    ) const
    {
        std::string_view type{"invalid_request"};
        std::string_view code{"IR_06"};
        interfaces::io::types::Status status{interfaces::io::types::Status::UNPROCESSABLE_CONTENT};
        std::string message = std::move(error.message);
        switch (error.kind) {
        case ProxyError::Kind::InvalidRequestData:
            code = "IR_06";
            status = interfaces::io::types::Status::UNPROCESSABLE_CONTENT;
            break;
        case ProxyError::Kind::PaymentMethodNotFound:
            code = "HE_02";
            status = interfaces::io::types::Status::NOT_FOUND;
            break;
        case ProxyError::Kind::UnprocessableEntity:
            code = "IR_29";
            status = interfaces::io::types::Status::UNPROCESSABLE_CONTENT;
            break;
        case ProxyError::Kind::InternalServerError:
            type = "api";
            code = "HE_00";
            status = interfaces::io::types::Status::INTERNAL_SERVER_ERROR;
            message = "Something went wrong";
            break;
        case ProxyError::Kind::Unauthorized:
            code = "IR_18";
            status = interfaces::io::types::Status::UNAUTHORIZED;
            break;
        case ProxyError::Kind::Forbidden:
            code = "IR_22";
            status = interfaces::io::types::Status::FORBIDDEN;
            break;
        }
        const std::string text = std::format(
            R"({{"error":{{"type":"{}","message":{},"code":"{}"}}}})",
            type,
            serde::Ser::encode_json(message),
            code
        );
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

    void send_auth_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        const core::error::AppError& error
    ) const
    {
        const bool forbidden = error.get_kind() == core::error::AppErrorKind::FORBIDDEN;
        send_error(res, send, m_errors{}(
            forbidden ? ProxyError::Kind::Forbidden : ProxyError::Kind::Unauthorized,
            error.get_message()));
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
    ProxyErrorBuilder m_errors{};
};

} // namespace app::handlers::proxy
