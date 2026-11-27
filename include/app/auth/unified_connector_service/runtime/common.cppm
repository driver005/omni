export module app_auth:unified_connector_service_runtime_common;

import std;
import interfaces;
import core_behavior;
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

export namespace app::handlers::unified_connector_service {

/** @brief Resolve the final path segment into the upstream listed key/scope pair. */
[[nodiscard]] inline ScopeInfo resolve_scope(const core::behavior::CallContext& ctx)
{
    ScopeInfo result{};
    if (ctx.path_segments.empty()) {
        return result;
    }
    result.listed_key = ctx.path_segments.back();
    if (result.listed_key.empty()) {
        return result;
    }
    result.scope = result.listed_key;
    const auto prefix = std::string{kKillSwitchPrefix} + "_";
    if (const auto pos = result.scope.find(prefix); pos != std::string::npos) {
        result.scope.erase(0, pos + prefix.size());
    }
    result.shadow = result.listed_key == kShadowKey;
    return result;
}

/** @brief Typed RolloutConfig subset consumed by trip_status. */
class RolloutConfigView
{
public:
    void set_kill_switch_threshold(std::optional<std::uint64_t> value) noexcept
    {
        m_kill_switch_threshold = value;
    }

    [[nodiscard]] const std::optional<std::uint64_t>& get_kill_switch_threshold() const noexcept
    {
        return m_kill_switch_threshold;
    }

    void set_connector_decline_threshold(std::optional<std::uint64_t> value) noexcept
    {
        m_connector_decline_threshold = value;
    }

    [[nodiscard]] const std::optional<std::uint64_t>&
    get_connector_decline_threshold() const noexcept
    {
        return m_connector_decline_threshold;
    }

private:
    // Upstream defaults applied at the call site: kill_switch_threshold -> 1
    // (helpers.rs:2719-2721), connector_decline_threshold -> None.
    std::optional<std::uint64_t> m_kill_switch_threshold{};
    std::optional<std::uint64_t> m_connector_decline_threshold{};
};

/**
 * @brief Shared kill-switch business helpers for both sub-runtimes.
 *
 * Redis (external infra) is replaced by the injected core::cache in-process, but
 * every observable behaviour is preserved: two independent store namespaces stand
 * in for `state.store` (tenant) and `state.global_store` (global); each Redis hash
 * is one cache entry whose JSON value carries the `counter` and
 * `counter_connector_decline` hash fields; expiry carries the HINCRBY/EXPIRE TTL;
 * `DEL` maps `KeyDeleted`/`KeyNotDeleted` exactly.
 */
class UnifiedConnectorServiceRuntimeSupport
{
public:
    explicit UnifiedConnectorServiceRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /** @brief `count >= threshold` (kill_switch.rs:31-34). */
    [[nodiscard]] bool
    exceeds_threshold(std::uint64_t count, std::uint64_t threshold) const noexcept
    {
        return count >= threshold;
    }

    /** @brief `counter_key(rollout_scope)` (kill_switch.rs:47-50). */
    [[nodiscard]] std::string redis_key(const ScopeInfo& scope) const
    {
        if (scope.shadow) {
            return std::string{kShadowKey};
        }
        return std::string{kKillSwitchPrefix} + "_" + scope.scope;
    }

    /**
     * @brief tenant/global store split (kill_switch.rs:545-552,915-924).
     * Real Redis uses two connections; the single local cache is partitioned
     * into two namespaces so the stores stay independent.
     */
    [[nodiscard]] std::string cache_key(const ScopeInfo& scope) const
    {
        return std::string{"unified_connector_service.redis."} +
               (scope.shadow ? "global:" : "tenant:") + redis_key(scope);
    }

    [[nodiscard]] std::optional<core::cache::CacheEntry> find_entry(std::string_view key) const
    {
        core::cache::CacheQuerySpec spec;
        spec.prefix = std::string{key};
        for (const auto& entry: m_context.cache().execute(spec)) {
            if (entry.key == key) {
                return entry;
            }
        }
        return std::nullopt;
    }

    /** @brief Internal JSON encoding of one Redis hash value. */
    [[nodiscard]] std::string encode_hash(std::uint64_t count, std::uint64_t decline) const
    {
        return std::format(R"({{"counter":{},"counter_connector_decline":{}}})", count, decline);
    }

    [[nodiscard]] std::uint64_t
    decode_hash_field(const std::string& json, std::string_view field) const
    {
        const auto key = std::string{"\""} + std::string{field} + "\":";
        const auto pos = json.find(key);
        if (pos == std::string::npos) {
            return 0;
        }
        std::uint64_t value{};
        const char* begin = json.data() + pos + key.size();
        const char* end = json.data() + json.size();
        const auto [ptr, ec] = std::from_chars(begin, end, value);
        if (ec != std::errc{} || ptr == begin) {
            return 0;
        }
        return value;
    }

    /** @brief `read_hash_counter` (kill_switch.rs:186-196) over the local cache. */
    [[nodiscard]] std::uint64_t
    read_hash_counter(const ScopeInfo& scope, std::string_view field) const
    {
        const auto entry = find_entry(cache_key(scope));
        if (!entry || entry->expired()) {
            // Missing key or field -> 0 (kill_switch.rs:192-195).
            return 0;
        }
        return decode_hash_field(
            core::cache::value_as<std::string>(entry->value, std::string{}),
            field
        );
    }

    /**
     * @brief Port of `write_hash_counter` (kill_switch.rs:499-521):
     * HINCRBY one field then best-effort EXPIRE. Used by the enforcement hooks
     * that feed the counters; the read/reset endpoints share this hash layout.
     */
    [[maybe_unused]] std::uint64_t write_hash_counter(
        const ScopeInfo& scope,
        std::string_view field,
        std::int64_t ttl_seconds
    ) const
    {
        const auto key = cache_key(scope);
        const auto entry = find_entry(key);
        const auto raw =
            entry ? core::cache::value_as<std::string>(entry->value, std::string{}) : std::string{};
        auto count = decode_hash_field(raw, kCounterField);
        auto decline = decode_hash_field(raw, kDeclineField);
        std::uint64_t incremented{0};
        if (field == kDeclineField) {
            incremented = ++decline;
        } else {
            incremented = ++count;
        }
        m_context.cache()
            .put(key, encode_hash(count, decline), core::cache::Ttl{}.seconds(ttl_seconds));
        return incremented;
    }

    /** @brief `increment_counter` TTL selection (kill_switch.rs:485-497,1052-1057). */
    [[maybe_unused]] std::uint64_t
    increment_counter(const ScopeInfo& scope, std::string_view field) const
    {
        return write_hash_counter(
            scope,
            field,
            scope.shadow ? kShadowTtlSeconds : kPrimaryTtlSeconds
        );
    }

    /** @brief `DelReply` mapping for `delete_key` (kill_switch.rs:553-576). */
    bool delete_key(const ScopeInfo& scope) const
    {
        const auto key = cache_key(scope);
        // An expired entry is invisible to `execute` (CacheQuerySpec::matches),
        // just as an expired Redis key is absent for DEL -> KeyNotDeleted.
        if (!find_entry(key)) {
            return false;
        }
        m_context.cache().erase(key);
        return true;
    }

    /**
     * @brief Persistent RolloutConfig lookup (kill_switch.rs:610-638):
     * `find_config_by_key_unwrap_or(config_key, "not_configured")`, sentinel
     * filter, typed serde parse, then `(kill_switch_threshold, decline)`.
     */
    [[nodiscard]] std::optional<RolloutConfigView> load_rollout_config(std::string_view scope) const
    {
        const auto key = std::string{kRolloutConfigPrefix} + "_" + std::string{scope};
        const auto raw = m_context.repository().get_config(key);
        if (!raw || *raw == kNotConfigured) {
            return std::nullopt;
        }
        auto parsed = serde::Ser::deserialize<RolloutConfigView>("application/json", *raw);
        if (!parsed) {
            // kill_switch.rs:628-636: an unparseable config is logged and treated
            // as absent; fail-closed metrics counter is the local log substitute.
            m_context.metrics()
                .counter("unified_connector_service.rollout_config_parse_error")
                .increment();
            return std::nullopt;
        }
        return *parsed;
    }

    /** @brief Parse an unsigned config integer (no positivity requirement). */
    [[nodiscard]] std::optional<std::uint64_t> parse_unsigned(std::string_view text) const noexcept
    {
        if (text.empty()) {
            return std::nullopt;
        }
        std::uint64_t value{};
        const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
        if (ec == std::errc{} && ptr == text.data() + text.size()) {
            return value;
        }
        return std::nullopt;
    }

    /** @brief Parse a signed i64 (Superposition config value). */
    [[nodiscard]] std::optional<std::int64_t> parse_signed(std::string_view text) const noexcept
    {
        if (text.empty()) {
            return std::nullopt;
        }
        std::int64_t value{};
        const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
        if (ec == std::errc{} && ptr == text.data() + text.size()) {
            return value;
        }
        return std::nullopt;
    }

    /**
     * @brief Local substitute for `get_ucs_config_source(state)` (helpers.rs:2677-2688).
     * Default is `Database`; only "superposition" selects the Superposition path.
     */
    [[nodiscard]] bool config_source_is_superposition() const
    {
        std::optional<std::string> raw =
            m_context.config_store().get_value("grpc_clients", "unified_connector_service");
        if (!raw) {
            raw = m_context.repository().get_config("grpc_clients.unified_connector_service");
        }
        if (!raw) {
            if (const auto record = m_context.superposition()
                                        .get_config("grpc_clients", "unified_connector_service")) {
                raw = record->get_value();
            }
        }
        return raw && raw->find("superposition") != std::string::npos;
    }

    /**
     * @brief Port of `shadow::threshold` (kill_switch.rs:928-965).
     * Order matters: comparison-service fallback first, then the Superposition
     * value only when the configured source is Superposition; any failure or
     * non-positive value preserves the fallback.
     */
    [[nodiscard]] std::uint64_t shadow_threshold() const
    {
        std::uint64_t fallback{1};
        if (const auto value = m_context.config_store().get_value(
                "comparison_service",
                "shadow_kill_switch_threshold"
            )) {
            if (const auto parsed = parse_unsigned(*value)) {
                fallback = *parsed;
            }
        }
        if (!config_source_is_superposition()) {
            return fallback;
        }

        std::optional<std::string> raw =
            m_context.repository().get_config(kSuperpositionThresholdKey);
        if (!raw) {
            if (const auto record =
                    m_context.superposition().get_config("ucs", "shadow_kill_switch_threshold")) {
                raw = record->get_value();
            }
        }
        if (!raw) {
            return fallback;
        }
        const auto parsed = parse_signed(*raw);
        // u64::try_from(value) && > 0 (kill_switch.rs:954-964).
        if (!parsed || *parsed <= 0) {
            return fallback;
        }
        return static_cast<std::uint64_t>(*parsed);
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::unified_connector_service

template<>
struct serde::Serializable<::app::handlers::unified_connector_service::RolloutConfigView>
{
    using View = ::app::handlers::unified_connector_service::RolloutConfigView;

    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "kill_switch_threshold",
                &View::get_kill_switch_threshold,
                &View::set_kill_switch_threshold,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_decline_threshold",
                &View::get_connector_decline_threshold,
                &View::set_connector_decline_threshold,
                serde::FieldOptions{}>{}
        };
    }
};
