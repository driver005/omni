export module app_auth:unified_connector_service_runtime;

import std;
import core_error;
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

    [[nodiscard]] std::expected<KillSwitchStatusResult, core::error::AppError>
    status(const KillSwitchStatusInput& input) const
    {
        return m_kill_switch_status.status(input);
    }

    [[nodiscard]] std::expected<ResetKillSwitchResult, core::error::AppError>
    reset(const ResetKillSwitchInput& input) const
    {
        return m_reset_kill_switch.reset(input);
    }

private:
    ::app::context::AppContext& m_context;
    KillSwitchStatusRuntime m_kill_switch_status;
    ResetKillSwitchRuntime m_reset_kill_switch;
};

} // namespace app::handlers::unified_connector_service
