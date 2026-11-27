export module app_auth:unified_connector_service_runtime;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;

import :unified_connector_service_input;
import :unified_connector_service_result;
import :unified_connector_service_runtime_common;
import :unified_connector_service_runtime_kill_switch_status;
import :unified_connector_service_runtime_reset_kill_switch;

export namespace app::handlers::unified_connector_service {

class UnifiedConnectorServiceRuntime
{
public:
    explicit UnifiedConnectorServiceRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_kill_switch_status{context},
        m_reset_kill_switch{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<KillSwitchStatusResponse>
    status(const core::behavior::CallContext& ctx) const
    {
        return m_kill_switch_status.status(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    reset(const core::behavior::CallContext& ctx) const
    {
        return m_reset_kill_switch.reset(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    KillSwitchStatusRuntime m_kill_switch_status;
    ResetKillSwitchRuntime m_reset_kill_switch;
};

} // namespace app::handlers::unified_connector_service
