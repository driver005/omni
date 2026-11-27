export module app_store:health_runtime_common;

import std;
import interfaces;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_context_core;
import app_metrics;
import app_connector;
import app_events;
import app_context_core;
import app_workflow;
import app_keystore;

export namespace app::handlers::health {

/**
 * Local substitutes for the upstream Cargo features that shape the deep health
 * response (`api_models/health_check.rs:8-17`, `routes/app.rs:812-829`). This port
 * has no Cargo `#[cfg]`, so these constexpr gates are the exact local equivalent:
 * they decide both which fields are emitted and which probes run. Both feature
 * sets are compiled in here, matching an upstream release build.
 */
inline constexpr bool kOlap = true;           // analytics + opensearch fields
inline constexpr bool kDynamicRouting = true; // grpc_health_check + decision_engine fields

/** One probe result: state plus the error text the probe recorded on failure. */
struct HealthProbeOutcome
{
    ::app::dto::health_check::HealthState state{
        ::app::dto::health_check::HealthState::NotApplicable
    };
    std::string error{};
};

/**
 * @brief Local projection of upstream HealthState to wire types.
 * Running -> true / Some(true); Error -> false / Some(false);
 * NotApplicable -> false / None (api_models/health_check.rs:39-61).
 */
class HealthStateProjection
{
public:
    [[nodiscard]] bool as_bool(::app::dto::health_check::HealthState state) const noexcept
    {
        using State = ::app::dto::health_check::HealthState;
        return state == State::Running;
    }

    [[nodiscard]] std::optional<bool>
    as_optional(::app::dto::health_check::HealthState state) const noexcept
    {
        using State = ::app::dto::health_check::HealthState;
        if (state == State::NotApplicable) {
            return std::nullopt;
        }
        return state == State::Running;
    }
};

/**
 * @brief Shared probes and wire encoders for the health sub-runtimes.
 *
 * The old facade kept a mutable m_probe_error; here each probe returns the
 * error text it recorded, so the runtime methods stay const and free of HTTP.
 */
class HealthRuntimeSupport
{
public:
    explicit HealthRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ───────────────────────── Local health probes ─────────────────────────

    [[nodiscard]] HealthProbeOutcome probe_database() const
    {
        using State = ::app::dto::health_check::HealthState;
        try {
            if (!m_context.database().is_open()) {
                return {State::Error, "database is not open"};
            }
            (void)m_context.repository().list_payments();
            return {State::Running, {}};
        } catch (const std::exception& error) {
            return {State::Error, error.what()};
        } catch (...) {
            return {State::Error, "unknown database error"};
        }
    }

    [[nodiscard]] HealthProbeOutcome probe_redis() const
    {
        using State = ::app::dto::health_check::HealthState;
        const std::string key{"health.redis.test_key"};
        try {
            m_context.cache().put(
                key,
                core::cache::to_cache_value(std::string{"test_value"}),
                core::cache::Ttl{}.seconds(30)
            );
            const auto probe = m_context.cache().get(key);
            m_context.cache().erase(key);
            if (!probe.hit || probe.value_or<std::string>(std::string{}) != "test_value") {
                return {State::Error, "redis set/get round-trip mismatch"};
            }
            return {State::Running, {}};
        } catch (const std::exception& error) {
            return {State::Error, error.what()};
        } catch (...) {
            return {State::Error, "unknown redis error"};
        }
    }

    [[nodiscard]] HealthProbeOutcome probe_vault() const
    {
        using State = ::app::dto::health_check::HealthState;
        // Upstream gate: locker.locker_enabled && !locker.mock_locker. The local
        // ConfigStore holds the equivalent flags; absent values default to an
        // active locker so the keystore is genuinely probed by default.
        const auto enabled = m_context.config_store().get_value("health_check", "locker_enabled");
        const auto mocked = m_context.config_store().get_value("health_check", "mock_locker");
        const bool locker_active = (!enabled.has_value() || *enabled == "true") &&
                                   (!mocked.has_value() || *mocked != "true");
        if (!locker_active) {
            return {State::NotApplicable, {}};
        }
        try {
            // The local keystore is the vault/locker substitute; find_record is
            // backed by core::db::Database, so a failed store surfaces as Error.
            (void)m_context.key_store().find_record("health_check", "vault_probe");
            return {State::Running, {}};
        } catch (const std::exception& error) {
            return {State::Error, error.what()};
        } catch (...) {
            return {State::Error, "unknown locker error"};
        }
    }

    [[nodiscard]] HealthProbeOutcome probe_analytics() const
    {
        using State = ::app::dto::health_check::HealthState;
        // Upstream analytics providers are databases (Sqlx/Clickhouse). The
        // repository read over a distinct table is the real local DB substitute.
        try {
            (void)m_context.repository().list_refunds();
            return {State::Running, {}};
        } catch (const std::exception& error) {
            return {State::Error, error.what()};
        } catch (...) {
            return {State::Error, "unknown analytics error"};
        }
    }

    [[nodiscard]] HealthProbeOutcome probe_opensearch() const
    {
        using State = ::app::dto::health_check::HealthState;
        // Upstream: if an opensearch client is configured, deep_health_check it,
        // else NotApplicable. The local event bus is a DB-backed searchable store;
        // the configured-client flag is held in ConfigStore.
        const auto configured =
            m_context.config_store().get_value("health_check", "opensearch_enabled");
        if (configured.has_value() && *configured == "false") {
            return {State::NotApplicable, {}};
        }
        try {
            (void)m_context.events().find_by_event_id("health_check_probe_missing");
            return {State::Running, {}};
        } catch (const std::exception& error) {
            return {State::Error, error.what()};
        } catch (...) {
            return {State::Error, "unknown opensearch error"};
        }
    }

    [[nodiscard]] std::expected<std::unordered_map<std::string, bool>, std::string>
    probe_grpc_map() const
    {
        if constexpr (!kDynamicRouting) {
            return std::unordered_map<std::string, bool>{};
        }
        try {
            // Upstream perform_health_check returns HealthCheckServices -> bool.
            // ConfigStore records in the grpc_health_check namespace are the local
            // substitute for the configured gRPC service health results.
            std::unordered_map<std::string, bool> result{};
            for (const auto& record: m_context.config_store().list("grpc_health_check")) {
                result[record.get_config_key()] =
                    (record.get_value() == "true" || record.get_value() == "SERVING");
            }
            return result;
        } catch (const std::exception& error) {
            return std::unexpected{std::string{error.what()}};
        } catch (...) {
            return std::unexpected{std::string{"unknown gRPC health check error"}};
        }
    }

    [[nodiscard]] HealthProbeOutcome probe_decision_engine() const
    {
        using State = ::app::dto::health_check::HealthState;
        if constexpr (!kDynamicRouting) {
            return {State::NotApplicable, {}};
        }
        // Upstream gates on open_router.dynamic_routing_enabled; the local
        // constexpr gate is on, so evaluate a trivial program through the real
        // in-process DecisionEngine to prove the service is functional.
        constexpr std::string_view program =
            R"({"rules":[],"default_action":"allow","default_connector":""})";
        constexpr std::string_view context_json =
            R"({"amount":"1","currency":"USD","payment_method":"card","country":"US","connector":""})";
        try {
            const auto result = m_context.decision_engine().evaluate_checked(program, context_json);
            if (!result.has_value()) {
                return {State::Error, result.error().get_message()};
            }
            return {State::Running, {}};
        } catch (const std::exception& error) {
            return {State::Error, error.what()};
        } catch (...) {
            return {State::Error, "unknown decision engine error"};
        }
    }

    [[nodiscard]] HealthProbeOutcome probe_outgoing() const
    {
        using State = ::app::dto::health_check::HealthState;
        try {
            const auto connector = m_context.connector_router().select("health_check");
            if (!connector) {
                return {State::NotApplicable, {}};
            }
            const auto response = connector->sync(
                ::app::connector::ConnectorRequestBuilder{}
                    .operation("health_check")
                    .merchant_id("health_check")
                    .raw_payload("health_check")()
            );
            if (response.get_status().empty()) {
                return {State::Error, "outgoing connector probe returned no status"};
            }
            return {State::Running, {}};
        } catch (const std::exception& error) {
            return {State::Error, error.what()};
        } catch (...) {
            return {State::Error, "unknown outgoing probe error"};
        }
    }

    [[nodiscard]] HealthProbeOutcome probe_unified_connector_service() const
    {
        using State = ::app::dto::health_check::HealthState;
        // Superposition/ConfigStore is the local substitute for the configured
        // UCS gRPC client. Absent record == client not configured == NotApplicable.
        const auto client =
            m_context.superposition().get_config("grpc_clients", "unified_connector_service");
        if (!client.has_value()) {
            return {State::NotApplicable, {}};
        }
        if constexpr (!kDynamicRouting) {
            // Upstream non-dynamic_routing fallback: configured client => Running.
            return {State::Running, {}};
        }
        const auto& value = client->get_value();
        if (value == "NOT_SERVING") {
            return {State::Error, "Unified Connector Service reported NOT_SERVING"};
        }
        // SERVING (or any configured client state) is healthy.
        return {State::Running, {}};
    }

    // ───────────────────────────── Wire encoding ───────────────────────────────

    /**
     * Builds the RouterHealthCheckResponse document. Field order matches
     * api_models/health_check.rs:3-18. olap/dynamic_routing fields are gated by
     * the local constexpr feature flags; the two Option fields are omitted when
     * None, mirroring `#[serde(skip_serializing_if = "Option::is_none")]`.
     */
    [[nodiscard]] serde::Value encode_health_response(
        bool database,
        bool redis,
        std::optional<bool> vault,
        bool analytics,
        bool opensearch,
        bool outgoing,
        const std::unordered_map<std::string, bool>& grpc_map,
        bool decision_engine,
        std::optional<bool> unified_connector_service
    ) const
    {
        serde::Value::Object document{};
        document["database"] = serde::Value{database};
        document["redis"] = serde::Value{redis};
        if (vault.has_value()) {
            document["vault"] = serde::Value{*vault};
        }
        if constexpr (kOlap) {
            document["analytics"] = serde::Value{analytics};
            document["opensearch"] = serde::Value{opensearch};
        }
        document["outgoing_request"] = serde::Value{outgoing};
        if constexpr (kDynamicRouting) {
            serde::Value::Object grpc{};
            for (const auto& [service, healthy]: grpc_map) {
                grpc[service] = serde::Value{healthy};
            }
            document["grpc_health_check"] = serde::Value{std::move(grpc)};
            document["decision_engine"] = serde::Value{decision_engine};
        }
        if (unified_connector_service.has_value()) {
            document["unified_connector_service"] = serde::Value{*unified_connector_service};
        }
        return serde::Value{std::move(document)};
    }

    /**
     * ApiErrorResponse::HealthCheckError is converted upstream to
     * ApiErrorResponse::InternalServerError with code HE_00 and message
     * "{component} health check failed with error: {message}"
     * (hyperswitch_domain_models/errors/api_error_response.rs:60-64,425-427).
     * The message is the real human-readable text; the upstream code lives in
     * kind_name so Ser serializes both as typed AppError fields.
     */
    [[nodiscard]] core::error::AppError
    health_check_error(std::string_view component, std::string_view message) const
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INTERNAL_ERROR,
            std::format("{} health check failed with error: {}", component, message)
        );
        error.set_kind_name("HE_00");
        return error;
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::health
