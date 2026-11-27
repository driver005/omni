export module app_routing:superposition_proxy_runtime_common;

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
import app_metrics;
import app_store;
import app_dto;
import app_model;
import app_services;
import utils_openapi;
import core_behavior;

import :superposition_proxy_input;

export namespace app::handlers::superposition_proxy {

// ── Upstream ScopingDimension + validate_scope (core:27-133) ──────────────
enum class ScopingDimension : std::uint8_t
{
    OrganizationId,
    MerchantId,
    ProfileId,
    ProviderMerchantId,
    ProcessorMerchantId,
};

inline constexpr std::array<ScopingDimension, 4> kRequiredDimensions{
    ScopingDimension::OrganizationId,
    ScopingDimension::ProfileId,
    ScopingDimension::ProcessorMerchantId,
    ScopingDimension::ProviderMerchantId,
};

// ── Local SDK error + input/output stand-ins ─────────────────────────────
enum class SdkErrorKind : std::uint8_t
{
    BadRequest,
    NotFound,
    ClientError,
};

struct SdkError
{
    SdkErrorKind kind{SdkErrorKind::ClientError};
    std::string message{};
};

struct ListContextsSdkRequest
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

struct ListDefaultConfigsSdkRequest
{
    std::string org_id{};
    std::string workspace_id{};
    Pagination pagination{};
    std::optional<std::string> name{};
};

struct ListDimensionsSdkRequest
{
    std::string org_id{};
    std::string workspace_id{};
    Pagination pagination{};
};

struct GetDimensionSdkRequest
{
    std::string org_id{};
    std::string workspace_id{};
    std::string dimension{};
};

struct GetDefaultConfigSdkRequest
{
    std::string org_id{};
    std::string workspace_id{};
    std::string key{};
};

struct CreateContextSdkRequest
{
    std::string org_id{};
    std::string workspace_id{};
    ContextPutRequest request{};
};

struct GetDetailedResolvedConfigSdkRequest
{
    std::string org_id{};
    std::string workspace_id{};
    std::vector<std::pair<std::string, Doc>> context{};
};

struct GetResolvedConfigExplanationSdkRequest
{
    std::string org_id{};
    std::string workspace_id{};
    std::string key{};
    std::vector<std::pair<std::string, Doc>> context{};
};

struct ListAuditLogsSdkRequest
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

struct PaginatedResult
{
    std::int32_t total_pages{};
    std::int32_t total_items{};
    std::vector<serde::Value> data{};
};

/**
 * @brief Shared business helpers for every superposition_proxy sub-runtime.
 *
 * Holds the AppContext reference and reuses SuperpositionProxyInputSupport for
 * error envelopes. No HTTP types, no IRequest/IResponse, no send callback.
 */
class SuperpositionProxyRuntimeSupport
{
public:
    explicit SuperpositionProxyRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_inputs{context}
    {
    }

    [[nodiscard]] static std::string workspace_id(const ::core::behavior::CallContext& ctx)
    {
        const auto it = ctx.headers.find("x-workspace");
        return it == ctx.headers.end() ? std::string{} : it->second;
    }

    // ── Response completion: expected wire/error -> typed document ────────
    [[nodiscard]] core::behavior::Outcome<serde::Value> finish(
        std::expected<serde::Value, ApiError> result,
        std::string_view flow,
        std::string_view counter
    ) const
    {
        if (!result) {
            return std::unexpected{m_inputs.make_api_error(result.error())};
        }
        track_flow(flow, counter);
        return std::move(*result);
    }

    // ── handle_superposition_proxy_flow dispatch (core:344-355) ───────────
    [[nodiscard]] std::expected<serde::Value, ApiError>
    execute(const ListContextsQuery& request, const ::core::behavior::CallContext& ctx) const
    {
        if (const auto error = validate_superposition_params(request.dimension_params, ctx)) {
            return std::unexpected{*error};
        }
        auto output = send_with(into_input(request, ctx.org_id, workspace_id(ctx)));
        if (!output) {
            return std::unexpected{
                map_superposition_err(output.error(), "Failed to list contexts from Superposition")
            };
        }
        return wire_paginated(list_contexts_to_response(*output));
    }

    [[nodiscard]] std::expected<serde::Value, ApiError>
    execute(const ListDefaultConfigsQuery& request, const ::core::behavior::CallContext& ctx) const
    {
        auto output = send_with(into_input(request, ctx.org_id, workspace_id(ctx)));
        if (!output) {
            return std::unexpected{map_superposition_err(
                output.error(),
                "Failed to list default configs from Superposition"
            )};
        }
        return wire_paginated(list_default_configs_to_response(*output));
    }

    [[nodiscard]] std::expected<serde::Value, ApiError>
    execute(const ListDimensionsQuery& request, const ::core::behavior::CallContext& ctx) const
    {
        auto output = send_with(into_input(request, ctx.org_id, workspace_id(ctx)));
        if (!output) {
            return std::unexpected{map_superposition_err(
                output.error(),
                "Failed to list dimensions from Superposition"
            )};
        }
        return wire_paginated(list_dimensions_to_response(*output));
    }

    [[nodiscard]] std::expected<serde::Value, ApiError>
    execute(const GetDimensionRequest& request, const ::core::behavior::CallContext& ctx) const
    {
        auto output = send_with(into_input(request, ctx.org_id, workspace_id(ctx)));
        if (!output) {
            return std::unexpected{
                map_superposition_err(output.error(), "Failed to get dimension from Superposition")
            };
        }
        return wire_of(get_dimension_output_to_struct(*output));
    }

    [[nodiscard]] std::expected<serde::Value, ApiError>
    execute(const GetDefaultConfigRequest& request, const ::core::behavior::CallContext& ctx) const
    {
        auto output = send_with(into_input(request, ctx.org_id, workspace_id(ctx)));
        if (!output) {
            return std::unexpected{map_superposition_err(
                output.error(),
                "Failed to get default config from Superposition"
            )};
        }
        return wire_of(get_default_config_output_to_struct(*output));
    }

    [[nodiscard]] std::expected<serde::Value, ApiError>
    execute(const ContextPutRequest& request, const ::core::behavior::CallContext& ctx) const
    {
        auto input = into_input(request, ctx.org_id, workspace_id(ctx));
        if (!input) {
            return std::unexpected{
                map_superposition_err(input.error(), "Invalid context request body")
            };
        }
        // doc_map_to_json(context_put.context()) -> validate context body.
        std::vector<std::pair<std::string, Doc>> context_docs{};
        for (const auto& [key, value]: input->request.context) {
            context_docs.emplace_back(key, value);
        }
        if (const auto error = validate_superposition_context_body(context_docs, ctx)) {
            return std::unexpected{*error};
        }
        auto output = send_with(*input);
        if (!output) {
            return std::unexpected{
                map_superposition_err(output.error(), "Failed to create context in Superposition")
            };
        }
        return wire_of(create_context_output_to_struct(*output));
    }

    [[nodiscard]] std::expected<serde::Value, ApiError> execute(
        const ResolveDetailedConfigRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        if (const auto error = validate_superposition_context_body(request.context, ctx)) {
            return std::unexpected{*error};
        }
        auto output = send_with(into_input(request, ctx.org_id, workspace_id(ctx)));
        if (!output) {
            return std::unexpected{
                map_superposition_err(output.error(), "Failed to resolve config from Superposition")
            };
        }
        // ResolveConfigResponse is transparent over the resolved config document.
        return wire_resolve(output->config);
    }

    [[nodiscard]] std::expected<serde::Value, ApiError> execute(
        const ResolveConfigExplanationRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        if (const auto error = validate_superposition_context_body(request.context, ctx)) {
            return std::unexpected{*error};
        }
        auto output = send_with(into_input(request, ctx.org_id, workspace_id(ctx)));
        if (!output) {
            return std::unexpected{map_superposition_err(
                output.error(),
                "Failed to resolve config explanation from Superposition"
            )};
        }
        return wire_of(resolve_config_explanation_to_response(*output));
    }

    [[nodiscard]] std::expected<serde::Value, ApiError>
    execute(const ListAuditLogsQuery& request, const ::core::behavior::CallContext& ctx) const
    {
        if (const auto error = validate_superposition_params(request.dimension_params, ctx)) {
            return std::unexpected{*error};
        }
        auto output = send_with(into_input(request, ctx.org_id, workspace_id(ctx)));
        if (!output) {
            return std::unexpected{map_superposition_err(
                output.error(),
                "Failed to list audit logs from Superposition"
            )};
        }
        return wire_paginated(list_audit_logs_to_response(*output));
    }

    // ── Scope validation ──────────────────────────────────────────────────
    [[nodiscard]] std::optional<ApiError> validate_superposition_params(
        const std::vector<std::pair<std::string, std::string>>& params,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        std::vector<std::pair<ScopingDimension, std::optional<std::string_view>>> dimensions{};
        for (const auto& [key, value]: params) {
            if (const auto dimension = from_dimension_param(key)) {
                dimensions.emplace_back(*dimension, std::optional<std::string_view>{value});
            }
        }
        return validate_scope(dimensions, ctx);
    }

    [[nodiscard]] std::optional<ApiError> validate_superposition_context_body(
        const std::vector<std::pair<std::string, Doc>>& context,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        std::vector<std::pair<ScopingDimension, std::optional<std::string_view>>> dimensions{};
        for (const auto& [key, value]: context) {
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
        return validate_scope(dimensions, ctx);
    }

    [[nodiscard]] std::optional<ApiError> validate_scope(
        const std::vector<std::pair<ScopingDimension, std::optional<std::string_view>>>& dimensions,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        std::vector<std::string> missing{};
        for (const auto required: kRequiredDimensions) {
            const bool present = std::ranges::any_of(
                dimensions,
                [required](const auto& entry)
                {
                    return entry.first == required;
                }
            );
            if (!present) {
                missing.emplace_back(std::string{dimension_name(required)});
            }
        }
        if (!missing.empty()) {
            return m_inputs.invalid_request_data("scope is missing: " + join(missing, ", "));
        }

        const bool out_of_scope = std::ranges::any_of(
            dimensions,
            [this, &ctx](const auto& entry)
            {
                return entry.second !=
                       std::optional<std::string_view>{expected_value(entry.first, ctx)};
            }
        );
        if (out_of_scope) {
            return m_inputs.access_forbidden("superposition");
        }
        return std::nullopt;
    }

    // ── Error mapping (core:135-192) ──────────────────────────────────────
    [[nodiscard]] SdkError map_sdk_error(int status, std::string message) const
    {
        if (status == 404) {
            return {SdkErrorKind::NotFound, std::move(message)};
        }
        if (status >= 400 && status < 500) {
            return {SdkErrorKind::BadRequest, std::move(message)};
        }
        return {SdkErrorKind::ClientError, std::move(message)};
    }

    [[nodiscard]] ApiError
    map_superposition_err(const SdkError& error, std::string_view /*context*/) const
    {
        switch (error.kind) {
            case SdkErrorKind::BadRequest:
                return m_inputs.invalid_request_data(error.message);
            case SdkErrorKind::NotFound:
                return m_inputs.invalid_request_data(
                    "The specified org_id or workspace_id was not found in Superposition"
                );
            case SdkErrorKind::ClientError:
                return m_inputs.internal_server_error();
        }
        return m_inputs.internal_server_error();
    }

    // ── Input builders (external_services SDK builders) ───────────────────
    [[nodiscard]] ListContextsSdkRequest into_input(
        const ListContextsQuery& request,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        ListContextsSdkRequest input{};
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

    [[nodiscard]] ListDefaultConfigsSdkRequest into_input(
        const ListDefaultConfigsQuery& request,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        return {std::string{org_id}, std::string{workspace_id}, request.pagination, request.name};
    }

    [[nodiscard]] ListDimensionsSdkRequest into_input(
        const ListDimensionsQuery& request,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        return {std::string{org_id}, std::string{workspace_id}, request.pagination};
    }

    [[nodiscard]] GetDimensionSdkRequest into_input(
        const GetDimensionRequest& request,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        return {std::string{org_id}, std::string{workspace_id}, request.dimension};
    }

    [[nodiscard]] GetDefaultConfigSdkRequest into_input(
        const GetDefaultConfigRequest& request,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        return {std::string{org_id}, std::string{workspace_id}, request.key};
    }

    [[nodiscard]] std::expected<CreateContextSdkRequest, SdkError> into_input(
        const ContextPutRequest& request,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        // context_put_from_request(&self) maps errors to ClientError (core:589-590).
        auto context_put = context_put_from_request(request);
        if (!context_put) {
            return std::unexpected{
                SdkError{SdkErrorKind::ClientError, "Failed to build ContextPut"}
            };
        }
        return CreateContextSdkRequest{
            std::string{org_id},
            std::string{workspace_id},
            std::move(*context_put)
        };
    }

    [[nodiscard]] GetDetailedResolvedConfigSdkRequest into_input(
        const ResolveDetailedConfigRequest& request,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        return {std::string{org_id}, std::string{workspace_id}, request.context};
    }

    [[nodiscard]] GetResolvedConfigExplanationSdkRequest into_input(
        const ResolveConfigExplanationRequest& request,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        return {std::string{org_id}, std::string{workspace_id}, request.key, request.context};
    }

    [[nodiscard]] ListAuditLogsSdkRequest into_input(
        const ListAuditLogsQuery& request,
        std::string_view org_id,
        std::string_view workspace_id
    ) const
    {
        ListAuditLogsSdkRequest input{};
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

    // ── send_with: in-process substitute for the external SDK ─────────────
    [[nodiscard]] std::expected<ListContextsOutput, SdkError>
    send_with(const ListContextsSdkRequest& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        auto records = m_context.superposition().list_contexts();

        // dimension_params (post from_dimension_param validation): filter by scoped value.
        const bool prefix_match =
            input.dimension_match_strategy && *input.dimension_match_strategy == "prefix";
        if (!input.dimension_params.empty()) {
            std::vector<::app::services::ContextRecord> scoped{};
            for (const auto& record: records) {
                bool keep{true};
                for (const auto& [key, value]: input.dimension_params) {
                    if (!from_dimension_param(key)) {
                        continue;
                    }
                    const bool matches = prefix_match ? record.get_name().starts_with(value)
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
            std::vector<::app::services::ContextRecord> prefixed{};
            for (const auto& record: records) {
                if (matches_any(input.prefix, record.get_name(), true)) {
                    prefixed.push_back(record);
                }
            }
            records = std::move(prefixed);
        }
        if (!input.created_by.empty() || !input.last_modified_by.empty()) {
            std::vector<::app::services::ContextRecord> owned{};
            for (const auto& record: records) {
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
            std::vector<::app::services::ContextRecord> plain{};
            for (const auto& record: records) {
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

    [[nodiscard]] std::expected<ListDefaultConfigsOutput, SdkError>
    send_with(const ListDefaultConfigsSdkRequest& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        std::vector<::app::services::ConfigRecord> records{};
        if (input.name && !input.name->empty()) {
            if (const auto record =
                    m_context.superposition().get_config(default_config_ns(), *input.name)) {
                records.push_back(*record);
            }
        } else {
            for (const auto& entry: m_context.superposition().audit_logs()) {
                if (entry.get_config_namespace() != default_config_ns() ||
                    entry.get_action() != "put") {
                    continue;
                }
                if (const auto record = m_context.superposition().get_config(
                        default_config_ns(),
                        entry.get_config_key()
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

    [[nodiscard]] std::expected<ListDimensionsOutput, SdkError>
    send_with(const ListDimensionsSdkRequest& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        const auto records = m_context.config_store().list(dimension_ns());
        ListDimensionsOutput output{};
        output.total_items = static_cast<std::int32_t>(records.size());
        output.total_pages = total_pages(records.size(), input.pagination);
        const auto [begin, end] = slice_bounds(records.size(), input.pagination);
        for (std::size_t index{begin}; index < end; ++index) {
            output.data.push_back(dimension_from_record(records[index]));
        }
        return output;
    }

    [[nodiscard]] std::expected<GetDimensionOutput, SdkError>
    send_with(const GetDimensionSdkRequest& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        const auto record = m_context.config_store().get(dimension_ns(), input.dimension);
        if (!record) {
            return std::unexpected{
                map_sdk_error(404, "Failed to get dimension from Superposition")
            };
        }
        if (!is_json_document(record->get_value())) {
            return std::unexpected{
                map_sdk_error(400, "Invalid dimension document from Superposition")
            };
        }
        return GetDimensionOutput{dimension_from_record(*record)};
    }

    [[nodiscard]] std::expected<GetDefaultConfigOutput, SdkError>
    send_with(const GetDefaultConfigSdkRequest& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        const auto record = m_context.superposition().get_config(default_config_ns(), input.key);
        if (!record) {
            return std::unexpected{
                map_sdk_error(404, "Failed to get default config from Superposition")
            };
        }
        if (!is_json_document(record->get_value())) {
            return std::unexpected{
                map_sdk_error(400, "Invalid default config document from Superposition")
            };
        }
        return GetDefaultConfigOutput{default_config_from_record(*record)};
    }

    [[nodiscard]] std::expected<CreateContextOutput, SdkError>
    send_with(const CreateContextSdkRequest& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        // Upstream id is owned by the SDK output; generated in-process (allowed substitute).
        const auto id = m_context.ids().payment_id("ctx_");
        const auto context_json = context_put_document(input.request);

        ::app::services::ContextRecord record{};
        record.set_name(id);
        record.set_value(context_json);
        record.set_weight(1);
        m_context.superposition().add_context(record);
        m_context.superposition().put_config(default_config_ns(), id, context_json, id);

        put_context_meta(id, "override", "{}");
        put_context_meta(id, "override_id", id + ":override");
        put_context_meta(id, "description", input.request.description.value_or(""));
        put_context_meta(id, "change_reason", input.request.change_reason);
        put_context_meta(
            id,
            "created_by",
            m_context.config_store().get_value(proxy_config_ns(), "service_name").value_or("")
        );
        put_context_meta(
            id,
            "last_modified_by",
            m_context.config_store().get_value(proxy_config_ns(), "service_name").value_or("")
        );
        put_context_meta(id, "created_at", "");
        put_context_meta(id, "last_modified_at", "");

        return CreateContextOutput{context_from_record(record)};
    }

    [[nodiscard]] std::expected<GetDetailedResolvedConfigOutput, SdkError>
    send_with(const GetDetailedResolvedConfigSdkRequest& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        const auto context = context_document(input.context);
        auto keys = default_config_keys();
        // ResolveConfigResponse is #[serde(transparent)] over a BTreeMap: keys are sorted.
        std::ranges::sort(keys);
        serde::Value::Object resolved{};
        if (!keys.empty()) {
            for (const auto& key: keys) {
                const auto value =
                    m_context.superposition().resolve(default_config_ns(), key, context);
                const auto decoded =
                    decode_or_null(value ? value->get_value() : std::string{"null"});
                serde::Value::Object entry{};
                entry.insert(
                    std::string{"description"},
                    serde::Value{value ? value->get_context() : std::string{}}
                );
                entry.insert(std::string{"schema"}, serde::Value{serde::Value::Object{}});
                entry.insert(std::string{"value"}, serde::Value{decoded});
                resolved.insert(key, serde::Value{std::move(entry)});
            }
        } else {
            for (const auto& [key, value]: input.context) {
                const auto found = m_context.superposition().resolve(
                    default_config_ns(),
                    key,
                    m_inputs.scalar_text(value).value_or("")
                );
                serde::Value::Object entry{};
                entry.insert(std::string{"description"}, serde::Value{std::string{}});
                entry.insert(std::string{"schema"}, serde::Value{serde::Value::Object{}});
                entry.insert(
                    std::string{"value"},
                    serde::Value{found ? decode_or_null(found->get_value()) : value}
                );
                resolved.insert(key, serde::Value{std::move(entry)});
            }
        }
        return GetDetailedResolvedConfigOutput{serde::Value{std::move(resolved)}};
    }

    [[nodiscard]] std::expected<GetResolvedConfigExplanationOutput, SdkError>
    send_with(const GetResolvedConfigExplanationSdkRequest& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        const auto context = context_document(input.context);
        GetResolvedConfigExplanationOutput output{};
        output.key = input.key;
        for (const auto& log: m_context.superposition().audit_logs()) {
            if (log.get_config_key() != input.key) {
                continue;
            }
            const auto before =
                m_context.superposition().get_config(default_config_ns(), input.key);
            const auto after =
                m_context.superposition().resolve(default_config_ns(), input.key, context);
            ResolveExplanationEntryOut entry{};
            entry.context_id = log.get_config_namespace();
            for (const auto& [key, value]: input.context) {
                entry.condition.emplace(key, serde::Value{value});
            }
            entry.override_id = std::to_string(log.get_id());
            entry.value_before = decode_or_null(before ? before->get_value() : std::string{"null"});
            entry.value_after = decode_or_null(after ? after->get_value() : std::string{"null"});
            output.timeline.push_back(std::move(entry));
        }
        return output;
    }

    [[nodiscard]] std::expected<ListAuditLogsOutput, SdkError>
    send_with(const ListAuditLogsSdkRequest& input) const
    {
        (void)input.org_id;
        (void)input.workspace_id;
        auto logs = m_context.superposition().audit_logs();
        // dimension_params validate only; the local service has no per-row dimensions.
        (void)input.dimension_params;
        std::vector<::app::services::AuditLogRecord> filtered{};
        for (const auto& log: logs) {
            if (!input.tables.empty() &&
                !matches_any(input.tables, log.get_config_namespace(), false)) {
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
            std::ranges::sort(filtered, {}, &::app::services::AuditLogRecord::get_sequence);
            std::ranges::reverse(filtered);
        } else if (input.sort_by && *input.sort_by == "asc") {
            std::ranges::sort(filtered, {}, &::app::services::AuditLogRecord::get_sequence);
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

    // ── Upstream converters (external_services/superposition.rs:235-598) ──
    [[nodiscard]] Doc document_to_value(Doc doc) const
    {
        return doc;
    }

    [[nodiscard]] Doc value_to_document(Doc value) const
    {
        return value;
    }

    [[nodiscard]] Doc doc_map_to_json(const DocMap& map) const
    {
        serde::Value::Object object{};
        for (const auto& [key, value]: map) {
            object.insert(key, value);
        }
        return Doc{std::move(object)};
    }

    [[nodiscard]] std::string datetime_to_string(const std::string& value) const
    {
        return value;
    }

    [[nodiscard]] Doc
    dependency_graph_to_value(const std::map<std::string, std::vector<std::string>>& graph) const
    {
        serde::Value::Object object{};
        for (const auto& [key, values]: graph) {
            serde::Value::Array array{};
            for (const auto& value: values) {
                array.emplace_back(Doc{value});
            }
            object.insert(key, Doc{std::move(array)});
        }
        return Doc{std::move(object)};
    }

    // dimension_type_to_value: REGULAR -> "REGULAR", LOCAL/REMOTE_COHORT(s) -> object.
    [[nodiscard]] Doc dimension_type_to_value(const std::string& encoded) const
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

    [[nodiscard]] ::app::dto::superposition_proxy::ContextResponse
    context_response_to_struct(const SdkContextResponse& context) const
    {
        ::app::dto::superposition_proxy::ContextResponse response{};
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

    [[nodiscard]] ::app::dto::superposition_proxy::DefaultConfigResponse
    default_config_response_to_struct(const SdkDefaultConfigResponse& config) const
    {
        ::app::dto::superposition_proxy::DefaultConfigResponse response{};
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

    [[nodiscard]] ::app::dto::superposition_proxy::DimensionResponse
    dimension_response_to_struct(const SdkDimensionResponse& dimension) const
    {
        ::app::dto::superposition_proxy::DimensionResponse response{};
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

    [[nodiscard]] ::app::dto::superposition_proxy::DimensionResponse
    get_dimension_output_to_struct(const GetDimensionOutput& output) const
    {
        return dimension_response_to_struct(output.dimension);
    }

    [[nodiscard]] ::app::dto::superposition_proxy::DefaultConfigResponse
    get_default_config_output_to_struct(const GetDefaultConfigOutput& output) const
    {
        return default_config_response_to_struct(output.config);
    }

    [[nodiscard]] ::app::dto::superposition_proxy::ContextResponse
    create_context_output_to_struct(const CreateContextOutput& output) const
    {
        return context_response_to_struct(output.context);
    }

    [[nodiscard]] ::app::dto::superposition_proxy::AuditLogResponse
    audit_log_full_to_struct(const SdkAuditLogFull& log) const
    {
        ::app::dto::superposition_proxy::AuditLogResponse response{};
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

    [[nodiscard]] ::app::dto::superposition_proxy::ResolveConfigExplanationResponse
    resolve_config_explanation_to_response(const GetResolvedConfigExplanationOutput& output) const
    {
        ::app::dto::superposition_proxy::ResolveConfigExplanationResponse response{};
        response.set_key(output.key);
        std::vector<::app::dto::superposition_proxy::ResolveExplanationEntry> timeline{};
        for (const auto& item: output.timeline) {
            ::app::dto::superposition_proxy::ResolveExplanationEntry entry{};
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

    [[nodiscard]] PaginatedResult list_contexts_to_response(const ListContextsOutput& output) const
    {
        PaginatedResult result{};
        result.total_pages = output.total_pages;
        result.total_items = output.total_items;
        for (const auto& item: output.data) {
            result.data.push_back(wire_of(context_response_to_struct(item)));
        }
        return result;
    }

    [[nodiscard]] PaginatedResult
    list_default_configs_to_response(const ListDefaultConfigsOutput& output) const
    {
        PaginatedResult result{};
        result.total_pages = output.total_pages;
        result.total_items = output.total_items;
        for (const auto& item: output.data) {
            result.data.push_back(wire_of(default_config_response_to_struct(item)));
        }
        return result;
    }

    [[nodiscard]] PaginatedResult
    list_dimensions_to_response(const ListDimensionsOutput& output) const
    {
        PaginatedResult result{};
        result.total_pages = output.total_pages;
        result.total_items = output.total_items;
        for (const auto& item: output.data) {
            result.data.push_back(wire_of(dimension_response_to_struct(item)));
        }
        return result;
    }

    [[nodiscard]] PaginatedResult
    list_audit_logs_to_response(const ListAuditLogsOutput& output) const
    {
        PaginatedResult result{};
        result.total_pages = output.total_pages;
        result.total_items = output.total_items;
        for (const auto& item: output.data) {
            result.data.push_back(wire_of(audit_log_full_to_struct(item)));
        }
        return result;
    }

    // ── context_put_from_request (external_services:538-579) ───────────────
    [[nodiscard]] std::optional<ContextPutRequest>
    context_put_from_request(const ContextPutRequest& request) const
    {
        // builder.build() fails when no context/override entry was supplied.
        if (request.context.empty() && request.override_values.empty()) {
            return std::nullopt;
        }
        return request;
    }

    [[nodiscard]] std::string context_put_document(const ContextPutRequest& request) const
    {
        serde::Value::Object object{};
        for (const auto& [key, value]: request.context) {
            object.insert(key, value);
        }
        return serde::Ser::encode_json(Doc{std::move(object)});
    }

    // ── Local SDK-output builders from records ────────────────────────────
    [[nodiscard]] SdkContextResponse
    context_from_record(const ::app::services::ContextRecord& record) const
    {
        SdkContextResponse response{};
        response.id = record.get_name();
        response.value = document_to_map(record.get_value());
        response.override_values =
            document_to_map(context_meta(record.get_name(), "override").value_or("{}"));
        response.override_id = context_meta(record.get_name(), "override_id")
                                   .value_or(record.get_name() + ":override");
        response.weight = std::to_string(record.get_weight());
        response.description = context_meta(record.get_name(), "description").value_or("");
        response.change_reason = context_meta(record.get_name(), "change_reason").value_or("");
        response.created_at = context_meta(record.get_name(), "created_at").value_or("");
        response.created_by = context_meta(record.get_name(), "created_by").value_or("");
        response.last_modified_at =
            context_meta(record.get_name(), "last_modified_at").value_or("");
        response.last_modified_by =
            context_meta(record.get_name(), "last_modified_by").value_or("");
        return response;
    }

    [[nodiscard]] SdkDefaultConfigResponse
    default_config_from_record(const ::app::services::ConfigRecord& record) const
    {
        SdkDefaultConfigResponse response{};
        response.key = record.get_config_key();
        response.value = to_value(record.get_value());
        response.schema =
            document_to_map(config_meta(record.get_config_key(), "schema").value_or("{}"));
        response.description =
            config_meta(record.get_config_key(), "description").value_or(record.get_context());
        response.change_reason = config_meta(record.get_config_key(), "change_reason").value_or("");
        response.value_validation_function_name =
            config_meta(record.get_config_key(), "value_validation_function_name");
        response.value_compute_function_name =
            config_meta(record.get_config_key(), "value_compute_function_name");
        response.created_at = config_meta(record.get_config_key(), "created_at").value_or("");
        response.created_by = config_meta(record.get_config_key(), "created_by").value_or("");
        response.last_modified_at =
            config_meta(record.get_config_key(), "last_modified_at").value_or("");
        response.last_modified_by =
            config_meta(record.get_config_key(), "last_modified_by").value_or("");
        return response;
    }

    [[nodiscard]] SdkDimensionResponse
    dimension_from_record(const ::app::services::ConfigRecord& record) const
    {
        const auto& name = record.get_config_key();
        SdkDimensionResponse response{};
        response.dimension = name;
        response.position = static_cast<std::int32_t>(record.get_sequence());
        response.schema = document_to_map(record.get_value());
        response.value_validation_function_name =
            dimension_meta(name, "value_validation_function_name");
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
                    for (const auto& [key, value]: *object) {
                        std::vector<std::string> values{};
                        if (const auto* array = std::get_if<serde::Value::Array>(&value.get())) {
                            for (const auto& element: *array) {
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

    [[nodiscard]] SdkAuditLogFull
    audit_log_from_record(const ::app::services::AuditLogRecord& log) const
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

    // ── Wire builders — exact upstream serde shapes ───────────────────────
    template<typename T>
    [[nodiscard]] serde::Value wire_of(const T& dto) const
    {
        const auto text = serde::Ser::encode_json(dto);
        auto parsed = serde::Ser::decode_generic("application/json", text);
        return parsed ? std::move(*parsed) : serde::Value{};
    }

    [[nodiscard]] serde::Value wire_paginated(const PaginatedResult& result) const
    {
        serde::Value::Object object{};
        object.insert(
            std::string{"total_pages"},
            serde::Value{static_cast<std::int64_t>(result.total_pages)}
        );
        object.insert(
            std::string{"total_items"},
            serde::Value{static_cast<std::int64_t>(result.total_items)}
        );
        serde::Value::Array data{};
        for (const auto& item: result.data) {
            data.push_back(item);
        }
        object.insert(std::string{"data"}, serde::Value{std::move(data)});
        return serde::Value{std::move(object)};
    }

    // ResolveConfigResponse is #[serde(transparent)] over BTreeMap<String, ResolvedConfigEntry>.
    [[nodiscard]] serde::Value wire_resolve(const serde::Value& decoded) const
    {
        return decoded;
    }

    // ── Namespaces + record meta ──────────────────────────────────────────
    [[nodiscard]] std::string_view default_config_ns() const noexcept
    {
        return "superposition.default_config";
    }

    [[nodiscard]] std::string_view dimension_ns() const noexcept
    {
        return "superposition.dimension";
    }

    [[nodiscard]] std::string_view context_meta_ns() const noexcept
    {
        return "superposition.context_meta";
    }

    [[nodiscard]] std::string_view default_config_meta_ns() const noexcept
    {
        return "superposition.default_config_meta";
    }

    [[nodiscard]] std::string_view dimension_meta_ns() const noexcept
    {
        return "superposition.dimension_meta";
    }

    [[nodiscard]] std::string_view audit_meta_ns() const noexcept
    {
        return "superposition.audit_meta";
    }

    [[nodiscard]] std::string_view proxy_config_ns() const noexcept
    {
        return "superposition_proxy";
    }

    [[nodiscard]] std::optional<std::string>
    context_meta(std::string_view name, std::string_view key) const
    {
        return m_context.config_store().get_value(
            context_meta_ns(),
            std::string{name} + "." + std::string{key}
        );
    }

    void put_context_meta(std::string_view name, std::string_view key, std::string value) const
    {
        m_context.config_store()
            .put(context_meta_ns(), std::string{name} + "." + std::string{key}, std::move(value));
    }

    [[nodiscard]] std::optional<std::string>
    config_meta(std::string_view key, std::string_view field) const
    {
        return m_context.config_store().get_value(
            default_config_meta_ns(),
            std::string{key} + "." + std::string{field}
        );
    }

    [[nodiscard]] std::optional<std::string>
    dimension_meta(std::string_view name, std::string_view field) const
    {
        return m_context.config_store().get_value(
            dimension_meta_ns(),
            std::string{name} + "." + std::string{field}
        );
    }

    [[nodiscard]] std::optional<std::string> audit_meta(std::int64_t id, std::string_view key) const
    {
        return m_context.config_store().get_value(
            audit_meta_ns(),
            std::to_string(id) + "." + std::string{key}
        );
    }

    [[nodiscard]] std::vector<std::string> default_config_keys() const
    {
        std::vector<std::string> keys{};
        for (const auto& log: m_context.superposition().audit_logs()) {
            if (log.get_config_namespace() != default_config_ns() || log.get_action() != "put") {
                continue;
            }
            if (std::ranges::find(keys, log.get_config_key()) == keys.end()) {
                keys.push_back(log.get_config_key());
            }
        }
        return keys;
    }

    [[nodiscard]] std::string
    context_document(const std::vector<std::pair<std::string, Doc>>& entries) const
    {
        serde::Value::Object object{};
        for (const auto& [key, value]: entries) {
            object.insert(key, value);
        }
        return serde::Ser::encode_json(Doc{std::move(object)});
    }

    [[nodiscard]] DocMap document_to_map(std::string_view json) const
    {
        DocMap map{};
        auto decoded = serde::Ser::decode_generic("application/json", json);
        if (!decoded) {
            return map;
        }
        if (const auto* object = std::get_if<serde::Value::Object>(&decoded->get())) {
            for (const auto& [key, value]: *object) {
                map.emplace(key, value);
            }
        }
        return map;
    }

    [[nodiscard]] Doc decode_or_null(std::string_view json) const
    {
        auto decoded = serde::Ser::decode_generic("application/json", json);
        return decoded ? std::move(*decoded) : Doc{};
    }

    [[nodiscard]] Doc to_value(std::string_view json) const
    {
        return decode_or_null(json);
    }

    [[nodiscard]] std::string
    join(const std::vector<std::string>& values, std::string_view separator) const
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
        std::vector<::app::services::ContextRecord>& records,
        std::string_view sort_on,
        std::string_view sort_by
    ) const
    {
        if (sort_on != "name" && sort_on != "value" && sort_on != "weight") {
            return;
        }
        const bool descending = sort_by == "desc";
        std::ranges::sort(
            records,
            [sort_on, descending](
                const ::app::services::ContextRecord& lhs,
                const ::app::services::ContextRecord& rhs
            )
            {
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
            }
        );
    }

    [[nodiscard]] bool matches_any(
        const std::vector<std::string>& values,
        std::string_view candidate,
        bool prefix
    ) const
    {
        for (const auto& value: values) {
            if (prefix ? candidate.starts_with(value) : candidate == value) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool contains_ci(std::string_view haystack, std::string_view needle) const
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

    [[nodiscard]] std::pair<std::size_t, std::size_t>
    slice_bounds(std::size_t total, const Pagination& paging) const noexcept
    {
        if ((paging.all && *paging.all) || total == 0U) {
            return {0U, total};
        }
        const auto limit =
            static_cast<std::size_t>(paging.count && *paging.count > 0 ? *paging.count : 20);
        const auto page =
            static_cast<std::size_t>(paging.page && *paging.page > 0 ? *paging.page : 1);
        const auto start = (page - 1U) * limit;
        if (start >= total) {
            return {total, total};
        }
        return {start, std::min(total, start + limit)};
    }

    [[nodiscard]] std::int32_t
    total_pages(std::size_t total, const Pagination& paging) const noexcept
    {
        if (total == 0U) {
            return 0;
        }
        if (paging.all && *paging.all) {
            return 1;
        }
        const auto limit =
            static_cast<std::size_t>(paging.count && *paging.count > 0 ? *paging.count : 20);
        return static_cast<std::int32_t>((total + limit - 1U) / limit);
    }

    [[nodiscard]] bool is_json_document(std::string_view json) const
    {
        return serde::Ser::decode_generic("application/json", json).has_value();
    }

    // ApiEventMetric analogue: paired named flow/counter on every success.
    void track_flow(std::string_view flow, std::string_view counter) const
    {
        m_context.metrics().record_flow(flow);
        m_context.metrics().counter(counter).increment();
        m_context.metrics().record_flow(std::string{"api_event."} + std::string{flow});
        m_context.metrics().counter(std::string{"api_event."} + std::string{counter}).increment();
    }

private:
    [[nodiscard]] std::optional<ScopingDimension>
    from_context_key(std::string_view key) const noexcept
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
    [[nodiscard]] std::optional<ScopingDimension>
    from_dimension_param(std::string_view key) const noexcept
    {
        if (!key.starts_with("dimension[") || !key.ends_with(']') || key.size() <= 11U) {
            return std::nullopt;
        }
        return from_context_key(key.substr(10U, key.size() - 11U));
    }

    [[nodiscard]] std::string_view dimension_name(ScopingDimension dimension) const noexcept
    {
        switch (dimension) {
            case ScopingDimension::OrganizationId:
                return "organization_id";
            case ScopingDimension::MerchantId:
                return "merchant_id";
            case ScopingDimension::ProfileId:
                return "profile_id";
            case ScopingDimension::ProviderMerchantId:
                return "provider_merchant_id";
            case ScopingDimension::ProcessorMerchantId:
                return "processor_merchant_id";
        }
        return {};
    }

    [[nodiscard]] std::string_view expected_value(
        ScopingDimension dimension,
        const ::core::behavior::CallContext& ctx
    ) const noexcept
    {
        switch (dimension) {
            case ScopingDimension::OrganizationId:
                return ctx.org_id;
            case ScopingDimension::ProfileId:
                return ctx.profile_id;
            case ScopingDimension::MerchantId:
            case ScopingDimension::ProviderMerchantId:
            case ScopingDimension::ProcessorMerchantId:
                return ctx.merchant_id;
        }
        return {};
    }

    ::app::context::AppContext& m_context;
    SuperpositionProxyInputSupport m_inputs;
};

} // namespace app::handlers::superposition_proxy
