export module app_handlers_superposition_proxy:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_list_contexts;
import :runtime_create_context;
import :runtime_list_default_configs;
import :runtime_get_default_config;
import :runtime_list_dimensions;
import :runtime_get_dimension;
import :runtime_resolve_detailed_config;
import :runtime_resolve_config_explanation;
import :runtime_list_audit_logs;

export namespace app::handlers::superposition_proxy {

class SuperpositionProxyRuntime
{
public:
    explicit SuperpositionProxyRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_list_contexts{context},
        m_create_context{context},
        m_list_default_configs{context},
        m_get_default_config{context},
        m_list_dimensions{context},
        m_get_dimension{context},
        m_resolve_detailed_config{context},
        m_resolve_config_explanation{context},
        m_list_audit_logs{context}
    {
    }

    [[nodiscard]] std::expected<ListContextsResult, core::error::AppError>
    list_contexts(const ListContextsHandlerInput& input) const
    {
        return m_list_contexts.list_contexts(input);
    }

    [[nodiscard]] std::expected<CreateContextResult, core::error::AppError>
    create_context(const CreateContextHandlerInput& input) const
    {
        return m_create_context.create_context(input);
    }

    [[nodiscard]] std::expected<ListDefaultConfigsResult, core::error::AppError>
    list_default_configs(const ListDefaultConfigsHandlerInput& input) const
    {
        return m_list_default_configs.list_default_configs(input);
    }

    [[nodiscard]] std::expected<GetDefaultConfigResult, core::error::AppError>
    get_default_config(const GetDefaultConfigHandlerInput& input) const
    {
        return m_get_default_config.get_default_config(input);
    }

    [[nodiscard]] std::expected<ListDimensionsResult, core::error::AppError>
    list_dimensions(const ListDimensionsHandlerInput& input) const
    {
        return m_list_dimensions.list_dimensions(input);
    }

    [[nodiscard]] std::expected<GetDimensionResult, core::error::AppError>
    get_dimension(const GetDimensionHandlerInput& input) const
    {
        return m_get_dimension.get_dimension(input);
    }

    [[nodiscard]] std::expected<ResolveDetailedConfigResult, core::error::AppError>
    resolve_detailed_config(const ResolveDetailedConfigHandlerInput& input) const
    {
        return m_resolve_detailed_config.resolve_detailed_config(input);
    }

    [[nodiscard]] std::expected<ResolveConfigExplanationResult, core::error::AppError>
    resolve_config_explanation(const ResolveConfigExplanationHandlerInput& input) const
    {
        return m_resolve_config_explanation.resolve_config_explanation(input);
    }

    [[nodiscard]] std::expected<ListAuditLogsResult, core::error::AppError>
    list_audit_logs(const ListAuditLogsHandlerInput& input) const
    {
        return m_list_audit_logs.list_audit_logs(input);
    }

private:
    app::context::AppContext& m_context;
    ListContextsRuntime m_list_contexts;
    CreateContextRuntime m_create_context;
    ListDefaultConfigsRuntime m_list_default_configs;
    GetDefaultConfigRuntime m_get_default_config;
    ListDimensionsRuntime m_list_dimensions;
    GetDimensionRuntime m_get_dimension;
    ResolveDetailedConfigRuntime m_resolve_detailed_config;
    ResolveConfigExplanationRuntime m_resolve_config_explanation;
    ListAuditLogsRuntime m_list_audit_logs;
};

} // namespace app::handlers::superposition_proxy
