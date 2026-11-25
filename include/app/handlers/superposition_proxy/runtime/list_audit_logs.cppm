export module app_handlers_superposition_proxy:runtime_list_audit_logs;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::superposition_proxy {

class ListAuditLogsRuntime
{
public:
    explicit ListAuditLogsRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ListAuditLogsResult, core::error::AppError>
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
        return ListAuditLogsResult{std::move(*wire)};
    }

private:
    app::context::AppContext& m_context;
    SuperpositionProxyRuntimeSupport m_support;
};

} // namespace app::handlers::superposition_proxy
