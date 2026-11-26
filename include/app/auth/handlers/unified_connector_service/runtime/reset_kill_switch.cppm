export module app_auth:unified_connector_service_runtime_reset_kill_switch;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_context_core;
import app_http;
import app_metrics;
import app_services;

import :unified_connector_service_input;
import :unified_connector_service_result;
import :unified_connector_service_runtime_common;

export namespace app::handlers::unified_connector_service {

class ResetKillSwitchRuntime
{
public:
    explicit ResetKillSwitchRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream kill_switch::reset (kill_switch.rs:543-577). Auth and scope
    // resolution stay in the handler; everything below is the old core block.
    [[nodiscard]] std::expected<ResetKillSwitchResult, core::error::AppError>
    reset(const ResetKillSwitchInput& input) const
    {
        const auto& resolved = input.scope;

        // kill_switch.rs:553-559: connection failure -> InternalServerError.
        if (!m_context.cache().is_open()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "Failed to get a redis connection to clear the UCS kill switch"
            )};
        }

        // kill_switch.rs:545-552: shadow counter lives in the global store under
        // its listed key; primary counters live in the tenant store under the
        // `ucs_kill_switch_<scope>` hash key. Both hash fields are deleted by the
        // single DEL (the local substitute erases the one cache entry that
        // carries both fields).
        if (!m_support.delete_key(resolved)) {
            // kill_switch.rs:570-575: DelReply::KeyNotDeleted -> GenericNotFound.
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "No UCS kill switch counter found for scope " + resolved.scope
            )};
        }

        // Flow::UnifiedConnectorServiceKillSwitchReset instrumentation
        // (routes/unified_connector_service.rs:36,42).
        m_context.metrics().record_flow("UnifiedConnectorServiceKillSwitchReset");
        m_context.metrics().counter("unified_connector_service.kill_switch_reset").increment();

        // kill_switch.rs:562-569: ApplicationResponse::StatusOk, no JSON body.
        return ResetKillSwitchResult{};
    }

private:
    ::app::context::AppContext& m_context;
    UnifiedConnectorServiceRuntimeSupport m_support;
};

} // namespace app::handlers::unified_connector_service
