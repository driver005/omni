module;
export module app_handlers:superposition_proxy;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import core_cache;
import core_error;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_services;

export namespace app::handlers::superposition_proxy {

/**
 * Strict C++20 port of upstream `routes/superposition_proxy.rs` and
 * `core/superposition_proxy.rs` (SHA 7736d55adaf235f7af74e133ae408573b1826823).
 *
 * Structure mirrors upstream exactly:
 *   - typed query structs with `From<Vec<(String,String)>>` semantics;
 *   - `ScopingDimension` + `validate_scope` / `validate_superposition_params` /
 *     `validate_superposition_context_body`;
 *   - `map_sdk_error` + `map_superposition_err` + `extract_proxy_headers` /
 *     `required_header`;
 *   - per-operation `*InputBuilder`s whose `send_with` dispatches to the local,
 *     in-process Superposition service (the only allowed substitute: the external
 *     Superposition SDK is external infra);
 *   - `*_output_to_struct` / `list_*_to_response` converters that populate every
 *     upstream API-model field;
 *   - `handle_superposition_proxy_flow` -> `ApplicationResponse::Json` equivalent.
 *
 * Wire documents are built with `serde::Value` (rfl::Generic) so field names,
 * ordering and shapes are byte-for-byte the upstream serde shapes (including the
 * `#[serde(transparent)]` resolve map and the `{"error":{...}}` error envelope).
 */
class SuperpositionProxyRouteHandlers
{
public:
    explicit SuperpositionProxyRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    // ── GET /v1/superposition/context ──────────────────────────────────────────
    void handle_list_contexts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "list_contexts");
        if (auth_error) {
            send_api_error(res, send, *auth_error);
            return;
        }
        const auto scope = extract_proxy_headers(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto request = ListContextsQuery::from(query_pairs(req));
        const auto auth = app::auth::AuthContextBuilder{}(req);
        auto result = handle_superposition_proxy_flow(request, auth, scope->org_id, scope->workspace_id);
        finish(res, send, std::move(result), "SuperpositionListContexts", "superposition.list_contexts");
    }

    // ── PUT /v1/superposition/context ──────────────────────────────────────────
    void handle_create_context(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "create_context");
        if (auth_error) {
            send_api_error(res, send, *auth_error);
            return;
        }
        const auto scope = extract_proxy_headers(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_api_error(res, send, bad_request("empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_api_error(res, send, bad_request("invalid JSON"));
            return;
        }
        const auto request = context_put_request_from(*parsed);
        const auto auth = app::auth::AuthContextBuilder{}(req);
        auto result = handle_superposition_proxy_flow(request, auth, scope->org_id, scope->workspace_id);
        finish(res, send, std::move(result), "SuperpositionCreateContext", "superposition.create_context");
    }

    // ── GET /v1/superposition/default-config ───────────────────────────────────
    void handle_list_default_configs(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "list_default_configs");
        if (auth_error) {
            send_api_error(res, send, *auth_error);
            return;
        }
        const auto scope = extract_proxy_headers(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto request = ListDefaultConfigsQuery::from(query_pairs(req));
        const auto auth = app::auth::AuthContextBuilder{}(req);
        auto result = handle_superposition_proxy_flow(request, auth, scope->org_id, scope->workspace_id);
        finish(res, send, std::move(result), "SuperpositionListDefaultConfigs", "superposition.list_default_configs");
    }

    // ── GET /v1/superposition/default-config/{key} ──────────────────────────────
    void handle_get_default_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "get_default_config");
        if (auth_error) {
            send_api_error(res, send, *auth_error);
            return;
        }
        const auto scope = extract_proxy_headers(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto request = GetDefaultConfigRequest{last_path_segment(req)};
        const auto auth = app::auth::AuthContextBuilder{}(req);
        auto result = handle_superposition_proxy_flow(request, auth, scope->org_id, scope->workspace_id);
        finish(res, send, std::move(result), "SuperpositionGetDefaultConfig", "superposition.get_default_config");
    }

    // ── GET /v1/superposition/dimension ─────────────────────────────────────────
    void handle_list_dimensions(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "list_dimensions");
        if (auth_error) {
            send_api_error(res, send, *auth_error);
            return;
        }
        const auto scope = extract_proxy_headers(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto request = ListDimensionsQuery::from(query_pairs(req));
        const auto auth = app::auth::AuthContextBuilder{}(req);
        auto result = handle_superposition_proxy_flow(request, auth, scope->org_id, scope->workspace_id);
        finish(res, send, std::move(result), "SuperpositionListDimensions", "superposition.list_dimensions");
    }

    // ── GET /v1/superposition/dimension/{dimension_name} ────────────────────────
    void handle_get_dimension(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "get_dimension");
        if (auth_error) {
            send_api_error(res, send, *auth_error);
            return;
        }
        const auto scope = extract_proxy_headers(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto request = GetDimensionRequest{last_path_segment(req)};
        const auto auth = app::auth::AuthContextBuilder{}(req);
        auto result = handle_superposition_proxy_flow(request, auth, scope->org_id, scope->workspace_id);
        finish(res, send, std::move(result), "SuperpositionGetDimension", "superposition.get_dimension");
    }

    // ── POST /v1/superposition/config/resolve/detailed ──────────────────────────
    void handle_resolve_detailed_config(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "resolve_detailed_config");
        if (auth_error) {
            send_api_error(res, send, *auth_error);
            return;
        }
        const auto scope = extract_proxy_headers(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_api_error(res, send, bad_request("empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_api_error(res, send, bad_request("invalid JSON"));
            return;
        }
        const auto request = ResolveDetailedConfigRequest{context_entries(*parsed)};
        const auto auth = app::auth::AuthContextBuilder{}(req);
        auto result = handle_superposition_proxy_flow(request, auth, scope->org_id, scope->workspace_id);
        finish(res, send, std::move(result), "SuperpositionResolveDetailedConfig", "superposition.resolve_detailed_config");
    }

    // ── POST /v1/superposition/config/resolve/explain/{key} ─────────────────────
    void handle_resolve_config_explanation(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "resolve_config_explanation");
        if (auth_error) {
            send_api_error(res, send, *auth_error);
            return;
        }
        const auto scope = extract_proxy_headers(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto key = last_path_segment(req);
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_api_error(res, send, bad_request("empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_api_error(res, send, bad_request("invalid JSON"));
            return;
        }
        const auto request = ResolveConfigExplanationRequest{key, context_entries(*parsed)};
        const auto auth = app::auth::AuthContextBuilder{}(req);
        auto result = handle_superposition_proxy_flow(request, auth, scope->org_id, scope->workspace_id);
        finish(res, send, std::move(result), "SuperpositionResolveConfigExplanation", "superposition.resolve_config_explanation");
    }

    // ── GET /v1/superposition/audit ─────────────────────────────────────────────
    void handle_list_audit_logs(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "list_audit_logs");
        if (auth_error) {
            send_api_error(res, send, *auth_error);
            return;
        }
        const auto scope = extract_proxy_headers(req, res, send);
        if (!scope) {
            return;
        }
        cache_scope(req, *scope);

        const auto request = ListAuditLogsQuery::from(query_pairs(req));
        const auto auth = app::auth::AuthContextBuilder{}(req);
        auto result = handle_superposition_proxy_flow(request, auth, scope->org_id, scope->workspace_id);
        finish(res, send, std::move(result), "SuperpositionListAuditLogs", "superposition.list_audit_logs");
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/v1/superposition"}
            .add_route(
                Route{"/context"}
                    .get([this](
                             interfaces::io::IRequest& req,
                             interfaces::io::IResponse& res,
                             std::function<void()> send
                         ) {
                             handle_list_contexts(req, res, std::move(send));
                         })
                    .summary("Hyperswitch list_contexts")
            )
            .add_route(
                Route{"/context"}
                    .put([this](
                             interfaces::io::IRequest& req,
                             interfaces::io::IResponse& res,
                             std::function<void()> send
                         ) {
                             handle_create_context(req, res, std::move(send));
                         })
                    .summary("Hyperswitch create_context")
            )
            .add_route(
                Route{"/default-config"}
                    .get([this](
                             interfaces::io::IRequest& req,
                             interfaces::io::IResponse& res,
                             std::function<void()> send
                         ) {
                             handle_list_default_configs(req, res, std::move(send));
                         })
                    .summary("Hyperswitch list_default_configs")
            )
            .add_route(
                Route{"/default-config/:key"}
                    .get([this](
                             interfaces::io::IRequest& req,
                             interfaces::io::IResponse& res,
                             std::function<void()> send
                         ) {
                             handle_get_default_config(req, res, std::move(send));
                         })
                    .summary("Hyperswitch get_default_config")
            )
            .add_route(
                Route{"/dimension"}
                    .get([this](
                             interfaces::io::IRequest& req,
                             interfaces::io::IResponse& res,
                             std::function<void()> send
                         ) {
                             handle_list_dimensions(req, res, std::move(send));
                         })
                    .summary("Hyperswitch list_dimensions")
            )
            .add_route(
                Route{"/dimension/:dimension_name"}
                    .get([this](
                             interfaces::io::IRequest& req,
                             interfaces::io::IResponse& res,
                             std::function<void()> send
                         ) {
                             handle_get_dimension(req, res, std::move(send));
                         })
                    .summary("Hyperswitch get_dimension")
            )
            .add_route(
                Route{"/config/resolve/detailed"}
                    .post([this](
                              interfaces::io::IRequest& req,
                              interfaces::io::IResponse& res,
                              std::function<void()> send
                          ) {
                              handle_resolve_detailed_config(req, res, std::move(send));
                          })
                    .summary("Hyperswitch resolve_detailed_config")
            )
            .add_route(
                Route{"/config/resolve/explain/:key"}
                    .post([this](
                              interfaces::io::IRequest& req,
                              interfaces::io::IResponse& res,
                              std::function<void()> send
                          ) {
                              handle_resolve_config_explanation(req, res, std::move(send));
                          })
                    .summary("Hyperswitch resolve_config_explanation")
            )
            .add_route(
                Route{"/audit"}
                    .get([this](
                             interfaces::io::IRequest& req,
                             interfaces::io::IResponse& res,
                             std::function<void()> send
                         ) {
                             handle_list_audit_logs(req, res, std::move(send));
                         })
                    .summary("Hyperswitch list_audit_logs")
            );
    }

private:
    // ═════════════════════════════════════════════════════════════════════════════
    // Local types — in-process stand-ins for the external superposition SDK types
    // (smithy `Document` == serde::Value; doc maps == std::map<std::string, Value>).
    // ═════════════════════════════════════════════════════════════════════════════
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

    [[nodiscard]] static ApiError invalid_request_data(std::string message)
    {
        return {ApiError::Kind::InvalidRequestData, std::move(message), {}};
    }

    [[nodiscard]] static ApiError access_forbidden(std::string resource)
    {
        return {ApiError::Kind::AccessForbidden, {}, std::move(resource)};
    }

    [[nodiscard]] static ApiError internal_server_error()
    {
        return {ApiError::Kind::InternalServerError, "Something went wrong", {}};
    }

    [[nodiscard]] static ApiError bad_request(std::string message)
    {
        return {ApiError::Kind::BadRequestBody, std::move(message), {}};
    }

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
        QueryPairs(std::vector<std::pair<std::string, std::string>> v) : values(std::move(v)) {}

        [[nodiscard]] std::optional<std::string> first_value(std::string_view name) const
        {
            for (const auto& [key, value] : values) {
                if (key == name) {
                    return value;
                }
            }
            return std::nullopt;
        }

        [[nodiscard]] std::vector<std::string> all_values(std::string_view name) const
        {
            std::vector<std::string> result{};
            for (const auto& [key, value] : values) {
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
            for (const auto& [key, value] : values) {
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
            for (const auto& pair : values) {
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

        static ListContextsQuery from(const QueryPairs& pairs)
        {
            ListContextsQuery result{};
            result.pagination.count = SuperpositionProxyRouteHandlers::parse_int(pairs.first_value("count"));
            result.pagination.page = SuperpositionProxyRouteHandlers::parse_int(pairs.first_value("page"));
            result.pagination.all = SuperpositionProxyRouteHandlers::parse_bool(pairs.first_value("all"));
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
    };

    struct ListDefaultConfigsQuery
    {
        Pagination pagination{};
        std::optional<std::string> name{};

        static ListDefaultConfigsQuery from(const QueryPairs& pairs)
        {
            ListDefaultConfigsQuery result{};
            result.pagination.count = SuperpositionProxyRouteHandlers::parse_int(pairs.first_value("count"));
            result.pagination.page = SuperpositionProxyRouteHandlers::parse_int(pairs.first_value("page"));
            result.pagination.all = SuperpositionProxyRouteHandlers::parse_bool(pairs.first_value("all"));
            result.name = pairs.first_value("name");
            return result;
        }
    };

    struct ListDimensionsQuery
    {
        Pagination pagination{};

        static ListDimensionsQuery from(const QueryPairs& pairs)
        {
            ListDimensionsQuery result{};
            result.pagination.count = SuperpositionProxyRouteHandlers::parse_int(pairs.first_value("count"));
            result.pagination.page = SuperpositionProxyRouteHandlers::parse_int(pairs.first_value("page"));
            result.pagination.all = SuperpositionProxyRouteHandlers::parse_bool(pairs.first_value("all"));
            return result;
        }
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

        static ListAuditLogsQuery from(const QueryPairs& pairs)
        {
            ListAuditLogsQuery result{};
            result.pagination.count = SuperpositionProxyRouteHandlers::parse_int(pairs.first_value("count"));
            result.pagination.page = SuperpositionProxyRouteHandlers::parse_int(pairs.first_value("page"));
            result.pagination.all = SuperpositionProxyRouteHandlers::parse_bool(pairs.first_value("all"));
            result.from_date = SuperpositionProxyRouteHandlers::parse_datetime(pairs.first_value("from_date"));
            result.to_date = SuperpositionProxyRouteHandlers::parse_datetime(pairs.first_value("to_date"));
            result.table = pairs.csv_values("table");
            result.action = pairs.csv_values("action");
            result.username = pairs.first_value("username");
            result.sort_by = pairs.first_value("sort_by");
            result.dimension_params = pairs.dimension_params();
            return result;
        }
    };

    struct GetDimensionRequest
    {
        std::string dimension{};
    };

    struct GetDefaultConfigRequest
    {
        std::string key{};
    };

    // Upstream ResolveConfigBody.context: Map<String, Value> (preserve order for BTreeMap-like emission).
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

    // ═════════════════════════════════════════════════════════════════════════════
    // Upstream: ScopingDimension + validate_scope (core/superposition_proxy.rs:27-133)
    // ═════════════════════════════════════════════════════════════════════════════
    enum class ScopingDimension : std::uint8_t
    {
        OrganizationId,
        MerchantId,
        ProfileId,
        ProviderMerchantId,
        ProcessorMerchantId,
    };

    static constexpr std::array<ScopingDimension, 4> kRequiredDimensions{
        ScopingDimension::OrganizationId,
        ScopingDimension::ProfileId,
        ScopingDimension::ProcessorMerchantId,
        ScopingDimension::ProviderMerchantId,
    };

    [[nodiscard]] static std::optional<ScopingDimension> from_context_key(std::string_view key) noexcept
    {
        if (key == "organization_id") {
            return ScopingDimension::OrganizationId;
        }
        if (key == "merchant_id") {
            return ScopingDimension::MerchantId;
        }
        if (key == "profile_id") {
            return ScopingDimension::ProfileId;
        }
        if (key == "provider_merchant_id") {
            return ScopingDimension::ProviderMerchantId;
        }
        if (key == "processor_merchant_id") {
            return ScopingDimension::ProcessorMerchantId;
        }
        return std::nullopt;
    }

    // strip_prefix("dimension[") + strip_suffix(']') + from_context_key
    [[nodiscard]] static std::optional<ScopingDimension> from_dimension_param(std::string_view key) noexcept
    {
        if (!key.starts_with("dimension[") || !key.ends_with(']') || key.size() <= 11U) {
            return std::nullopt;
        }
        return from_context_key(key.substr(10U, key.size() - 11U));
    }

    [[nodiscard]] static std::string_view dimension_name(ScopingDimension dimension) noexcept
    {
        switch (dimension) {
        case ScopingDimension::OrganizationId: return "organization_id";
        case ScopingDimension::MerchantId: return "merchant_id";
        case ScopingDimension::ProfileId: return "profile_id";
        case ScopingDimension::ProviderMerchantId: return "provider_merchant_id";
        case ScopingDimension::ProcessorMerchantId: return "processor_merchant_id";
        }
        return {};
    }

    [[nodiscard]] static std::string_view expected_value(
        ScopingDimension dimension,
        const app::auth::AuthContext& auth
    ) noexcept
    {
        switch (dimension) {
        case ScopingDimension::OrganizationId: return auth.org_id();
        case ScopingDimension::ProfileId: return auth.profile_id();
        case ScopingDimension::MerchantId:
        case ScopingDimension::ProviderMerchantId:
        case ScopingDimension::ProcessorMerchantId: return auth.merchant_id();
        }
        return {};
    }

    // validate_scope(dimensions, auth): REQUIRED presence + value==expected.
    [[nodiscard]] std::optional<ApiError> validate_scope(
        const std::vector<std::pair<ScopingDimension, std::optional<std::string_view>>>& dimensions,
        const app::auth::AuthContext& auth
    ) const
    {
        std::vector<std::string> missing{};
        for (const auto required : kRequiredDimensions) {
            const bool present = std::ranges::any_of(
                dimensions,
                [required](const auto& entry) {
                    return entry.first == required;
                }
            );
            if (!present) {
                missing.emplace_back(dimension_name(required));
            }
        }
        if (!missing.empty()) {
            return invalid_request_data("scope is missing: " + join(missing, ", "));
        }

        const bool out_of_scope = std::ranges::any_of(
            dimensions,
            [&auth](const auto& entry) {
                return entry.second != std::optional<std::string_view>{expected_value(entry.first, auth)};
            }
        );
        if (out_of_scope) {
            return access_forbidden("superposition");
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<ApiError> validate_superposition_params(
        const std::vector<std::pair<std::string, std::string>>& params,
        const app::auth::AuthContext& auth
    ) const
    {
        std::vector<std::pair<ScopingDimension, std::optional<std::string_view>>> dimensions{};
        for (const auto& [key, value] : params) {
            if (const auto dimension = from_dimension_param(key)) {
                dimensions.emplace_back(*dimension, std::optional<std::string_view>{value});
            }
        }
        return validate_scope(dimensions, auth);
    }

    [[nodiscard]] std::optional<ApiError> validate_superposition_context_body(
        const std::vector<std::pair<std::string, Doc>>& context,
        const app::auth::AuthContext& auth
    ) const
    {
        std::vector<std::pair<ScopingDimension, std::optional<std::string_view>>> dimensions{};
        for (const auto& [key, value] : context) {
            const auto dimension = from_context_key(key);
            if (!dimension) {
                continue;
            }
            // value.as_str(): a non-string JSON value yields None -> out of scope.
            if (const auto* text = std::get_if<std::string>(&value.get())) {
                dimensions.emplace_back(*dimension, std::optional<std::string_view>{*text});
            } else {
                dimensions.emplace_back(*dimension, std::nullopt);
            }
        }
        return validate_scope(dimensions, auth);
    }

    // ═════════════════════════════════════════════════════════════════════════════
    // Upstream: error mapping (core:135-192)
    // ═════════════════════════════════════════════════════════════════════════════
    struct SdkError
    {
        enum class Kind : std::uint8_t
        {
            BadRequest,
            NotFound,
            ClientError,
        };
        Kind kind{Kind::ClientError};
        std::string message{};
    };

    // map_sdk_error: HTTP status -> SuperpositionError variant (external_services:317-326).
    [[nodiscard]] static SdkError map_sdk_error(int status, std::string message)
    {
        if (status == 404) {
            return {SdkError::Kind::NotFound, std::move(message)};
        }
        if (status >= 400 && status < 500) {
            return {SdkError::Kind::BadRequest, std::move(message)};
        }
        return {SdkError::Kind::ClientError, std::move(message)};
    }

    // map_superposition_err: SuperpositionError -> ApiErrorResponse (core:135-155).
    [[nodiscard]] static ApiError map_superposition_err(const SdkError& error, std::string_view /*context*/)
    {
        switch (error.kind) {
        case SdkError::Kind::BadRequest:
            return invalid_request_data(error.message);
        case SdkError::Kind::NotFound:
            return invalid_request_data("The specified org_id or workspace_id was not found in Superposition");
        case SdkError::Kind::ClientError:
            return internal_server_error();
        }
        return internal_server_error();
    }

    // extract_proxy_headers + required_header (core:157-192).
    [[nodiscard]] std::optional<ProxyScope> extract_proxy_headers(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()>& send
    ) const
    {
        const auto org_id = app::http::HeaderReader{}.get(req, "x-org-id");
        if (org_id.empty()) {
            send_header_error(res, send, "missing required header: x-org-id");
            return std::nullopt;
        }
        const auto workspace_id = app::http::HeaderReader{}.get(req, "x-workspace");
        if (workspace_id.empty()) {
            send_header_error(res, send, "missing required header: x-workspace");
            return std::nullopt;
        }
        // Config is the only trusted source for the Superposition org/workspace.
        const auto configured_org = m_context->config_store().get_value(proxy_config_ns(), "configured_org_id");
        const auto configured_workspace = m_context->config_store().get_value(proxy_config_ns(), "configured_workspace_id");
        if ((configured_org && *configured_org != org_id) ||
            (configured_workspace && *configured_workspace != workspace_id)) {
            send_api_error(res, send, access_forbidden("superposition org and workspace"));
            return std::nullopt;
        }
        return ProxyScope{org_id, workspace_id};
    }

    // ═════════════════════════════════════════════════════════════════════════════
    // Upstream: handle_superposition_proxy_flow + ApplicationResponse::Json (core:344-355)
    // ═════════════════════════════════════════════════════════════════════════════
    template<typename Request>
    [[nodiscard]] std::expected<serde::Value, ApiError> handle_superposition_proxy_flow(
        const Request& request,
        const app::auth::AuthContext& auth,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        // request.execute(state, auth, org, workspace) -> ApplicationResponse::Json(response)
        return execute(request, auth, org_id, workspace_id);
    }

    // ── execute: ListContextsQuery (core:390-411) ───────────────────────────────
    [[nodiscard]] std::expected<serde::Value, ApiError> execute(
        const ListContextsQuery& request,
        const app::auth::AuthContext& auth,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        if (const auto error = validate_superposition_params(request.dimension_params, auth)) {
            return std::unexpected{*error};
        }
        auto output = send_with(into_input(request, org_id, workspace_id));
        if (!output) {
            return std::unexpected{map_superposition_err(output.error(), "Failed to list contexts from Superposition")};
        }
        return wire_paginated(list_contexts_to_response(*output));
    }

    // ── execute: ListDefaultConfigsQuery (core:433-452) ─────────────────────────
    [[nodiscard]] std::expected<serde::Value, ApiError> execute(
        const ListDefaultConfigsQuery& request,
        const app::auth::AuthContext&,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        auto output = send_with(into_input(request, org_id, workspace_id));
        if (!output) {
            return std::unexpected{map_superposition_err(output.error(), "Failed to list default configs from Superposition")};
        }
        return wire_paginated(list_default_configs_to_response(*output));
    }

    // ── execute: ListDimensionsQuery (core:473-492) ─────────────────────────────
    [[nodiscard]] std::expected<serde::Value, ApiError> execute(
        const ListDimensionsQuery& request,
        const app::auth::AuthContext&,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        auto output = send_with(into_input(request, org_id, workspace_id));
        if (!output) {
            return std::unexpected{map_superposition_err(output.error(), "Failed to list dimensions from Superposition")};
        }
        return wire_paginated(list_dimensions_to_response(*output));
    }

    // ── execute: GetDimensionRequest (core:515-534) ─────────────────────────────
    [[nodiscard]] std::expected<serde::Value, ApiError> execute(
        const GetDimensionRequest& request,
        const app::auth::AuthContext&,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        auto output = send_with(into_input(request, org_id, workspace_id));
        if (!output) {
            return std::unexpected{map_superposition_err(output.error(), "Failed to get dimension from Superposition")};
        }
        return wire_of(get_dimension_output_to_struct(*output));
    }

    // ── execute: GetDefaultConfigRequest (core:557-576) ─────────────────────────
    [[nodiscard]] std::expected<serde::Value, ApiError> execute(
        const GetDefaultConfigRequest& request,
        const app::auth::AuthContext&,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        auto output = send_with(into_input(request, org_id, workspace_id));
        if (!output) {
            return std::unexpected{map_superposition_err(output.error(), "Failed to get default config from Superposition")};
        }
        return wire_of(get_default_config_output_to_struct(*output));
    }

    // ── execute: ContextPutRequest (core:598-626) ───────────────────────────────
    [[nodiscard]] std::expected<serde::Value, ApiError> execute(
        const ContextPutRequest& request,
        const app::auth::AuthContext& auth,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        auto input = into_input(request, org_id, workspace_id);
        if (!input) {
            return std::unexpected{map_superposition_err(input.error(), "Invalid context request body")};
        }
        // doc_map_to_json(context_put.context()) -> validate context body.
        std::vector<std::pair<std::string, Doc>> context_docs{};
        for (const auto& [key, value] : input->request.context) {
            context_docs.emplace_back(key, value);
        }
        if (const auto error = validate_superposition_context_body(context_docs, auth)) {
            return std::unexpected{*error};
        }
        auto output = send_with(*input);
        if (!output) {
            return std::unexpected{map_superposition_err(output.error(), "Failed to create context in Superposition")};
        }
        return wire_of(create_context_output_to_struct(*output));
    }

    // ── execute: ResolveDetailedConfigRequest (core:654-684) ────────────────────
    [[nodiscard]] std::expected<serde::Value, ApiError> execute(
        const ResolveDetailedConfigRequest& request,
        const app::auth::AuthContext& auth,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        if (const auto error = validate_superposition_context_body(request.context, auth)) {
            return std::unexpected{*error};
        }
        auto output = send_with(into_input(request, org_id, workspace_id));
        if (!output) {
            return std::unexpected{map_superposition_err(output.error(), "Failed to resolve config from Superposition")};
        }
        // document_to_value(output.config()) + serde_json::from_value with error mapping.
        auto decoded = serde::Ser::decode_generic("application/json", serde::Ser::encode_json(output->config));
        if (!decoded) {
            const auto sdk_error = map_sdk_error(0, "ClientError");
            return std::unexpected{map_superposition_err(sdk_error, "Failed to parse resolved config from Superposition")};
        }
        return wire_resolve(*decoded);
    }

    // ── execute: ResolveConfigExplanationRequest (core:716-738) ─────────────────
    [[nodiscard]] std::expected<serde::Value, ApiError> execute(
        const ResolveConfigExplanationRequest& request,
        const app::auth::AuthContext& auth,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        if (const auto error = validate_superposition_context_body(request.context, auth)) {
            return std::unexpected{*error};
        }
        auto output = send_with(into_input(request, org_id, workspace_id));
        if (!output) {
            return std::unexpected{map_superposition_err(output.error(), "Failed to resolve config explanation from Superposition")};
        }
        return wire_of(resolve_config_explanation_to_response(*output));
    }

    // ── execute: ListAuditLogsQuery (core:778-799) ──────────────────────────────
    [[nodiscard]] std::expected<serde::Value, ApiError> execute(
        const ListAuditLogsQuery& request,
        const app::auth::AuthContext& auth,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        if (const auto error = validate_superposition_params(request.dimension_params, auth)) {
            return std::unexpected{*error};
        }
        auto output = send_with(into_input(request, org_id, workspace_id));
        if (!output) {
            return std::unexpected{map_superposition_err(output.error(), "Failed to list audit logs from Superposition")};
        }
        return wire_paginated(list_audit_logs_to_response(*output));
    }

    // ═════════════════════════════════════════════════════════════════════════════
    // Input builders (external_services SDK builders) — exact field wiring.
    // ═════════════════════════════════════════════════════════════════════════════
    struct ListContextsInput
    {
        std::string org_id{};
        std::string workspace_id{};
        Pagination pagination{};
        std::vector<std::string> prefix{};
        std::optional<std::string> sort_on{};
        std::optional<std::string> sort_by{};
        std::optional<std::string> plaintext{};
        std::optional<std::string> dimension_match_strategy{};
        std::vector<std::string> created_by{};
        std::vector<std::string> last_modified_by{};
        std::vector<std::pair<std::string, std::string>> dimension_params{};
    };

    struct ListDefaultConfigsInput
    {
        std::string org_id{};
        std::string workspace_id{};
        Pagination pagination{};
        std::optional<std::string> name{};
    };

    struct ListDimensionsInput
    {
        std::string org_id{};
        std::string workspace_id{};
        Pagination pagination{};
    };

    struct GetDimensionInput
    {
        std::string org_id{};
        std::string workspace_id{};
        std::string dimension{};
    };

    struct GetDefaultConfigInput
    {
        std::string org_id{};
        std::string workspace_id{};
        std::string key{};
    };

    struct CreateContextInput
    {
        std::string org_id{};
        std::string workspace_id{};
        ContextPutRequest request{};
    };

    struct GetDetailedResolvedConfigInput
    {
        std::string org_id{};
        std::string workspace_id{};
        std::vector<std::pair<std::string, Doc>> context{};
    };

    struct GetResolvedConfigExplanationInput
    {
        std::string org_id{};
        std::string workspace_id{};
        std::string key{};
        std::vector<std::pair<std::string, Doc>> context{};
    };

    struct ListAuditLogsInput
    {
        std::string org_id{};
        std::string workspace_id{};
        Pagination pagination{};
        std::optional<std::string> from_date{};
        std::optional<std::string> to_date{};
        std::optional<std::string> username{};
        std::optional<std::string> sort_by{};
        std::vector<std::string> tables{};
        std::vector<std::string> action{};
        std::vector<std::pair<std::string, std::string>> dimension_params{};
    };

    [[nodiscard]] static ListContextsInput into_input(
        const ListContextsQuery& request,
        std::string_view org_id,
        std::string_view workspace_id
    )
    {
        ListContextsInput input{};
        input.org_id = org_id;
        input.workspace_id = workspace_id;
        input.pagination = request.pagination;
        input.prefix = request.prefix;
        input.sort_on = request.sort_on;
        input.sort_by = request.sort_by;
        input.plaintext = request.plaintext;
        input.dimension_match_strategy = request.dimension_match_strategy;
        input.created_by = request.created_by;
        input.last_modified_by = request.last_modified_by;
        input.dimension_params = request.dimension_params;
        return input;
    }

    [[nodiscard]] static ListDefaultConfigsInput into_input(
        const ListDefaultConfigsQuery& request,
        std::string_view org_id,
        std::string_view workspace_id
    )
    {
        return {std::string{org_id}, std::string{workspace_id}, request.pagination, request.name};
    }

    [[nodiscard]] static ListDimensionsInput into_input(
        const ListDimensionsQuery& request,
        std::string_view org_id,
        std::string_view workspace_id
    )
    {
        return {std::string{org_id}, std::string{workspace_id}, request.pagination};
    }

    [[nodiscard]] static GetDimensionInput into_input(
        const GetDimensionRequest& request,
        std::string_view org_id,
        std::string_view workspace_id
    )
    {
        return {std::string{org_id}, std::string{workspace_id}, request.dimension};
    }

    [[nodiscard]] static GetDefaultConfigInput into_input(
        const GetDefaultConfigRequest& request,
        std::string_view org_id,
        std::string_view workspace_id
    )
    {
        return {std::string{org_id}, std::string{workspace_id}, request.key};
    }

    [[nodiscard]] std::expected<CreateContextInput, SdkError> into_input(
        const ContextPutRequest& request,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        // context_put_from_request(&self) maps errors to ClientError (core:589-590).
        auto context_put = context_put_from_request(request);
        if (!context_put) {
            return std::unexpected{SdkError{SdkError::Kind::ClientError, "Failed to build ContextPut"}};
        }
        return CreateContextInput{std::string{org_id}, std::string{workspace_id}, std::move(*context_put)};
    }

    [[nodiscard]] static GetDetailedResolvedConfigInput into_input(
        const ResolveDetailedConfigRequest& request,
        std::string_view org_id,
        std::string_view workspace_id
    )
    {
        return {std::string{org_id}, std::string{workspace_id}, request.context};
    }

    [[nodiscard]] static GetResolvedConfigExplanationInput into_input(
        const ResolveConfigExplanationRequest& request,
        std::string_view org_id,
        std::string_view workspace_id
    )
    {
        return {std::string{org_id}, std::string{workspace_id}, request.key, request.context};
    }

    [[nodiscard]] static ListAuditLogsInput into_input(
        const ListAuditLogsQuery& request,
        std::string_view org_id,
        std::string_view workspace_id
    )
    {
        ListAuditLogsInput input{};
        input.org_id = org_id;
        input.workspace_id = workspace_id;
        input.pagination = request.pagination;
        input.from_date = request.from_date;
        input.to_date = request.to_date;
        input.username = request.username;
        input.sort_by = request.sort_by;
        input.tables = request.table;
        input.action = request.action;
        input.dimension_params = request.dimension_params;
        return input;
    }

    // ═════════════════════════════════════════════════════════════════════════════
    // Local SDK outputs + `send_with` (in-process substitute for external infra).
    // ═════════════════════════════════════════════════════════════════════════════
    struct SdkContextResponse
    {
        std::string id{};
        DocMap value{};
        DocMap override_values{};
        std::string override_id{};
        std::string weight{};
        std::string description{};
        std::string change_reason{};
        std::string created_at{};
        std::string created_by{};
        std::string last_modified_at{};
        std::string last_modified_by{};
    };

    struct ListContextsOutput
    {
        std::int32_t total_pages{};
        std::int32_t total_items{};
        std::vector<SdkContextResponse> data{};
    };

    struct SdkDefaultConfigResponse
    {
        std::string key{};
        Doc value{};
        DocMap schema{};
        std::string description{};
        std::string change_reason{};
        std::optional<std::string> value_validation_function_name{};
        std::optional<std::string> value_compute_function_name{};
        std::string created_at{};
        std::string created_by{};
        std::string last_modified_at{};
        std::string last_modified_by{};
    };

    struct ListDefaultConfigsOutput
    {
        std::int32_t total_pages{};
        std::int32_t total_items{};
        std::vector<SdkDefaultConfigResponse> data{};
    };

    struct SdkDimensionResponse
    {
        std::string dimension{};
        std::int32_t position{};
        DocMap schema{};
        std::optional<std::string> value_validation_function_name{};
        std::string description{};
        std::string change_reason{};
        std::string last_modified_at{};
        std::string last_modified_by{};
        std::string created_at{};
        std::string created_by{};
        std::map<std::string, std::vector<std::string>> dependency_graph{};
        std::string dimension_type{};
        std::optional<std::string> value_compute_function_name{};
        bool mandatory{};
    };

    struct ListDimensionsOutput
    {
        std::int32_t total_pages{};
        std::int32_t total_items{};
        std::vector<SdkDimensionResponse> data{};
    };

    struct GetDimensionOutput
    {
        SdkDimensionResponse dimension{};
    };

    struct GetDefaultConfigOutput
    {
        SdkDefaultConfigResponse config{};
    };

    struct SdkAuditLogFull
    {
        std::string id{};
        std::string table_name{};
        std::string user_name{};
        std::string timestamp{};
        std::string action{};
        std::optional<Doc> original_data{};
        std::optional<Doc> new_data{};
        std::string query{};
    };

    struct ListAuditLogsOutput
    {
        std::int32_t total_pages{};
        std::int32_t total_items{};
        std::vector<SdkAuditLogFull> data{};
    };

    struct CreateContextOutput
    {
        SdkContextResponse context{};
    };

    struct GetDetailedResolvedConfigOutput
    {
        Doc config{};
    };

    struct ResolveExplanationEntryOut
    {
        std::string context_id{};
        DocMap condition{};
        std::string override_id{};
        Doc value_before{};
        Doc value_after{};
    };

    struct GetResolvedConfigExplanationOutput
    {
        std::string key{};
        std::vector<ResolveExplanationEntryOut> timeline{};
    };

    // ── send_with: ListContextsInput ────────────────────────────────────────────
    [[nodiscard]] std::expected<ListContextsOutput, SdkError> send_with(const ListContextsInput& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        auto records = m_context->superposition().list_contexts();

        // dimension_params (post from_dimension_param validation): filter by scoped value.
        const bool prefix_match = input.dimension_match_strategy && *input.dimension_match_strategy == "prefix";
        if (!input.dimension_params.empty()) {
            std::vector<app::services::ContextRecord> scoped{};
            for (const auto& record : records) {
                bool keep{true};
                for (const auto& [key, value] : input.dimension_params) {
                    if (!from_dimension_param(key)) {
                        continue;
                    }
                    const bool matches = prefix_match
                                             ? record.get_name().starts_with(value)
                                             : record.get_name() == value;
                    if (!matches) {
                        keep = false;
                        break;
                    }
                }
                if (keep) {
                    scoped.push_back(record);
                }
            }
            records = std::move(scoped);
        }
        if (!input.prefix.empty()) {
            std::vector<app::services::ContextRecord> prefixed{};
            for (const auto& record : records) {
                if (matches_any(input.prefix, record.get_name(), true)) {
                    prefixed.push_back(record);
                }
            }
            records = std::move(prefixed);
        }
        if (!input.created_by.empty() || !input.last_modified_by.empty()) {
            std::vector<app::services::ContextRecord> owned{};
            for (const auto& record : records) {
                const auto creator = context_meta(record.get_name(), "created_by");
                const auto modifier = context_meta(record.get_name(), "last_modified_by");
                if (!input.created_by.empty() &&
                    (!creator || !matches_any(input.created_by, *creator, false))) {
                    continue;
                }
                if (!input.last_modified_by.empty() &&
                    (!modifier || !matches_any(input.last_modified_by, *modifier, false))) {
                    continue;
                }
                owned.push_back(record);
            }
            records = std::move(owned);
        }
        if (input.plaintext) {
            std::vector<app::services::ContextRecord> plain{};
            for (const auto& record : records) {
                if (contains_ci(record.get_name(), *input.plaintext) ||
                    contains_ci(record.get_value(), *input.plaintext)) {
                    plain.push_back(record);
                }
            }
            records = std::move(plain);
        }
        sort_contexts(records, input.sort_on.value_or(""), input.sort_by.value_or(""));

        ListContextsOutput output{};
        output.total_items = static_cast<std::int32_t>(records.size());
        output.total_pages = total_pages(records.size(), input.pagination);
        const auto [begin, end] = slice_bounds(records.size(), input.pagination);
        for (std::size_t index{begin}; index < end; ++index) {
            output.data.push_back(context_from_record(records[index]));
        }
        return output;
    }

    // ── send_with: ListDefaultConfigsInput ──────────────────────────────────────
    [[nodiscard]] std::expected<ListDefaultConfigsOutput, SdkError> send_with(const ListDefaultConfigsInput& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        std::vector<app::services::ConfigRecord> records{};
        if (input.name && !input.name->empty()) {
            if (const auto record = m_context->superposition().get_config(default_config_ns(), *input.name)) {
                records.push_back(*record);
            }
        } else {
            for (const auto& entry : m_context->superposition().audit_logs()) {
                if (entry.get_config_namespace() != default_config_ns() || entry.get_action() != "put") {
                    continue;
                }
                if (const auto record = m_context->superposition().get_config(
                        default_config_ns(), entry.get_config_key()
                    )) {
                    records.push_back(*record);
                }
            }
        }
        ListDefaultConfigsOutput output{};
        output.total_items = static_cast<std::int32_t>(records.size());
        output.total_pages = total_pages(records.size(), input.pagination);
        const auto [begin, end] = slice_bounds(records.size(), input.pagination);
        for (std::size_t index{begin}; index < end; ++index) {
            output.data.push_back(default_config_from_record(records[index]));
        }
        return output;
    }

    // ── send_with: ListDimensionsInput ──────────────────────────────────────────
    [[nodiscard]] std::expected<ListDimensionsOutput, SdkError> send_with(const ListDimensionsInput& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        const auto records = m_context->config_store().list(dimension_ns());
        ListDimensionsOutput output{};
        output.total_items = static_cast<std::int32_t>(records.size());
        output.total_pages = total_pages(records.size(), input.pagination);
        const auto [begin, end] = slice_bounds(records.size(), input.pagination);
        for (std::size_t index{begin}; index < end; ++index) {
            output.data.push_back(dimension_from_record(records[index]));
        }
        return output;
    }

    // ── send_with: GetDimensionInput ────────────────────────────────────────────
    [[nodiscard]] std::expected<GetDimensionOutput, SdkError> send_with(const GetDimensionInput& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        const auto record = m_context->config_store().get(dimension_ns(), input.dimension);
        if (!record) {
            return std::unexpected{map_sdk_error(404, "Failed to get dimension from Superposition")};
        }
        if (!is_json_document(record->get_value())) {
            return std::unexpected{map_sdk_error(400, "Invalid dimension document from Superposition")};
        }
        return GetDimensionOutput{dimension_from_record(*record)};
    }

    // ── send_with: GetDefaultConfigInput ────────────────────────────────────────
    [[nodiscard]] std::expected<GetDefaultConfigOutput, SdkError> send_with(const GetDefaultConfigInput& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        const auto record = m_context->superposition().get_config(default_config_ns(), input.key);
        if (!record) {
            return std::unexpected{map_sdk_error(404, "Failed to get default config from Superposition")};
        }
        if (!is_json_document(record->get_value())) {
            return std::unexpected{map_sdk_error(400, "Invalid default config document from Superposition")};
        }
        return GetDefaultConfigOutput{default_config_from_record(*record)};
    }

    // ── send_with: CreateContextInput ───────────────────────────────────────────
    [[nodiscard]] std::expected<CreateContextOutput, SdkError> send_with(const CreateContextInput& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        // Upstream id is owned by the SDK output (create_context_output_to_struct reads out.id()).
        // No external SDK is linked; the server-owned id is generated in-process (allowed substitute).
        const auto id = m_context->ids().payment_id("ctx_");
        const auto context_json = context_put_document(input.request);

        app::services::ContextRecord record{};
        record.set_name(id);
        record.set_value(context_json);
        record.set_weight(1);
        m_context->superposition().add_context(record);
        m_context->superposition().put_config(default_config_ns(), id, context_json, id);

        put_context_meta(id, "override", "{}");
        put_context_meta(id, "override_id", id + ":override");
        put_context_meta(id, "description", input.request.description.value_or(""));
        put_context_meta(id, "change_reason", input.request.change_reason);
        put_context_meta(id, "created_by", m_context->config_store().get_value(proxy_config_ns(), "service_name").value_or(""));
        put_context_meta(id, "last_modified_by", m_context->config_store().get_value(proxy_config_ns(), "service_name").value_or(""));
        put_context_meta(id, "created_at", "");
        put_context_meta(id, "last_modified_at", "");

        return CreateContextOutput{context_from_record(record)};
    }

    // ── send_with: GetDetailedResolvedConfigInput ───────────────────────────────
    [[nodiscard]] std::expected<GetDetailedResolvedConfigOutput, SdkError> send_with(
        const GetDetailedResolvedConfigInput& input
    ) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        const auto context = context_document(input.context);
        auto keys = default_config_keys();
        // ResolveConfigResponse is #[serde(transparent)] over a BTreeMap: keys are sorted.
        std::ranges::sort(keys);
        serde::Value::Object resolved{};
        if (!keys.empty()) {
            for (const auto& key : keys) {
                const auto value = m_context->superposition().resolve(default_config_ns(), key, context);
                const auto decoded = decode_or_null(value ? value->get_value() : std::string{"null"});
                serde::Value::Object entry{};
                entry.insert(std::string{"description"}, serde::Value{value ? value->get_context() : std::string{}});
                entry.insert(std::string{"schema"}, serde::Value{serde::Value::Object{}});
                entry.insert(std::string{"value"}, serde::Value{decoded});
                resolved.insert(key, serde::Value{std::move(entry)});
            }
        } else {
            for (const auto& [key, value] : input.context) {
                const auto found = m_context->superposition().resolve(
                    default_config_ns(), key, scalar_text(value).value_or("")
                );
                serde::Value::Object entry{};
                entry.insert(std::string{"description"}, serde::Value{std::string{}});
                entry.insert(std::string{"schema"}, serde::Value{serde::Value::Object{}});
                entry.insert(std::string{"value"}, serde::Value{decode_or_null(found ? found->get_value() : serde::Ser::encode_json(value))});
                resolved.insert(key, serde::Value{std::move(entry)});
            }
        }
        return GetDetailedResolvedConfigOutput{serde::Value{std::move(resolved)}};
    }

    // ── send_with: GetResolvedConfigExplanationInput ────────────────────────────
    [[nodiscard]] std::expected<GetResolvedConfigExplanationOutput, SdkError> send_with(
        const GetResolvedConfigExplanationInput& input
    ) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        const auto context = context_document(input.context);
        GetResolvedConfigExplanationOutput output{};
        output.key = input.key;
        for (const auto& log : m_context->superposition().audit_logs()) {
            if (log.get_config_key() != input.key) {
                continue;
            }
            const auto before = m_context->superposition().get_config(default_config_ns(), input.key);
            const auto after = m_context->superposition().resolve(default_config_ns(), input.key, context);
            ResolveExplanationEntryOut entry{};
            entry.context_id = log.get_config_namespace();
            for (const auto& [key, value] : input.context) {
                entry.condition.emplace(key, serde::Value{value});
            }
            entry.override_id = std::to_string(log.get_id());
            entry.value_before = decode_or_null(before ? before->get_value() : std::string{"null"});
            entry.value_after = decode_or_null(after ? after->get_value() : std::string{"null"});
            output.timeline.push_back(std::move(entry));
        }
        return output;
    }

    // ── send_with: ListAuditLogsInput ───────────────────────────────────────────
    [[nodiscard]] std::expected<ListAuditLogsOutput, SdkError> send_with(const ListAuditLogsInput& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        auto logs = m_context->superposition().audit_logs();
        // dimension_params validate only; the local service has no per-row dimensions.
        (void)input.dimension_params;
        std::vector<app::services::AuditLogRecord> filtered{};
        for (const auto& log : logs) {
            if (!input.tables.empty() && !matches_any(input.tables, log.get_config_namespace(), false)) {
                continue;
            }
            if (!input.action.empty() && !matches_any(input.action, log.get_action(), false)) {
                continue;
            }
            const auto username = audit_meta(log.get_id(), "username");
            if (input.username && (!username || *username != *input.username)) {
                continue;
            }
            const auto timestamp = audit_meta(log.get_id(), "timestamp");
            if (input.from_date && (!timestamp || *timestamp < *input.from_date)) {
                continue;
            }
            if (input.to_date && (!timestamp || *timestamp > *input.to_date)) {
                continue;
            }
            filtered.push_back(log);
        }
        if (input.sort_by && *input.sort_by == "desc") {
            std::ranges::sort(filtered, {}, &app::services::AuditLogRecord::get_sequence);
            std::ranges::reverse(filtered);
        } else if (input.sort_by && *input.sort_by == "asc") {
            std::ranges::sort(filtered, {}, &app::services::AuditLogRecord::get_sequence);
        }

        ListAuditLogsOutput output{};
        output.total_items = static_cast<std::int32_t>(filtered.size());
        output.total_pages = total_pages(filtered.size(), input.pagination);
        const auto [begin, end] = slice_bounds(filtered.size(), input.pagination);
        for (std::size_t index{begin}; index < end; ++index) {
            output.data.push_back(audit_log_from_record(filtered[index]));
        }
        return output;
    }

    // ═════════════════════════════════════════════════════════════════════════════
    // Upstream converters (external_services/superposition.rs:235-598)
    // ═════════════════════════════════════════════════════════════════════════════
    [[nodiscard]] static Doc document_to_value(Doc doc) { return doc; }
    [[nodiscard]] static Doc value_to_document(Doc value) { return value; }

    [[nodiscard]] static Doc doc_map_to_json(const DocMap& map)
    {
        serde::Value::Object object{};
        for (const auto& [key, value] : map) {
            object.insert(key, value);
        }
        return Doc{std::move(object)};
    }

    [[nodiscard]] static std::string datetime_to_string(const std::string& value) { return value; }

    [[nodiscard]] static std::optional<std::string> parse_datetime(const std::optional<std::string>& value)
    {
        // Lenient: malformed values are ignored, not rejected (superposition.rs:291-294).
        if (!value || value->empty() || value->find('-') == std::string::npos || value->size() < 10U) {
            return std::nullopt;
        }
        return value;
    }

    [[nodiscard]] static Doc dependency_graph_to_value(
        const std::map<std::string, std::vector<std::string>>& graph
    )
    {
        serde::Value::Object object{};
        for (const auto& [key, values] : graph) {
            serde::Value::Array array{};
            for (const auto& value : values) {
                array.emplace_back(Doc{value});
            }
            object.insert(key, Doc{std::move(array)});
        }
        return Doc{std::move(object)};
    }

    // dimension_type_to_value: REGULAR -> "REGULAR", LOCAL_COHORT(s)/REMOTE_COHORT(s) -> object.
    [[nodiscard]] static Doc dimension_type_to_value(const std::string& encoded)
    {
        if (encoded == "REGULAR") {
            return Doc{std::string{"REGULAR"}};
        }
        if (encoded.starts_with("LOCAL_COHORT:")) {
            serde::Value::Object object{};
            object.insert(std::string{"LOCAL_COHORT"}, Doc{encoded.substr(13U)});
            return Doc{std::move(object)};
        }
        if (encoded.starts_with("REMOTE_COHORT:")) {
            serde::Value::Object object{};
            object.insert(std::string{"REMOTE_COHORT"}, Doc{encoded.substr(14U)});
            return Doc{std::move(object)};
        }
        return Doc{std::string{"UNKNOWN"}};
    }

    [[nodiscard]] app::dto::superposition_proxy::ContextResponse
    context_response_to_struct(const SdkContextResponse& context) const
    {
        app::dto::superposition_proxy::ContextResponse response{};
        response.set_id(context.id);
        response.set_value(doc_map_to_json(context.value));
        response.set_override_(doc_map_to_json(context.override_values));
        response.set_override_id(context.override_id);
        response.set_weight(context.weight);
        response.set_description(context.description);
        response.set_change_reason(context.change_reason);
        response.set_created_at(datetime_to_string(context.created_at));
        response.set_created_by(context.created_by);
        response.set_last_modified_at(datetime_to_string(context.last_modified_at));
        response.set_last_modified_by(context.last_modified_by);
        return response;
    }

    [[nodiscard]] app::dto::superposition_proxy::DefaultConfigResponse
    default_config_response_to_struct(const SdkDefaultConfigResponse& config) const
    {
        app::dto::superposition_proxy::DefaultConfigResponse response{};
        response.set_key(config.key);
        response.set_value(document_to_value(config.value));
        response.set_schema(doc_map_to_json(config.schema));
        response.set_description(config.description);
        response.set_change_reason(config.change_reason);
        response.set_value_validation_function_name(config.value_validation_function_name);
        response.set_value_compute_function_name(config.value_compute_function_name);
        response.set_created_at(datetime_to_string(config.created_at));
        response.set_created_by(config.created_by);
        response.set_last_modified_at(datetime_to_string(config.last_modified_at));
        response.set_last_modified_by(config.last_modified_by);
        return response;
    }

    [[nodiscard]] app::dto::superposition_proxy::DimensionResponse
    dimension_response_to_struct(const SdkDimensionResponse& dimension) const
    {
        app::dto::superposition_proxy::DimensionResponse response{};
        response.set_dimension(dimension.dimension);
        response.set_position(dimension.position);
        response.set_schema(doc_map_to_json(dimension.schema));
        response.set_value_validation_function_name(dimension.value_validation_function_name);
        response.set_description(dimension.description);
        response.set_change_reason(dimension.change_reason);
        response.set_last_modified_at(datetime_to_string(dimension.last_modified_at));
        response.set_last_modified_by(dimension.last_modified_by);
        response.set_created_at(datetime_to_string(dimension.created_at));
        response.set_created_by(dimension.created_by);
        response.set_dependency_graph(dependency_graph_to_value(dimension.dependency_graph));
        response.set_dimension_type(dimension_type_to_value(dimension.dimension_type));
        response.set_value_compute_function_name(dimension.value_compute_function_name);
        response.set_mandatory(dimension.mandatory);
        return response;
    }

    [[nodiscard]] app::dto::superposition_proxy::DimensionResponse
    get_dimension_output_to_struct(const GetDimensionOutput& output) const
    {
        return dimension_response_to_struct(output.dimension);
    }

    [[nodiscard]] app::dto::superposition_proxy::DefaultConfigResponse
    get_default_config_output_to_struct(const GetDefaultConfigOutput& output) const
    {
        return default_config_response_to_struct(output.config);
    }

    [[nodiscard]] app::dto::superposition_proxy::ContextResponse
    create_context_output_to_struct(const CreateContextOutput& output) const
    {
        return context_response_to_struct(output.context);
    }

    [[nodiscard]] app::dto::superposition_proxy::AuditLogResponse
    audit_log_full_to_struct(const SdkAuditLogFull& log) const
    {
        app::dto::superposition_proxy::AuditLogResponse response{};
        response.set_id(log.id);
        response.set_table_name(log.table_name);
        response.set_user_name(log.user_name);
        response.set_timestamp(datetime_to_string(log.timestamp));
        response.set_action(log.action);
        response.set_original_data(log.original_data);
        response.set_new_data(log.new_data);
        response.set_query(log.query);
        return response;
    }

    [[nodiscard]] app::dto::superposition_proxy::ResolveConfigExplanationResponse
    resolve_config_explanation_to_response(const GetResolvedConfigExplanationOutput& output) const
    {
        app::dto::superposition_proxy::ResolveConfigExplanationResponse response{};
        response.set_key(output.key);
        std::vector<app::dto::superposition_proxy::ResolveExplanationEntry> timeline{};
        for (const auto& item : output.timeline) {
            app::dto::superposition_proxy::ResolveExplanationEntry entry{};
            entry.set_context_id(item.context_id);
            entry.set_condition(doc_map_to_json(item.condition));
            entry.set_override_id(item.override_id);
            entry.set_value_before(item.value_before);
            entry.set_value_after(item.value_after);
            timeline.push_back(std::move(entry));
        }
        response.set_timeline(std::move(timeline));
        return response;
    }

    struct PaginatedResult
    {
        std::int32_t total_pages{};
        std::int32_t total_items{};
        std::vector<serde::Value> data{};
    };

    [[nodiscard]] PaginatedResult list_contexts_to_response(const ListContextsOutput& output) const
    {
        PaginatedResult result{};
        result.total_pages = output.total_pages;
        result.total_items = output.total_items;
        for (const auto& item : output.data) {
            result.data.push_back(wire_of(context_response_to_struct(item)));
        }
        return result;
    }

    [[nodiscard]] PaginatedResult list_default_configs_to_response(const ListDefaultConfigsOutput& output) const
    {
        PaginatedResult result{};
        result.total_pages = output.total_pages;
        result.total_items = output.total_items;
        for (const auto& item : output.data) {
            result.data.push_back(wire_of(default_config_response_to_struct(item)));
        }
        return result;
    }

    [[nodiscard]] PaginatedResult list_dimensions_to_response(const ListDimensionsOutput& output) const
    {
        PaginatedResult result{};
        result.total_pages = output.total_pages;
        result.total_items = output.total_items;
        for (const auto& item : output.data) {
            result.data.push_back(wire_of(dimension_response_to_struct(item)));
        }
        return result;
    }

    [[nodiscard]] PaginatedResult list_audit_logs_to_response(const ListAuditLogsOutput& output) const
    {
        PaginatedResult result{};
        result.total_pages = output.total_pages;
        result.total_items = output.total_items;
        for (const auto& item : output.data) {
            result.data.push_back(wire_of(audit_log_full_to_struct(item)));
        }
        return result;
    }

    // ═════════════════════════════════════════════════════════════════════════════
    // context_put_from_request (external_services:538-579)
    // ═════════════════════════════════════════════════════════════════════════════
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
                for (const auto& [key, value] : *context_object) {
                    request.context.emplace_back(key, value);
                }
            }
        }
        const auto* override_value = find_member(*object, "override");
        if (override_value) {
            if (const auto* override_object = std::get_if<serde::Value::Object>(&override_value->get())) {
                for (const auto& [key, value] : *override_object) {
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

    [[nodiscard]] std::optional<ContextPutRequest> context_put_from_request(const ContextPutRequest& request) const
    {
        // builder.build() fails when no context/override entry was supplied.
        if (request.context.empty() && request.override_values.empty()) {
            return std::nullopt;
        }
        return request;
    }

    [[nodiscard]] static std::string context_put_document(const ContextPutRequest& request)
    {
        serde::Value::Object object{};
        for (const auto& [key, value] : request.context) {
            object.insert(key, value);
        }
        return serde::Ser::encode_json(Doc{std::move(object)});
    }

    // ═════════════════════════════════════════════════════════════════════════════
    // Local SDK-output builders from records (substitute for smithy SDK output).
    // ═════════════════════════════════════════════════════════════════════════════
    [[nodiscard]] SdkContextResponse context_from_record(const app::services::ContextRecord& record) const
    {
        SdkContextResponse response{};
        response.id = record.get_name();
        response.value = document_to_map(record.get_value());
        response.override_values = document_to_map(context_meta(record.get_name(), "override").value_or("{}"));
        response.override_id = context_meta(record.get_name(), "override_id").value_or(record.get_name() + ":override");
        response.weight = std::to_string(record.get_weight());
        response.description = context_meta(record.get_name(), "description").value_or("");
        response.change_reason = context_meta(record.get_name(), "change_reason").value_or("");
        response.created_at = context_meta(record.get_name(), "created_at").value_or("");
        response.created_by = context_meta(record.get_name(), "created_by").value_or("");
        response.last_modified_at = context_meta(record.get_name(), "last_modified_at").value_or("");
        response.last_modified_by = context_meta(record.get_name(), "last_modified_by").value_or("");
        return response;
    }

    [[nodiscard]] SdkDefaultConfigResponse default_config_from_record(const app::services::ConfigRecord& record) const
    {
        SdkDefaultConfigResponse response{};
        response.key = record.get_config_key();
        response.value = to_value(record.get_value());
        response.schema = document_to_map(config_meta(record.get_config_key(), "schema").value_or("{}"));
        response.description = config_meta(record.get_config_key(), "description").value_or(record.get_context());
        response.change_reason = config_meta(record.get_config_key(), "change_reason").value_or("");
        response.value_validation_function_name = config_meta(record.get_config_key(), "value_validation_function_name");
        response.value_compute_function_name = config_meta(record.get_config_key(), "value_compute_function_name");
        response.created_at = config_meta(record.get_config_key(), "created_at").value_or("");
        response.created_by = config_meta(record.get_config_key(), "created_by").value_or("");
        response.last_modified_at = config_meta(record.get_config_key(), "last_modified_at").value_or("");
        response.last_modified_by = config_meta(record.get_config_key(), "last_modified_by").value_or("");
        return response;
    }

    [[nodiscard]] SdkDimensionResponse dimension_from_record(const app::services::ConfigRecord& record) const
    {
        const auto& name = record.get_config_key();
        SdkDimensionResponse response{};
        response.dimension = name;
        response.position = static_cast<std::int32_t>(record.get_sequence());
        response.schema = document_to_map(record.get_value());
        response.value_validation_function_name = dimension_meta(name, "value_validation_function_name");
        response.description = dimension_meta(name, "description").value_or(record.get_context());
        response.change_reason = dimension_meta(name, "change_reason").value_or("");
        response.last_modified_at = dimension_meta(name, "last_modified_at").value_or("");
        response.last_modified_by = dimension_meta(name, "last_modified_by").value_or("");
        response.created_at = dimension_meta(name, "created_at").value_or("");
        response.created_by = dimension_meta(name, "created_by").value_or("");
        response.dimension_type = dimension_meta(name, "dimension_type").value_or("REGULAR");
        response.value_compute_function_name = dimension_meta(name, "value_compute_function_name");
        response.mandatory = dimension_meta(name, "mandatory").value_or("false") == "true";
        const auto graph = dimension_meta(name, "dependency_graph");
        if (graph) {
            auto decoded = serde::Ser::decode_generic("application/json", *graph);
            if (decoded) {
                if (const auto* object = std::get_if<serde::Value::Object>(&decoded->get())) {
                    for (const auto& [key, value] : *object) {
                        std::vector<std::string> values{};
                        if (const auto* array = std::get_if<serde::Value::Array>(&value.get())) {
                            for (const auto& element : *array) {
                                if (const auto* text = std::get_if<std::string>(&element.get())) {
                                    values.push_back(*text);
                                }
                            }
                        }
                        response.dependency_graph.emplace(key, std::move(values));
                    }
                }
            }
        }
        return response;
    }

    [[nodiscard]] SdkAuditLogFull audit_log_from_record(const app::services::AuditLogRecord& log) const
    {
        SdkAuditLogFull entry{};
        entry.id = std::to_string(log.get_id());
        entry.table_name = log.get_config_namespace();
        entry.user_name = audit_meta(log.get_id(), "username").value_or("");
        entry.timestamp = audit_meta(log.get_id(), "timestamp").value_or("");
        entry.action = log.get_action();
        if (const auto original = audit_meta(log.get_id(), "original_data")) {
            entry.original_data = to_value(*original);
        }
        if (const auto latest = audit_meta(log.get_id(), "new_data")) {
            entry.new_data = to_value(*latest);
        }
        entry.query = log.get_detail();
        return entry;
    }

    // ═════════════════════════════════════════════════════════════════════════════
    // Wire builders — exact upstream serde shapes.
    // ═════════════════════════════════════════════════════════════════════════════
    template<typename T>
    [[nodiscard]] static serde::Value wire_of(const T& dto)
    {
        const auto text = serde::Ser::encode_json(dto);
        auto parsed = serde::Ser::decode_generic("application/json", text);
        return parsed ? std::move(*parsed) : serde::Value{};
    }

    [[nodiscard]] static serde::Value wire_paginated(const PaginatedResult& result)
    {
        serde::Value::Object object{};
        object.insert(std::string{"total_pages"}, serde::Value{static_cast<std::int64_t>(result.total_pages)});
        object.insert(std::string{"total_items"}, serde::Value{static_cast<std::int64_t>(result.total_items)});
        serde::Value::Array data{};
        for (const auto& item : result.data) {
            data.push_back(item);
        }
        object.insert(std::string{"data"}, serde::Value{std::move(data)});
        return serde::Value{std::move(object)};
    }

    // ResolveConfigResponse is #[serde(transparent)] over BTreeMap<String, ResolvedConfigEntry>.
    [[nodiscard]] static serde::Value wire_resolve(const serde::Value& decoded)
    {
        return decoded;
    }

    // ═════════════════════════════════════════════════════════════════════════════
    // Errors + response completion.
    // ═════════════════════════════════════════════════════════════════════════════
    [[nodiscard]] static interfaces::io::types::Status api_error_status(ApiError::Kind kind)
    {
        using interfaces::io::types::Status;
        switch (kind) {
        case ApiError::Kind::InvalidRequestData: return Status::UNPROCESSABLE_CONTENT;
        case ApiError::Kind::AccessForbidden: return Status::FORBIDDEN;
        case ApiError::Kind::InternalServerError: return Status::INTERNAL_SERVER_ERROR;
        case ApiError::Kind::MissingHeader:
        case ApiError::Kind::BadRequestBody: return Status::BAD_REQUEST;
        }
        return Status::INTERNAL_SERVER_ERROR;
    }

    [[nodiscard]] static serde::Value api_error_wire(const ApiError& error)
    {
        serde::Value::Object inner{};
        switch (error.kind) {
        case ApiError::Kind::InvalidRequestData:
            inner.insert(std::string{"type"}, serde::Value{std::string{"invalid_request_error"}});
            inner.insert(std::string{"message"}, serde::Value{error.message});
            inner.insert(std::string{"code"}, serde::Value{std::string{"IR_06"}});
            break;
        case ApiError::Kind::AccessForbidden:
            inner.insert(std::string{"type"}, serde::Value{std::string{"invalid_request_error"}});
            inner.insert(
                std::string{"message"},
                serde::Value{"Access forbidden. Not authorized to access this resource " + error.resource}
            );
            inner.insert(std::string{"code"}, serde::Value{std::string{"IR_22"}});
            break;
        case ApiError::Kind::InternalServerError:
            inner.insert(std::string{"type"}, serde::Value{std::string{"server_not_available"}});
            inner.insert(std::string{"message"}, serde::Value{error.message});
            inner.insert(std::string{"code"}, serde::Value{std::string{"HE_00"}});
            break;
        case ApiError::Kind::MissingHeader:
        case ApiError::Kind::BadRequestBody:
            inner.insert(std::string{"message"}, serde::Value{error.message});
            break;
        }
        serde::Value::Object root{};
        root.insert(std::string{"error"}, serde::Value{std::move(inner)});
        return serde::Value{std::move(root)};
    }

    void send_api_error(
        interfaces::io::IResponse& res,
        std::function<void()>& send,
        const ApiError& error
    ) const
    {
        write_response(res, send, api_error_wire(error), api_error_status(error.kind));
    }

    void send_header_error(
        interfaces::io::IResponse& res,
        std::function<void()>& send,
        std::string_view message
    ) const
    {
        send_api_error(res, send, {ApiError::Kind::MissingHeader, std::string{message}, {}});
    }

    void finish(
        interfaces::io::IResponse& res,
        std::function<void()>& send,
        std::expected<serde::Value, ApiError> result,
        std::string_view flow,
        std::string_view counter
    ) const
    {
        if (!result) {
            send_api_error(res, send, result.error());
            return;
        }
        track_flow(flow, counter);
        write_response(res, send, *result, interfaces::io::types::Status::OK);
    }

    void write_response(
        interfaces::io::IResponse& res,
        std::function<void()>& send,
        const serde::Value& value,
        interfaces::io::types::Status status
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", serde::Ser::encode_json(value));
        res.set_status(status);
        res.set_body(std::move(bytes));
        send();
    }

    // ═════════════════════════════════════════════════════════════════════════════
    // Auth (upstream JWT ProfileSuperpositionConfigRead/Write, allow_connected/platform).
    // ═════════════════════════════════════════════════════════════════════════════
    [[nodiscard]] std::optional<ApiError> authorize_request(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        const auto auth = app::auth::AuthContextBuilder{}(req);
        // Upstream: allow_connected=true, allow_platform=true for every superposition route.
        // The local registry rule is re-based with the upstream allow flags; the dedicated
        // ProfileSuperpositionConfigRead/Write permission is a shared-contract request
        // (AppPermission enum + registry are shared files), so PROFILE_READ/WRITE is the
        // in-process stand-in that carries the same claim-check semantics.
        const bool write = operation == "create_context";
        const auto rebased = app::auth::RouteAuthRuleBuilder{}
                                 .scheme(rule.scheme())
                                 .permission(write
                                                 ? app::auth::AppPermission::PROFILE_WRITE
                                                 : app::auth::AppPermission::PROFILE_READ)
                                 .allow_connected(true)
                                 .allow_platform(true)
                                 .build();
        const auto denied = m_context->authorizer().authorize_route(auth, rebased);
        if (!denied) {
            return std::nullopt;
        }
        return access_forbidden("superposition");
    }

    // ═════════════════════════════════════════════════════════════════════════════
    // Namespaces, scope cache + small utilities.
    // ═════════════════════════════════════════════════════════════════════════════
    [[nodiscard]] std::string_view default_config_ns() const noexcept { return "superposition.default_config"; }
    [[nodiscard]] std::string_view dimension_ns() const noexcept { return "superposition.dimension"; }
    [[nodiscard]] std::string_view context_meta_ns() const noexcept { return "superposition.context_meta"; }
    [[nodiscard]] std::string_view default_config_meta_ns() const noexcept { return "superposition.default_config_meta"; }
    [[nodiscard]] std::string_view dimension_meta_ns() const noexcept { return "superposition.dimension_meta"; }
    [[nodiscard]] std::string_view audit_meta_ns() const noexcept { return "superposition.audit_meta"; }
    [[nodiscard]] std::string_view proxy_config_ns() const noexcept { return "superposition_proxy"; }

    void cache_scope(interfaces::io::IRequest& req, const ProxyScope& scope) const
    {
        const auto token = app::auth::TokenParser{}(req).value();
        const auto digest = app::crypto::KeyedHasher{"superposition_proxy.scope"}.hash(
            scope.org_id + ":" + scope.workspace_id + ":" + token
        );
        const auto key = "superposition_proxy.scope." + digest;
        if (!m_context->cache().get(key).hit) {
            m_context->cache().put(
                key, core::cache::to_cache_value(scope.org_id + ":" + scope.workspace_id)
            );
        }
    }

    [[nodiscard]] static std::string last_path_segment(interfaces::io::IRequest& req)
    {
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() == 0U) {
            return {};
        }
        return std::string{segments.at(segments.count() - 1U).value_or("")};
    }

    [[nodiscard]] static std::vector<std::pair<std::string, std::string>> query_pairs(
        interfaces::io::IRequest& req
    )
    {
        std::vector<std::pair<std::string, std::string>> pairs{};
        const auto path = std::string{req.get_path()};
        const auto pos = path.find('?');
        if (pos == std::string::npos) {
            return pairs;
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
        return pairs;
    }

    [[nodiscard]] static std::string percent_decode(std::string_view value)
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

    [[nodiscard]] static int hex_digit(char ch) noexcept
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

    [[nodiscard]] std::optional<std::string> context_meta(std::string_view name, std::string_view key) const
    {
        return m_context->config_store().get_value(
            context_meta_ns(), std::string{name} + "." + std::string{key}
        );
    }

    void put_context_meta(std::string_view name, std::string_view key, std::string value) const
    {
        m_context->config_store().put(
            context_meta_ns(), std::string{name} + "." + std::string{key}, std::move(value)
        );
    }

    [[nodiscard]] std::optional<std::string> config_meta(std::string_view key, std::string_view field) const
    {
        return m_context->config_store().get_value(
            default_config_meta_ns(), std::string{key} + "." + std::string{field}
        );
    }

    [[nodiscard]] std::optional<std::string> dimension_meta(std::string_view name, std::string_view field) const
    {
        return m_context->config_store().get_value(
            dimension_meta_ns(), std::string{name} + "." + std::string{field}
        );
    }

    [[nodiscard]] std::optional<std::string> audit_meta(std::int64_t id, std::string_view key) const
    {
        return m_context->config_store().get_value(
            audit_meta_ns(), std::to_string(id) + "." + std::string{key}
        );
    }

    [[nodiscard]] std::vector<std::string> default_config_keys() const
    {
        std::vector<std::string> keys{};
        for (const auto& log : m_context->superposition().audit_logs()) {
            if (log.get_config_namespace() != default_config_ns() || log.get_action() != "put") {
                continue;
            }
            if (std::ranges::find(keys, log.get_config_key()) == keys.end()) {
                keys.push_back(log.get_config_key());
            }
        }
        return keys;
    }

    // ResolveConfigBody.context == serde_json::Map<String, Value> (values kept verbatim).
    [[nodiscard]] std::vector<std::pair<std::string, Doc>> context_entries(
        const serde::Value& root
    ) const
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
        for (const auto& [key, value] : *context_object) {
            entries.emplace_back(key, value);
        }
        return entries;
    }

    [[nodiscard]] static std::string context_document(
        const std::vector<std::pair<std::string, Doc>>& entries
    )
    {
        serde::Value::Object object{};
        for (const auto& [key, value] : entries) {
            object.insert(key, value);
        }
        return serde::Ser::encode_json(Doc{std::move(object)});
    }

    [[nodiscard]] static const serde::Value* find_member(const serde::Value::Object& object, std::string_view key)
    {
        for (const auto& [member, value] : object) {
            if (member == key) {
                return &value;
            }
        }
        return nullptr;
    }

    [[nodiscard]] static std::optional<std::string> scalar_text(const serde::Value& value)
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

    [[nodiscard]] static DocMap document_to_map(std::string_view json)
    {
        DocMap map{};
        auto decoded = serde::Ser::decode_generic("application/json", json);
        if (!decoded) {
            return map;
        }
        if (const auto* object = std::get_if<serde::Value::Object>(&decoded->get())) {
            for (const auto& [key, value] : *object) {
                map.emplace(key, value);
            }
        }
        return map;
    }

    [[nodiscard]] static Doc decode_or_null(std::string_view json)
    {
        auto decoded = serde::Ser::decode_generic("application/json", json);
        return decoded ? std::move(*decoded) : Doc{};
    }

    [[nodiscard]] static Doc to_value(std::string_view json)
    {
        return decode_or_null(json);
    }

    [[nodiscard]] static std::string join(const std::vector<std::string>& values, std::string_view separator)
    {
        std::string result{};
        for (std::size_t i{0}; i < values.size(); ++i) {
            if (i > 0U) {
                result += separator;
            }
            result += values[i];
        }
        return result;
    }

    void sort_contexts(
        std::vector<app::services::ContextRecord>& records,
        std::string_view sort_on,
        std::string_view sort_by
    ) const
    {
        if (sort_on != "name" && sort_on != "value" && sort_on != "weight") {
            return;
        }
        const bool descending = sort_by == "desc";
        std::ranges::sort(records, [sort_on, descending](
            const app::services::ContextRecord& lhs,
            const app::services::ContextRecord& rhs
        ) {
            if (sort_on == "name") {
                return descending ? lhs.get_name() > rhs.get_name()
                                  : lhs.get_name() < rhs.get_name();
            }
            if (sort_on == "value") {
                return descending ? lhs.get_value() > rhs.get_value()
                                  : lhs.get_value() < rhs.get_value();
            }
            return descending ? lhs.get_weight() > rhs.get_weight()
                              : lhs.get_weight() < rhs.get_weight();
        });
    }

    [[nodiscard]] static bool matches_any(
        const std::vector<std::string>& values,
        std::string_view candidate,
        bool prefix
    )
    {
        for (const auto& value : values) {
            if (prefix ? candidate.starts_with(value) : candidate == value) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] static bool contains_ci(std::string_view haystack, std::string_view needle)
    {
        if (needle.empty()) {
            return true;
        }
        if (needle.size() > haystack.size()) {
            return false;
        }
        for (std::size_t i{0}; i + needle.size() <= haystack.size(); ++i) {
            bool equal{true};
            for (std::size_t j{0}; j < needle.size(); ++j) {
                if (std::tolower(static_cast<unsigned char>(haystack[i + j])) !=
                    std::tolower(static_cast<unsigned char>(needle[j]))) {
                    equal = false;
                    break;
                }
            }
            if (equal) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] static std::optional<std::int32_t> parse_int(const std::optional<std::string>& value)
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

    [[nodiscard]] static std::optional<bool> parse_bool(const std::optional<std::string>& value)
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

    [[nodiscard]] static std::pair<std::size_t, std::size_t> slice_bounds(
        std::size_t total,
        const Pagination& paging
    ) noexcept
    {
        if ((paging.all && *paging.all) || total == 0U) {
            return {0U, total};
        }
        const auto limit = static_cast<std::size_t>(
            paging.count && *paging.count > 0 ? *paging.count : 20
        );
        const auto page = static_cast<std::size_t>(
            paging.page && *paging.page > 0 ? *paging.page : 1
        );
        const auto start = (page - 1U) * limit;
        if (start >= total) {
            return {total, total};
        }
        return {start, std::min(total, start + limit)};
    }

    [[nodiscard]] static std::int32_t total_pages(std::size_t total, const Pagination& paging) noexcept
    {
        if (total == 0U) {
            return 0;
        }
        if (paging.all && *paging.all) {
            return 1;
        }
        const auto limit = static_cast<std::size_t>(
            paging.count && *paging.count > 0 ? *paging.count : 20
        );
        return static_cast<std::int32_t>((total + limit - 1U) / limit);
    }

    [[nodiscard]] static bool is_json_document(std::string_view json)
    {
        return serde::Ser::decode_generic("application/json", json).has_value();
    }

    // ApiEventMetric analogue: upstream instruments every response with ApiEventMetric
    // and ApplicationResponse::Json; the local equivalent is a paired named flow/counter.
    void track_flow(std::string_view flow, std::string_view counter) const
    {
        m_context->metrics().record_flow(flow);
        m_context->metrics().counter(counter).increment();
        m_context->metrics().record_flow(std::string{"api_event."} + std::string{flow});
        m_context->metrics().counter(std::string{"api_event."} + std::string{counter}).increment();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::superposition_proxy
