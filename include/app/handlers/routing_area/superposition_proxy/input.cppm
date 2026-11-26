export module app_routing:superposition_proxy_input;

import std;
import interfaces;
import core_serde;
import core_error;
import core_cache;
import core_router;
import app_context;
import app_auth;
import app_http;
import app_crypto;
import app_dto;
import app_model;
import utils_openapi;

export namespace app::handlers::superposition_proxy {

// smithy `Document` == serde::Value; doc maps == ordered string/Value maps.
using Doc = serde::Value;
using DocMap = std::map<std::string, Doc>;

// ApiErrorResponse wire model (hyperswitch_domain_models/errors/api_error_response.rs).
struct ApiError
{
    enum class Kind : std::uint8_t
    {
        InvalidRequestData,  // IR_06 / 422
        AccessForbidden,     // IR_22 / 403
        InternalServerError, // HE_00 / 500
        MissingHeader,       // 400 {"error":{"message":...}}
        BadRequestBody,      // 400 {"error":{"message":...}}
    };
    Kind kind{Kind::InternalServerError};
    std::string message{};
    std::string resource{};
};

struct ProxyScope
{
    std::string org_id{};
    std::string workspace_id{};
};

struct Pagination
{
    std::optional<std::int32_t> count{};
    std::optional<std::int32_t> page{};
    std::optional<bool> all{};
};

struct QueryPairs
{
    std::vector<std::pair<std::string, std::string>> values{};

    QueryPairs() = default;

    QueryPairs(std::vector<std::pair<std::string, std::string>> v) :
        values(std::move(v))
    {
    }

    [[nodiscard]] std::optional<std::string> first_value(std::string_view name) const
    {
        for (const auto& [key, value]: values) {
            if (key == name) {
                return value;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::vector<std::string> all_values(std::string_view name) const
    {
        std::vector<std::string> result{};
        for (const auto& [key, value]: values) {
            if (key == name) {
                result.push_back(value);
            }
        }
        return result;
    }

    // Upstream csv_values: split every matching value on ',', trim whitespace.
    [[nodiscard]] std::vector<std::string> csv_values(std::string_view name) const
    {
        std::vector<std::string> result{};
        for (const auto& [key, value]: values) {
            if (key != name) {
                continue;
            }
            std::size_t start{0};
            while (start <= value.size()) {
                const auto comma = value.find(',', start);
                const auto end = comma == std::string_view::npos ? value.size() : comma;
                const auto token = value.substr(start, end - start);
                const auto first = token.find_first_not_of(" \t");
                if (first != std::string_view::npos) {
                    const auto last = token.find_last_not_of(" \t");
                    result.emplace_back(token.substr(first, last - first + 1U));
                }
                if (comma == std::string_view::npos) {
                    break;
                }
                start = comma + 1U;
            }
        }
        return result;
    }

    [[nodiscard]] std::vector<std::pair<std::string, std::string>> dimension_params() const
    {
        std::vector<std::pair<std::string, std::string>> result{};
        for (const auto& pair: values) {
            if (pair.first.starts_with("dimension[")) {
                result.push_back(pair);
            }
        }
        return result;
    }
};

// ── Typed query structs (core/superposition_proxy.rs:194-315) ───────────────
struct ListContextsQuery
{
    Pagination pagination{};
    std::vector<std::string> prefix{};
    std::optional<std::string> sort_on{};
    std::optional<std::string> sort_by{};
    std::vector<std::string> created_by{};
    std::vector<std::string> last_modified_by{};
    std::optional<std::string> plaintext{};
    std::optional<std::string> dimension_match_strategy{};
    std::vector<std::pair<std::string, std::string>> dimension_params{};
};

struct ListDefaultConfigsQuery
{
    Pagination pagination{};
    std::optional<std::string> name{};
};

struct ListDimensionsQuery
{
    Pagination pagination{};
};

struct ListAuditLogsQuery
{
    Pagination pagination{};
    std::optional<std::string> from_date{};
    std::optional<std::string> to_date{};
    std::vector<std::string> table{};
    std::vector<std::string> action{};
    std::optional<std::string> username{};
    std::optional<std::string> sort_by{};
    std::vector<std::pair<std::string, std::string>> dimension_params{};
};

struct GetDimensionRequest
{
    std::string dimension{};
};

struct GetDefaultConfigRequest
{
    std::string key{};
};

// Upstream ResolveConfigBody.context: Map<String, Value> (order preserved).
struct ResolveDetailedConfigRequest
{
    std::vector<std::pair<std::string, Doc>> context{};
};

struct ResolveConfigExplanationRequest
{
    std::string key{};
    std::vector<std::pair<std::string, Doc>> context{};
};

// Upstream ContextPutRequest (superposition_types::api::context::PutRequest).
struct ContextPutRequest
{
    std::vector<std::pair<std::string, Doc>> context{};
    std::vector<std::pair<std::string, Doc>> override_values{};
    std::optional<std::string> description{};
    std::string change_reason{};
};

// ── One Input per handle_* method: everything the old flow call took ───────
struct ListContextsHandlerInput
{
    ListContextsQuery request{};
    ::app::auth::AuthContext auth{};
    std::string org_id{};
    std::string workspace_id{};
};

struct CreateContextHandlerInput
{
    ContextPutRequest request{};
    ::app::auth::AuthContext auth{};
    std::string org_id{};
    std::string workspace_id{};
};

struct ListDefaultConfigsHandlerInput
{
    ListDefaultConfigsQuery request{};
    ::app::auth::AuthContext auth{};
    std::string org_id{};
    std::string workspace_id{};
};

struct GetDefaultConfigHandlerInput
{
    GetDefaultConfigRequest request{};
    ::app::auth::AuthContext auth{};
    std::string org_id{};
    std::string workspace_id{};
};

struct ListDimensionsHandlerInput
{
    ListDimensionsQuery request{};
    ::app::auth::AuthContext auth{};
    std::string org_id{};
    std::string workspace_id{};
};

struct GetDimensionHandlerInput
{
    GetDimensionRequest request{};
    ::app::auth::AuthContext auth{};
    std::string org_id{};
    std::string workspace_id{};
};

struct ResolveDetailedConfigHandlerInput
{
    ResolveDetailedConfigRequest request{};
    ::app::auth::AuthContext auth{};
    std::string org_id{};
    std::string workspace_id{};
};

struct ResolveConfigExplanationHandlerInput
{
    ResolveConfigExplanationRequest request{};
    ::app::auth::AuthContext auth{};
    std::string org_id{};
    std::string workspace_id{};
};

struct ListAuditLogsHandlerInput
{
    ListAuditLogsQuery request{};
    ::app::auth::AuthContext auth{};
    std::string org_id{};
    std::string workspace_id{};
};

/**
 * @brief Parse/auth/path/query support for the superposition_proxy handlers.
 *
 * Only request data -> plain Input conversion. Business logic stays in runtime/.
 * Upstream error codes go into AppError::kind_name; the real, human-readable
 * message goes into AppError::message. Both use the shared AppError shape.
 */
class SuperpositionProxyInputSupport
{
public:
    explicit SuperpositionProxyInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ── ApiError builders + AppError wire mapping ─────────────────────────
    [[nodiscard]] ApiError invalid_request_data(std::string message) const
    {
        return {ApiError::Kind::InvalidRequestData, std::move(message), {}};
    }

    [[nodiscard]] ApiError access_forbidden(std::string resource) const
    {
        return {ApiError::Kind::AccessForbidden, {}, std::move(resource)};
    }

    [[nodiscard]] ApiError internal_server_error() const
    {
        return {ApiError::Kind::InternalServerError, "Something went wrong", {}};
    }

    [[nodiscard]] ApiError bad_request(std::string message) const
    {
        return {ApiError::Kind::BadRequestBody, std::move(message), {}};
    }

    // Upstream code preserved on the wire in AppError::kind_name.
    [[nodiscard]] std::string_view error_code(ApiError::Kind kind) const noexcept
    {
        switch (kind) {
            case ApiError::Kind::InvalidRequestData:
                return "IR_06";
            case ApiError::Kind::AccessForbidden:
                return "IR_22";
            case ApiError::Kind::InternalServerError:
                return "HE_00";
            case ApiError::Kind::MissingHeader:
                return "MissingHeader";
            case ApiError::Kind::BadRequestBody:
                return "BadRequestBody";
        }
        return "HE_00";
    }

    // AccessForbidden stores its resource instead of a message upstream.
    [[nodiscard]] std::string error_message(const ApiError& error) const
    {
        if (error.kind == ApiError::Kind::AccessForbidden) {
            return "Access forbidden. Not authorized to access this resource " + error.resource;
        }
        return error.message;
    }

    // Kind chosen so AppError::get_status() matches api_error_status():
    // IR_06 -> 422, IR_22 -> 403, HE_00 -> 500, header/body -> 400.
    [[nodiscard]] core::error::AppError make_api_error(const ApiError& error) const
    {
        using core::error::AppErrorKind;
        AppErrorKind kind{AppErrorKind::INTERNAL};
        switch (error.kind) {
            case ApiError::Kind::InvalidRequestData:
                kind = AppErrorKind::VALIDATION_FAILED;
                break;
            case ApiError::Kind::AccessForbidden:
                kind = AppErrorKind::FORBIDDEN;
                break;
            case ApiError::Kind::InternalServerError:
                kind = AppErrorKind::INTERNAL;
                break;
            case ApiError::Kind::MissingHeader:
            case ApiError::Kind::BadRequestBody:
                kind = AppErrorKind::INVALID_REQUEST;
                break;
        }
        auto result = core::error::AppErrorBuilder{}(kind, error_message(error));
        result.set_kind_name(std::string{error_code(error.kind)});
        return result;
    }

    // ── Auth (upstream JWT ProfileSuperpositionConfigRead/Write) ──────────
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_request(interfaces::io::IRequest& req, std::string_view operation) const
    {
        const auto rule = m_context.auth_rules().rule_for(operation);
        const auto auth = ::app::auth::AuthContextBuilder{}(req);
        // Upstream: allow_connected=true, allow_platform=true for every superposition route.
        // PROFILE_READ/WRITE is the in-process stand-in for the shared-contract
        // ProfileSuperpositionConfigRead/Write permission.
        const bool write = operation == "create_context";
        const auto rebased = ::app::auth::RouteAuthRuleBuilder{}
                                 .scheme(rule.scheme())
                                 .permission(
                                     write ? ::app::auth::AppPermission::PROFILE_WRITE
                                           : ::app::auth::AppPermission::PROFILE_READ
                                 )
                                 .allow_connected(true)
                                 .allow_platform(true)
                                 .build();
        const auto denied = m_context.authorizer().authorize_route(auth, rebased);
        if (!denied) {
            return std::nullopt;
        }
        return make_api_error(access_forbidden("superposition"));
    }

    // extract_proxy_headers + required_header (core:157-192).
    [[nodiscard]] std::expected<ProxyScope, core::error::AppError>
    extract_proxy_headers(interfaces::io::IRequest& req) const
    {
        const auto org_id = ::app::http::HeaderReader{}.get(req, "x-org-id");
        if (org_id.empty()) {
            return std::unexpected{make_api_error(
                {ApiError::Kind::MissingHeader, "missing required header: x-org-id", {}}
            )};
        }
        const auto workspace_id = ::app::http::HeaderReader{}.get(req, "x-workspace");
        if (workspace_id.empty()) {
            return std::unexpected{make_api_error(
                {ApiError::Kind::MissingHeader, "missing required header: x-workspace", {}}
            )};
        }
        // Config is the only trusted source for the Superposition org/workspace.
        const auto configured_org =
            m_context.config_store().get_value(proxy_config_ns(), "configured_org_id");
        const auto configured_workspace =
            m_context.config_store().get_value(proxy_config_ns(), "configured_workspace_id");
        if ((configured_org && *configured_org != org_id) ||
            (configured_workspace && *configured_workspace != workspace_id)) {
            return std::unexpected{
                make_api_error(access_forbidden("superposition org and workspace"))
            };
        }
        return ProxyScope{org_id, workspace_id};
    }

    void cache_scope(interfaces::io::IRequest& req, const ProxyScope& scope) const
    {
        const auto token = ::app::auth::TokenParser{}(req).value();
        const auto digest = ::app::crypto::KeyedHasher{"superposition_proxy.scope"}.hash(
            scope.org_id + ":" + scope.workspace_id + ":" + token
        );
        const auto key = "superposition_proxy.scope." + digest;
        if (!m_context.cache().get(key).hit) {
            m_context.cache().put(
                key,
                core::cache::to_cache_value(scope.org_id + ":" + scope.workspace_id)
            );
        }
    }

    // ── Path / query helpers ──────────────────────────────────────────────
    [[nodiscard]] std::string last_path_segment(interfaces::io::IRequest& req) const
    {
        const auto segments = ::app::http::RequestPath{}(req);
        if (segments.count() == 0U) {
            return {};
        }
        return std::string{segments.at(segments.count() - 1U).value_or("")};
    }

    [[nodiscard]] QueryPairs query_pairs(interfaces::io::IRequest& req) const
    {
        std::vector<std::pair<std::string, std::string>> pairs{};
        const auto path = std::string{req.get_path()};
        const auto pos = path.find('?');
        if (pos == std::string::npos) {
            return QueryPairs{std::move(pairs)};
        }
        const auto query = std::string_view{path}.substr(pos + 1);
        std::size_t start{0};
        while (start < query.size()) {
            const auto amp = query.find('&', start);
            const auto end = amp == std::string_view::npos ? query.size() : amp;
            const auto pair = query.substr(start, end - start);
            const auto eq = pair.find('=');
            if (eq != std::string_view::npos) {
                pairs.emplace_back(
                    percent_decode(pair.substr(0, eq)),
                    percent_decode(pair.substr(eq + 1))
                );
            }
            if (amp == std::string_view::npos) {
                break;
            }
            start = amp + 1;
        }
        return QueryPairs{std::move(pairs)};
    }

    [[nodiscard]] std::optional<std::int32_t>
    parse_int(const std::optional<std::string>& value) const
    {
        if (!value) {
            return std::nullopt;
        }
        try {
            return static_cast<std::int32_t>(std::stoi(*value));
        } catch (...) {
            return std::nullopt;
        }
    }

    [[nodiscard]] std::optional<bool> parse_bool(const std::optional<std::string>& value) const
    {
        if (!value) {
            return std::nullopt;
        }
        if (*value == "true" || *value == "1") {
            return true;
        }
        if (*value == "false" || *value == "0") {
            return false;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::string>
    parse_datetime(const std::optional<std::string>& value) const
    {
        // Lenient: malformed values are ignored, not rejected (superposition.rs:291-294).
        if (!value || value->empty() || value->find('-') == std::string::npos ||
            value->size() < 10U) {
            return std::nullopt;
        }
        return value;
    }

    [[nodiscard]] ListContextsQuery list_contexts_query(const QueryPairs& pairs) const
    {
        ListContextsQuery result{};
        result.pagination.count = parse_int(pairs.first_value("count"));
        result.pagination.page = parse_int(pairs.first_value("page"));
        result.pagination.all = parse_bool(pairs.first_value("all"));
        result.prefix = pairs.all_values("prefix");
        result.sort_on = pairs.first_value("sort_on");
        result.sort_by = pairs.first_value("sort_by");
        result.created_by = pairs.all_values("created_by");
        result.last_modified_by = pairs.all_values("last_modified_by");
        result.plaintext = pairs.first_value("plaintext");
        result.dimension_match_strategy = pairs.first_value("dimension_match_strategy");
        result.dimension_params = pairs.dimension_params();
        return result;
    }

    [[nodiscard]] ListDefaultConfigsQuery list_default_configs_query(const QueryPairs& pairs) const
    {
        ListDefaultConfigsQuery result{};
        result.pagination.count = parse_int(pairs.first_value("count"));
        result.pagination.page = parse_int(pairs.first_value("page"));
        result.pagination.all = parse_bool(pairs.first_value("all"));
        result.name = pairs.first_value("name");
        return result;
    }

    [[nodiscard]] ListDimensionsQuery list_dimensions_query(const QueryPairs& pairs) const
    {
        ListDimensionsQuery result{};
        result.pagination.count = parse_int(pairs.first_value("count"));
        result.pagination.page = parse_int(pairs.first_value("page"));
        result.pagination.all = parse_bool(pairs.first_value("all"));
        return result;
    }

    [[nodiscard]] ListAuditLogsQuery list_audit_logs_query(const QueryPairs& pairs) const
    {
        ListAuditLogsQuery result{};
        result.pagination.count = parse_int(pairs.first_value("count"));
        result.pagination.page = parse_int(pairs.first_value("page"));
        result.pagination.all = parse_bool(pairs.first_value("all"));
        result.from_date = parse_datetime(pairs.first_value("from_date"));
        result.to_date = parse_datetime(pairs.first_value("to_date"));
        result.table = pairs.csv_values("table");
        result.action = pairs.csv_values("action");
        result.username = pairs.first_value("username");
        result.sort_by = pairs.first_value("sort_by");
        result.dimension_params = pairs.dimension_params();
        return result;
    }

    // ResolveConfigBody.context == serde_json::Map<String, Value> (values verbatim).
    [[nodiscard]] std::vector<std::pair<std::string, Doc>>
    context_entries(const serde::Value& root) const
    {
        std::vector<std::pair<std::string, Doc>> entries{};
        const auto* object = std::get_if<serde::Value::Object>(&root.get());
        if (object == nullptr) {
            return entries;
        }
        const auto* context = find_member(*object, "context");
        if (!context) {
            return entries;
        }
        const auto* context_object = std::get_if<serde::Value::Object>(&context->get());
        if (context_object == nullptr) {
            return entries;
        }
        for (const auto& [key, value]: *context_object) {
            entries.emplace_back(key, value);
        }
        return entries;
    }

    [[nodiscard]] ContextPutRequest context_put_request_from(const serde::Value& root) const
    {
        ContextPutRequest request{};
        const auto* object = std::get_if<serde::Value::Object>(&root.get());
        if (object == nullptr) {
            return request;
        }
        const auto* context = find_member(*object, "context");
        if (context) {
            if (const auto* context_object = std::get_if<serde::Value::Object>(&context->get())) {
                for (const auto& [key, value]: *context_object) {
                    request.context.emplace_back(key, value);
                }
            }
        }
        const auto* override_value = find_member(*object, "override");
        if (override_value) {
            if (const auto* override_object =
                    std::get_if<serde::Value::Object>(&override_value->get())) {
                for (const auto& [key, value]: *override_object) {
                    request.override_values.emplace_back(key, value);
                }
            }
        }
        if (const auto* description = find_member(*object, "description")) {
            if (const auto text = scalar_text(*description)) {
                request.description = *text;
            }
        }
        const auto* change_reason = find_member(*object, "change_reason");
        if (change_reason) {
            if (const auto text = scalar_text(*change_reason)) {
                request.change_reason = *text;
            }
        }
        return request;
    }

    [[nodiscard]] std::optional<std::string> scalar_text(const serde::Value& value) const
    {
        if (const auto* text = std::get_if<std::string>(&value.get())) {
            return *text;
        }
        if (const auto* number = std::get_if<std::int64_t>(&value.get())) {
            return std::to_string(*number);
        }
        if (const auto* flag = std::get_if<bool>(&value.get())) {
            return *flag ? std::string{"true"} : std::string{"false"};
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string_view proxy_config_ns() const noexcept
    {
        return "superposition_proxy";
    }

private:
    [[nodiscard]] std::string percent_decode(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (std::size_t i{0}; i < value.size(); ++i) {
            if (value[i] == '+') {
                out.push_back(' ');
                continue;
            }
            if (value[i] == '%' && i + 2U < value.size()) {
                const int high = hex_digit(value[i + 1U]);
                const int low = hex_digit(value[i + 2U]);
                if (high >= 0 && low >= 0) {
                    out.push_back(static_cast<char>((high << 4) | low));
                    i += 2U;
                    continue;
                }
            }
            out.push_back(value[i]);
        }
        return out;
    }

    [[nodiscard]] int hex_digit(char ch) const noexcept
    {
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        }
        if (ch >= 'a' && ch <= 'f') {
            return ch - 'a' + 10;
        }
        if (ch >= 'A' && ch <= 'F') {
            return ch - 'A' + 10;
        }
        return -1;
    }

    [[nodiscard]] const serde::Value*
    find_member(const serde::Value::Object& object, std::string_view key) const
    {
        for (const auto& [member, value]: object) {
            if (member == key) {
                return &value;
            }
        }
        return nullptr;
    }

    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::superposition_proxy
