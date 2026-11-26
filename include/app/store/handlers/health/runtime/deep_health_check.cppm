export module app_store:health_runtime_deep_health_check;

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
import app_connector;
import app_events;
import app_context_core;
import app_workflow;
import app_keystore;

import :health_input;
import :health_result;
import :health_runtime_common;

export namespace app::handlers::health {

class DeepHealthCheckRuntime
{
public:
    explicit DeepHealthCheckRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // Sequence and component names mirror deep_health_check_func exactly
    // (routes/health.rs:51-166). Any failure maps to the upstream
    // ApiErrorResponse::HealthCheckError wire shape and returns early.
    [[nodiscard]] std::expected<DeepHealthCheckResult, core::error::AppError>
    deep_health_check(const DeepHealthCheckInput& input) const
    {
        (void)input;

        // HEALTH_METRIC.add(1, &[]) (routes/health.rs:28) — local metrics substitute.
        m_context.metrics().record_flow("deep_health_check");
        m_context.metrics().counter("health").increment();

        const HealthStateProjection project{};
        using State = ::app::dto::health_check::HealthState;

        // Database: store.health_check_db() (core/health_check.rs:51-55).
        const auto database = m_support.probe_database();
        if (database.state == State::Error) {
            return std::unexpected{m_support.health_check_error("Database", database.error)};
        }

        // Redis: set/get/delete cycle (core/health_check.rs:57-85).
        const auto redis = m_support.probe_redis();
        if (redis.state == State::Error) {
            return std::unexpected{m_support.health_check_error("Redis", redis.error)};
        }

        // Locker/vault: real keystore probe when the locker is active; the
        // disabled/mocked path returns NotApplicable (core/health_check.rs:87-105).
        const auto vault = m_support.probe_vault();
        if (vault.state == State::Error) {
            return std::unexpected{m_support.health_check_error("Locker", vault.error)};
        }

        // [olap] Analytics provider deep health check (core/health_check.rs:107-144).
        State analytics_state = State::NotApplicable;
        if constexpr (kOlap) {
            const auto analytics = m_support.probe_analytics();
            if (analytics.state == State::Error) {
                return std::unexpected{m_support.health_check_error("Analytics", analytics.error)};
            }
            analytics_state = analytics.state;
        }

        // [dynamic_routing] gRPC HealthCheckMap (core/health_check.rs:181-195).
        std::unordered_map<std::string, bool> grpc_map{};
        if constexpr (kDynamicRouting) {
            auto probed = m_support.probe_grpc_map();
            if (!probed.has_value()) {
                return std::unexpected{
                    m_support.health_check_error("gRPC services", probed.error())
                };
            }
            grpc_map = std::move(*probed);
        }

        // [dynamic_routing] Decision Engine (core/health_check.rs:197-221).
        State decision_state = State::NotApplicable;
        if constexpr (kDynamicRouting) {
            const auto decision = m_support.probe_decision_engine();
            if (decision.state == State::Error) {
                return std::unexpected{
                    m_support.health_check_error("Decision Engine service", decision.error)
                };
            }
            decision_state = decision.state;
        }

        // [olap] OpenSearch (core/health_check.rs:146-159).
        State opensearch_state = State::NotApplicable;
        if constexpr (kOlap) {
            const auto opensearch = m_support.probe_opensearch();
            if (opensearch.state == State::Error) {
                return std::unexpected{
                    m_support.health_check_error("Opensearch", opensearch.error)
                };
            }
            opensearch_state = opensearch.state;
        }

        // Outgoing Request (core/health_check.rs:161-179).
        const auto outgoing = m_support.probe_outgoing();
        if (outgoing.state == State::Error) {
            return std::unexpected{
                m_support.health_check_error("Outgoing Request", outgoing.error)
            };
        }

        // Unified Connector Service (core/health_check.rs:223-285).
        const auto ucs = m_support.probe_unified_connector_service();
        if (ucs.state == State::Error) {
            return std::unexpected{
                m_support.health_check_error("Unified Connector Service", ucs.error)
            };
        }

        // Exact HealthState -> bool / Option<bool> conversions
        // (api_models/health_check.rs:45-61), with feature-gated field presence
        // and `skip_serializing_if = "Option::is_none"` on the two Option fields.
        const serde::Value body = m_support.encode_health_response(
            project.as_bool(database.state),
            project.as_bool(redis.state),
            project.as_optional(vault.state),
            project.as_bool(analytics_state),
            project.as_bool(opensearch_state),
            project.as_bool(outgoing.state),
            grpc_map,
            project.as_bool(decision_state),
            project.as_optional(ucs.state)
        );

        return DeepHealthCheckResult{body};
    }

private:
    ::app::context::AppContext& m_context;
    HealthRuntimeSupport m_support;
};

} // namespace app::handlers::health
