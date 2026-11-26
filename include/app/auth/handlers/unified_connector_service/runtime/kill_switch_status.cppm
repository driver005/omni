export module app_auth:unified_connector_service_runtime_kill_switch_status;

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

class KillSwitchStatusRuntime
{
public:
    explicit KillSwitchStatusRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Upstream kill_switch::trip_status (kill_switch.rs:580-671). Auth and scope
    // resolution stay in the handler; everything below is the old core block.
    [[nodiscard]] std::expected<KillSwitchStatusResult, core::error::AppError>
    status(const KillSwitchStatusInput& input) const
    {
        const auto& resolved = input.scope;

        // A closed local cache is the in-process equivalent of a failed
        // `get_redis_conn`. Upstream fails the request with InternalServerError
        // and the matching attach_printable message (kill_switch.rs:585-588,
        // :604-608, :646-650).
        if (!m_context.cache().is_open()) {
            const auto message = resolved.shadow
                                     ? std::string{"Failed to read the shadow kill switch counter"}
                                     : std::string{"Failed to read the UCS kill switch counter"};
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::INTERNAL_ERROR, message)
            };
        }

        std::uint64_t counter{0};
        std::optional<std::uint64_t> threshold{};
        std::uint64_t decline_counter{0};
        std::optional<std::uint64_t> decline_threshold{};
        bool tripped{false};

        if (resolved.shadow) {
            // kill_switch.rs:584-599: global hash `ucs_shadow_killed`, field
            // `counter`; threshold from shadow::threshold; declines fixed at 0.
            counter = m_support.read_hash_counter(resolved, kCounterField);
            const auto configured = m_support.shadow_threshold();
            threshold = configured;
            tripped = m_support.exceeds_threshold(counter, configured);
        } else {
            // kill_switch.rs:602-659: tenant hash `ucs_kill_switch_<scope>`.
            counter = m_support.read_hash_counter(resolved, kCounterField);

            const auto config = m_support.load_rollout_config(resolved.scope);
            if (config) {
                // RolloutConfig::kill_switch_threshold defaults to 1 when present
                // but unset (helpers.rs:2696-2697,2719-2721).
                threshold = config->get_kill_switch_threshold().value_or(1);
                decline_threshold = config->get_connector_decline_threshold();
            }

            // kill_switch.rs:645-653: only read when a threshold can use it.
            if (decline_threshold) {
                decline_counter = m_support.read_hash_counter(resolved, kDeclineField);
            }

            // kill_switch.rs:655-659: no threshold => any count trips, exactly
            // as upstream (not `counter >= 1`, which differs only when 0).
            tripped =
                (threshold ? m_support.exceeds_threshold(counter, *threshold) : counter > 0) ||
                (decline_threshold
                     ? m_support.exceeds_threshold(decline_counter, *decline_threshold)
                     : false);
        }

        // Flow::UnifiedConnectorServiceKillSwitchStatus instrumentation
        // (routes/unified_connector_service.rs:14,20).
        m_context.metrics().record_flow("UnifiedConnectorServiceKillSwitchStatus");
        m_context.metrics().counter("unified_connector_service.kill_switch_status").increment();

        // Upstream KillSwitchStatusResponse (kill_switch.rs:524-538): serde emits
        // Option<u64> thresholds as JSON null when absent. Field order matches.
        KillSwitchStatusResult response{};
        response.set_rollout_scope(resolved.shadow ? resolved.listed_key : resolved.scope);
        response.set_counter(counter);
        response.set_threshold(threshold);
        response.set_decline_counter(decline_counter);
        response.set_connector_decline_threshold(decline_threshold);
        response.set_tripped(tripped);
        return response;
    }

private:
    ::app::context::AppContext& m_context;
    UnifiedConnectorServiceRuntimeSupport m_support;
};

} // namespace app::handlers::unified_connector_service
