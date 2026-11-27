export module app_routing:superposition_proxy_runtime;

import std;
import core_serde;
import core_error;
import app_context;
import core_behavior;

import :superposition_proxy_input;
import :superposition_proxy_runtime_common;
import :superposition_proxy_runtime_list_contexts;
import :superposition_proxy_runtime_create_context;
import :superposition_proxy_runtime_list_default_configs;
import :superposition_proxy_runtime_get_default_config;
import :superposition_proxy_runtime_list_dimensions;
import :superposition_proxy_runtime_get_dimension;
import :superposition_proxy_runtime_resolve_detailed_config;
import :superposition_proxy_runtime_resolve_config_explanation;
import :superposition_proxy_runtime_list_audit_logs;

export namespace app::handlers::superposition_proxy {

class SuperpositionProxyRuntime
{
public:
    explicit SuperpositionProxyRuntime(::app::context::AppContext& context) noexcept :
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

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_contexts(const ListContextsQuery& request, const ::core::behavior::CallContext& ctx) const
    {
        return m_list_contexts.list_contexts(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    create_context(const ContextPutRequest& request, const ::core::behavior::CallContext& ctx) const
    {
        return m_create_context.create_context(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> list_default_configs(
        const ListDefaultConfigsQuery& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_list_default_configs.list_default_configs(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> get_default_config(
        const GetDefaultConfigRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_get_default_config.get_default_config(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> list_dimensions(
        const ListDimensionsQuery& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_list_dimensions.list_dimensions(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> get_dimension(
        const GetDimensionRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_get_dimension.get_dimension(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> resolve_detailed_config(
        const ResolveDetailedConfigRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_resolve_detailed_config.resolve_detailed_config(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> resolve_config_explanation(
        const ResolveConfigExplanationRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_resolve_config_explanation.resolve_config_explanation(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> list_audit_logs(
        const ListAuditLogsQuery& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_list_audit_logs.list_audit_logs(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
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
