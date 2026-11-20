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

/** @brief Instance-only ASCII/text helpers. No free or static functions. */
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
 * @brief Deterministic local substitute for upstream ContentType::from_headers:
 * case-insensitive lookup, conflicting-duplicate rejection, parameter stripping,
 * and the application/json default.
 */
class ContentTypeInspector
{
public:
    struct Outcome
    {
        bool ok{false};
        bool json{false};
        bool xml{false};
        std::string value{};
    };

    Outcome operator()(const app::dto::proxy::Headers& headers) const
    {
        std::optional<std::string> discovered{};
        for (const auto& [name, value] : headers.get_value()) {
            if (m_text.lower(name) != "content-type") {
                continue;
            }
            if (discovered.has_value() && m_text.lower(*discovered) != m_text.lower(value)) {
                return Outcome{};
            }
            discovered = value;
        }

        Outcome outcome{};
        std::string chosen = discovered.value_or(std::string{"application/json"});
        const std::size_t semicolon = chosen.find(';');
        if (semicolon != std::string::npos) {
            chosen = chosen.substr(0, semicolon);
        }
        outcome.value = m_text.trim(m_text.lower(chosen));
        if (outcome.value == "application/json" || outcome.value.ends_with("+json")) {
            outcome.ok = true;
            outcome.json = true;
        } else if (outcome.value == "application/xml" || outcome.value == "text/xml"
                   || outcome.value.ends_with("+xml")) {
            outcome.ok = true;
            outcome.xml = true;
        }
        return outcome;
    }

private:
    TextTools m_text{};
};

/**
 * @brief SSRF-safe destination check. Upstream validate_destination() resolves the host
 * to public IPs; without DNS this substitute rejects non-public literal IPs and local
 * names, and honours the operator bypass list from ConfigStore.
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
            return std::string{"destination_url must use http or https"};
        }

        const std::size_t end = rest.find_first_of("/?#");
        std::string_view authority = rest.substr(0, end);
        if (authority.empty()) {
            return std::string{"destination_url has no host"};
        }
        if (const std::size_t at = authority.rfind('@'); at != std::string_view::npos) {
            authority = authority.substr(at + 1);
        }
        std::string_view host = authority;
        if (const std::size_t colon = host.rfind(':');
            colon != std::string_view::npos && host.find(']') == std::string_view::npos) {
            host = host.substr(0, colon);
        }
        if (host.empty()) {
            return std::string{"destination_url has no host"};
        }

        const std::string host_lower = m_text.lower(host);
        if (is_bypassed(host_lower, bypass_csv)) {
            return std::nullopt;
        }
        if (is_local_name(host_lower) || is_private_literal(host_lower)) {
            return std::string{"destination_url host is not publicly routable"};
        }
        return std::nullopt;
    }

private:
    [[nodiscard]] bool is_bypassed(
        std::string_view host, std::string_view bypass_csv) const
    {
        for (const std::string& entry : m_text.split(bypass_csv, ',')) {
            if (!entry.empty() && m_text.trim(entry) == host) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool is_local_name(std::string_view host) const
    {
        if (host == "localhost" || host.ends_with(".localhost")
            || host.ends_with(".local") || host.ends_with(".internal")
            || host.ends_with(".localdomain")) {
            return true;
        }
        const bool dotted = host.find('.') != std::string_view::npos;
        const bool ip_literal = host.find(':') != std::string_view::npos
            || (host.front() >= '0' && host.front() <= '9');
        return !dotted && !ip_literal;
    }

    [[nodiscard]] bool is_private_literal(std::string_view host) const
    {
        std::string_view value = host;
        if (value.size() >= 2 && value.front() == '[' && value.back() == ']') {
            value = value.substr(1, value.size() - 2);
        }
        if (value == "::1" || value == "::" || value.starts_with("fc")
            || value.starts_with("fd") || value.starts_with("fe80:")
            || value.starts_with("::ffff:127.")) {
            return true;
        }

        std::array<int, 4> octets{};
        std::size_t index{0};
        std::size_t start{0};
        while (index < 4) {
            const std::size_t dot = value.find('.', start);
            const std::string_view part = value.substr(
                start, dot == std::string_view::npos ? std::string_view::npos : dot - start);
            if (part.empty() || part.size() > 3) {
                return false;
            }
            int number{0};
            for (const char character : part) {
                if (character < '0' || character > '9') {
                    return false;
                }
                number = number * 10 + (character - '0');
            }
            if (number > 255) {
                return false;
            }
            octets[index] = number;
            ++index;
            if (dot == std::string_view::npos) {
                break;
            }
            start = dot + 1;
        }
        if (index != 4) {
            return false;
        }

        const int first = octets[0];
        const int second = octets[1];
        const int third = octets[2];
        if (first == 0 || first == 10 || first == 127 || first >= 224) {
            return true;
        }
        if (first == 169 && second == 254) {
            return true;
        }
        if (first == 172 && second >= 16 && second <= 31) {
            return true;
        }
        if (first == 192 && second == 168) {
            return true;
        }
        if (first == 100 && second >= 64 && second <= 127) {
            return true;
        }
        if (first == 192 && second == 0 && (third == 0 || third == 2)) {
            return true;
        }
        return first == 198 && (second == 18 || second == 19);
    }

    TextTools m_text{};
};

/** @brief Maps a connector status string onto the downstream status carried in ProxyResponse. */
class ConnectorStatusMapper
{
public:
    std::uint16_t operator()(std::string_view status) const
    {
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

private:
    TextTools m_text{};
};

/**
 * @brief Validates/interpolates the outbound body and rebuilds its header set.
 * Content-Type/Content-Length/Transfer-Encoding from the caller are dropped before
 * the replacement header set is produced.
 */
class BodyRenderer
{
public:
    struct Rendered
    {
        std::string body{};
        std::unordered_map<std::string, std::string> headers{};
    };

    Rendered operator()(
        std::string_view template_body,
        const std::unordered_map<std::string, std::string>& vault_values,
        const app::dto::proxy::Headers& caller_headers,
        std::string_view content_type,
        bool xml
    ) const
    {
        auto forwarded = sanitize(caller_headers);
        forwarded["content-type"] = std::string{content_type};

        std::string rendered{template_body};
        for (const auto& [field, value] : vault_values) {
            rendered = m_text.replace_all(rendered, "{{$" + std::string{field} + "}}", std::string{value});
        }
        if (xml) {
            rendered = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><proxy_request>"
                + m_text.escape_xml(rendered) + "</proxy_request>";
        }
        return Rendered{std::move(rendered), std::move(forwarded)};
    }

private:
    [[nodiscard]] std::unordered_map<std::string, std::string>
    sanitize(const app::dto::proxy::Headers& headers) const
    {
        std::unordered_map<std::string, std::string> result{};
        for (const auto& [name, value] : headers.get_value()) {
            const std::string lowered = m_text.lower(name);
            if (lowered == "content-type" || lowered == "content-length"
                || lowered == "transfer-encoding") {
                continue;
            }
            result[lowered] = value;
        }
        return result;
    }

    TextTools m_text{};
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
        // Upstream V2ApiKeyAuth with connected-scope and platform-self disabled.
        // Every credential/permission decision comes from the registry; the profile
        // header check is added because upstream scopes the call by X-Profile-Id.
        const auto base_rule = m_context->auth_rules().rule_for("proxy");
        const auto rule = app::auth::RouteAuthRuleBuilder{}
                              .scheme(base_rule.scheme())
                              .permission(base_rule.permission())
                              .allow_connected(base_rule.allow_connected())
                              .allow_platform(base_rule.allow_platform())
                              .id_source("x-profile-id")
                              .build();
        const auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        const auto auth = m_context->authorizer().authorize_route(auth_ctx, rule);
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        // Bind the forward to the calling identity, never to the request body.
        const auto header_merchant_id = app::http::merchant_id_from_header{}(req);
        const auto header_org_id = app::http::org_id_from_header{}(req);
        const auto header_profile_id = app::http::HeaderReader{}.get(req, "x-profile-id");
        const auto merchant_id = header_merchant_id.empty()
            ? auth_ctx.merchant_id()
            : header_merchant_id;
        const auto org_id = header_org_id.empty()
            ? auth_ctx.org_id()
            : header_org_id;
        const auto profile_id = header_profile_id.empty()
            ? auth_ctx.profile_id()
            : header_profile_id;
        (void)org_id;
        (void)profile_id;

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::proxy::ProxyRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream validate_destination(): a bad destination is InvalidRequestData,
        // not a malformed JSON document. Bypass hosts are operator config data.
        const auto bypass_hosts = m_context->config_store()
                                      .get_value("proxy", "bypass_proxy_hosts")
                                      .value_or(std::string{});
        const auto& destination = parsed->get_destination_url().get_value();
        if (const auto problem = DestinationValidator{}(destination, bypass_hosts)) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST, *problem));
            return;
        }

        // Upstream Method is a validated enum; unknown verbs cannot be forwarded.
        const auto method = TextTools{}.upper(parsed->get_method().get_value());
        if (!is_supported_method(method)) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST, "method is not valid"));
            return;
        }
        // Upstream: GET/DELETE/HEAD forward with no body and never consume the CVC.
        const bool forwards_body = method != "GET" && method != "DELETE" && method != "HEAD";

        // Content-Type is parsed case-insensitively before any body work.
        const auto content = ContentTypeInspector{}(parsed->get_headers());
        if (!content.ok) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "unsupported or conflicting Content-Type"));
            return;
        }

        // Body validation happens before the vault read so an invalid request
        // never consumes the single-use CVC.
        std::string template_body{};
        if (forwards_body) {
            template_body = serde::Ser::encode_json(parsed->get_request_body());
            if (template_body.empty() || template_body == "null" || template_body == "{}") {
                send_error(res, send, core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "request_body is required for this method"));
                return;
            }
        }

        // Token resolution across all four upstream TokenTypes.
        const auto& token = parsed->get_token();
        if (token.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST, "token is required"));
            return;
        }
        const app::crypto::SecretString token_secret{token};
        const auto token_digest = app::crypto::KeyedHasher{"proxy.token"}.hash(token_secret.value());
        auto resolved = resolve_token(parsed->get_token_type(), token, token_digest);
        if (!resolved) {
            m_context->metrics().counter("proxy.token_not_found").increment();
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND, "payment method / token not found"));
            return;
        }

        // ProxyRecord::validate_ownership(): reject a token of another merchant.
        if (resolved->merchant_id.empty() || resolved->merchant_id != merchant_id) {
            m_context->metrics().counter("proxy.ownership_denied").increment();
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "token does not belong to the authenticated merchant"));
            return;
        }

        // Vault data retrieval. The local vault is the injected KeyStore via the
        // secret resolver; the CVC is read once through the one-time state store.
        const auto vault_key = resolved->vault_id.empty()
            ? token_digest
            : resolved->vault_id;
        std::unordered_map<std::string, std::string> vault_values{};
        for (const auto field : {"number", "expiry", "cvc", "card_holder_name"}) {
            const auto purpose = std::string{"proxy.vault."} + field;
            if (const auto secret = m_context->secret_resolver().resolve(purpose, vault_key)) {
                vault_values[field] = *secret;
            }
        }
        const auto cvc = vault_values.find("cvc");
        if (cvc == vault_values.end() || cvc->second.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "vault CVC not available for token"));
            return;
        }
        const auto cvc_hash = app::crypto::KeyedHasher{"proxy.cvc"}.hash(
            vault_key + ":" + cvc->second);
        if (!m_context->one_time_store().consume_totp(cvc_hash, 0)) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "vault CVC has already been consumed"));
            return;
        }

        // Format-aware interpolation and hop-by-hop header rebuild.
        const auto rendered = BodyRenderer{}(
            template_body, vault_values, parsed->get_headers(), content.value, content.xml);

        // Real outbound call: select a connector and forward the rendered envelope.
        // sync() is the local stand-in for call_connector_api(state, req, "proxy", None).
        auto outbound = app::connector::ConnectorRequestBuilder{}
                            .operation("proxy")
                            .merchant_id(merchant_id)
                            .resource_id(resolved->vault_id.empty() ? token : resolved->vault_id)
                            .raw_payload(build_envelope(rendered))
                            ();
        auto selected = m_context->connector_router().select_for(outbound);
        if (!selected) {
            const auto error = m_context->connector_router().last_error();
            m_context->metrics().counter("proxy.connector_unavailable").increment();
            send_error(res, send, error.value_or(core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::CONNECTOR_ERROR,
                "no outbound connector registered for proxy")));
            return;
        }
        const auto connector_result = selected->sync(outbound);
        if (connector_result.get_retryable() || connector_result.get_status().empty()) {
            m_context->metrics().counter("proxy.connector_failed").increment();
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INTERNAL_ERROR, "proxy connector failed"));
            return;
        }

        // Response body conversion: JSON else UTF-8 text, with real status and headers.
        const auto& raw = connector_result.get_raw_json();
        const bool json_response = !raw.empty() && (raw.front() == '{' || raw.front() == '[');
        std::unordered_map<std::string, std::string> response_headers{};
        response_headers["content-type"] = json_response
            ? "application/json"
            : "text/plain; charset=utf-8";
        response_headers["x-proxy-reference"] = connector_result.get_reference_id();
        app::dto::proxy::Headers headers_dto{};
        headers_dto.set_value(std::move(response_headers));

        app::dto::proxy::ProxyResponse response{};
        response.set_response(json_response
            ? app::dto::proxy::ProxyResponseBody::Json
            : app::dto::proxy::ProxyResponseBody::Text);
        response.set_status_code(ConnectorStatusMapper{}(connector_result.get_status()));
        response.set_response_headers(std::move(headers_dto));
        auto text = serde::Ser::encode_json(response);

        m_context->metrics().record_flow("proxy");
        m_context->metrics().counter("proxy.proxy").increment();
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/proxy"}
            .add_route(
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
        Router{router, "/v2/proxy"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_proxy(req, res, std::move(send));
                    })
                    .summary("Hyperswitch proxy v2")
            );
        Router{router, "/v1/proxy"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_proxy(req, res, std::move(send));
                    })
                    .summary("Hyperswitch proxy v1")
            );
    }

    explicit ProxyRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    struct ResolvedRecord
    {
        std::string merchant_id{};
        std::string profile_id{};
        std::string vault_id{};
        std::string kind{};
    };

    [[nodiscard]] bool is_supported_method(std::string_view method) const
    {
        return method == "GET" || method == "POST" || method == "PUT"
            || method == "PATCH" || method == "DELETE" || method == "HEAD";
    }

    [[nodiscard]] std::optional<ResolvedRecord> resolve_token(
        app::dto::proxy::TokenType type,
        std::string_view token,
        std::string_view digest
    ) const
    {
        switch (type) {
        case app::dto::proxy::TokenType::PaymentMethodId:
        case app::dto::proxy::TokenType::PaymentMethodToken:
            return resolve_payment_method(token);
        case app::dto::proxy::TokenType::TokenizationId:
            return resolve_tokenization(token);
        case app::dto::proxy::TokenType::VolatilePaymentMethodId:
            return resolve_volatile(digest);
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<ResolvedRecord> resolve_payment_method(
        std::string_view token
    ) const
    {
        auto record = m_context->repository().find_payment_method_by_string(token);
        if (!record) {
            return std::nullopt;
        }
        ResolvedRecord resolved{};
        resolved.merchant_id = record->get_merchant_id();
        resolved.profile_id = record->get_profile_id();
        resolved.vault_id = record->get_vault_id();
        resolved.kind = "payment_method";
        return resolved;
    }

    [[nodiscard]] std::optional<ResolvedRecord> resolve_tokenization(
        std::string_view token
    ) const
    {
        auto record = m_context->repository().find_tokenization_by_string(token);
        if (!record) {
            return std::nullopt;
        }
        ResolvedRecord resolved{};
        resolved.merchant_id = record->get_merchant_id();
        resolved.vault_id = record->get_vault_id();
        resolved.kind = "tokenization";
        return resolved;
    }

    [[nodiscard]] std::optional<ResolvedRecord> resolve_volatile(
        std::string_view digest
    ) const
    {
        const auto result = m_context->cache().get(
            std::string{"proxy.volatile."} + std::string{digest});
        if (!result.hit) {
            return std::nullopt;
        }
        const auto packed = core::cache::value_as<std::string>(result.value);
        const auto parts = TextTools{}.split(packed, '|');
        if (parts.size() < 3) {
            return std::nullopt;
        }
        ResolvedRecord resolved{};
        resolved.merchant_id = parts[0];
        resolved.profile_id = parts[1];
        resolved.vault_id = parts[2];
        resolved.kind = "volatile";
        return resolved;
    }

    [[nodiscard]] std::string build_envelope(const BodyRenderer::Rendered& rendered) const
    {
        std::string headers_json{"{"};
        bool first{true};
        for (const auto& [name, value] : rendered.headers) {
            if (!first) {
                headers_json += ",";
            }
            headers_json += serde::Ser::encode_json(name)
                + ":" + serde::Ser::encode_json(value);
            first = false;
        }
        headers_json += "}";
        return std::format(
            "{{\"headers\":{},\"body\":{}}}",
            headers_json,
            serde::Ser::encode_json(rendered.body)
        );
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
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

} // namespace app::handlers::proxy
