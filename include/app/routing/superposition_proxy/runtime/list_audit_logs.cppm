export module app_routing:superposition_proxy_runtime_list_audit_logs;

import std;
import core_serde;
import core_error;
import app_context;

import :superposition_proxy_input;
import :superposition_proxy_runtime_common;

export namespace app::handlers::superposition_proxy {

class ListAuditLogsRuntime
{
public:
    explicit ListAuditLogsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    list_audit_logs(const ListAuditLogsHandlerInput& input) const
    {
        auto wire = m_support.finish(
            m_support.execute(input.request, input.auth, input.org_id, input.workspace_id),
            "SuperpositionListAuditLogs",
            "superposition.list_audit_logs"
        );
        if (!wire) {
            return std::unexpected{wire.error()};
        }
        return std::move(*wire);
    }

private:
    ::app::context::AppContext& m_context;
    SuperpositionProxyRuntimeSupport m_support;
};

} // namespace app::handlers::superposition_proxy
