module;
export module app_handlers:unified_connector_service;

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
import app_auth;
import app_http;
import app_metrics;
import app_services;

export namespace app::handlers::unified_connector_service {

// Upstream gates the whole scope behind `#[cfg(feature = "olap")]`
// (routes/app.rs:2444). The project's local compile-time-feature convention is
// a namespace-scoped `constexpr bool` (health.cppm:27); this is that substitute.
inline constexpr bool kOlap = true;

// Constants mirrored verbatim from upstream consts.rs:389,393,399,405,407,409
// and kill_switch.rs:23,29.
inline constexpr std::string_view kRolloutConfigPrefix{"ucs_rollout_config"};
inline constexpr std::string_view kNotConfigured{"not_configured"};
inline constexpr std::string_view kKillSwitchPrefix{"ucs_kill_switch"};
inline constexpr std::string_view kShadowKey{"ucs_shadow_killed"};
inline constexpr std::string_view kCounterField{"counter"};
inline constexpr std::string_view kDeclineField{"counter_connector_decline"};
inline constexpr std::int64_t kPrimaryTtlSeconds{7 * 24 * 60 * 60};    // consts.rs:405
inline constexpr std::int64_t kShadowTtlSeconds{24 * 60 * 60};         // consts.rs:409
inline constexpr std::string_view kSuperpositionThresholdKey{"ucs.shadow_kill_switch_threshold"}; // consts.rs:615

/**
 * @brief Strict local port of upstream `core/unified_connector_service/kill_switch.rs`
 * (SHA 7736d55adaf235f7af74e133ae408573b1826823).
 *
 * The two route endpoints are ported line-by-line:
 *  - `kill_switch::trip_status` (kill_switch.rs:580-671) -> handle_kill_switch_status
 *  - `kill_switch::reset`       (kill_switch.rs:543-577) -> handle_reset_kill_switch
 *
 * Redis (external infra) is replaced by the injected `core::cache` in-process, but
 * every observable behaviour is preserved: two independent store namespaces stand
 * in for `state.store` (tenant) and `state.global_store` (global); each Redis hash
 * is one cache entry whose JSON value carries the `counter` and
 * `counter_connector_decline` hash fields; expiry carries the HINCRBY/EXPIRE TTL;
 * `DEL` maps `KeyDeleted`/`KeyNotDeleted` exactly.
 */
class UnifiedConnectorServiceRouteHandlers
{
public:
    void handle_kill_switch_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream routes/unified_connector_service.rs:22-30: server_wrap(flow,
        // ..., AdminApiAuth, LockAction::NotApplicable). The declarative rule for
        // this operation is PLATFORM_ADMIN / AppPermission::ADMIN
        // (auth/registry.cppm:191), the local AdminApiAuth equivalent. No lock is
        // acquired, which is LockAction::NotApplicable locally; record_flow below
        // is the #[instrument(flow = ...)] substitute.
        const auto rule = m_context->auth_rules().rule_for("kill_switch_status");
        const auto auth = m_context->authorizer().authorize_route(
            app::auth::AuthContextBuilder{}(req),
            rule
        );
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        const auto resolved = resolve_scope(req);
        if (resolved.listed_key.empty()) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::INVALID_REQUEST, "scope is required"
            ));
            return;
        }

        // A closed local cache is the in-process equivalent of a failed
        // `get_redis_conn`. Upstream fails the request with InternalServerError
        // and the matching attach_printable message (kill_switch.rs:585-588,
        // :604-608, :646-650).
        if (!m_context->cache().is_open()) {
            const auto message = resolved.shadow
                ? std::string{"Failed to read the shadow kill switch counter"}
                : std::string{"Failed to read the UCS kill switch counter"};
            send_error(res, send, make_error(
                core::error::AppErrorKind::INTERNAL_ERROR, message
            ));
            return;
        }

        std::uint64_t counter{0};
        std::optional<std::uint64_t> threshold{};
        std::uint64_t decline_counter{0};
        std::optional<std::uint64_t> decline_threshold{};
        bool tripped{false};

        if (resolved.shadow) {
            // kill_switch.rs:584-599: global hash `ucs_shadow_killed`, field
            // `counter`; threshold from shadow::threshold; declines fixed at 0.
            counter = read_hash_counter(resolved, kCounterField);
            const auto configured = shadow_threshold();
            threshold = configured;
            tripped = exceeds_threshold(counter, configured);
        } else {
            // kill_switch.rs:602-659: tenant hash `ucs_kill_switch_<scope>`.
            counter = read_hash_counter(resolved, kCounterField);

            const auto config = load_rollout_config(resolved.scope);
            if (config) {
                // RolloutConfig::kill_switch_threshold defaults to 1 when present
                // but unset (helpers.rs:2696-2697,2719-2721).
                threshold = config->get_kill_switch_threshold().value_or(1);
                decline_threshold = config->get_connector_decline_threshold();
            }

            // kill_switch.rs:645-653: only read when a threshold can use it.
            if (decline_threshold) {
                decline_counter = read_hash_counter(resolved, kDeclineField);
            }

            // kill_switch.rs:655-659: no threshold => any count trips, exactly
            // as upstream (not `counter >= 1`, which differs only when 0).
            tripped = (threshold ? exceeds_threshold(counter, *threshold) : counter > 0)
                || (decline_threshold
                        ? exceeds_threshold(decline_counter, *decline_threshold)
                        : false);
        }

        // Flow::UnifiedConnectorServiceKillSwitchStatus instrumentation
        // (routes/unified_connector_service.rs:14,20).
        m_context->metrics().record_flow("UnifiedConnectorServiceKillSwitchStatus");
        m_context->metrics().counter("unified_connector_service.kill_switch_status").increment();

        // Upstream KillSwitchStatusResponse (kill_switch.rs:524-538): serde emits
        // Option<u64> thresholds as JSON null when absent. Field order matches.
        KillSwitchStatusResponse response{};
        response.set_rollout_scope(
            resolved.shadow ? resolved.listed_key : resolved.scope
        );
        response.set_counter(counter);
        response.set_threshold(threshold);
        response.set_decline_counter(decline_counter);
        response.set_connector_decline_threshold(decline_threshold);
        response.set_tripped(tripped);
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_reset_kill_switch(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream routes/unified_connector_service.rs:44-52:
        // server_wrap(flow, ..., AdminApiAuth, LockAction::NotApplicable).
        const auto rule = m_context->auth_rules().rule_for("reset_kill_switch");
        const auto auth = m_context->authorizer().authorize_route(
            app::auth::AuthContextBuilder{}(req),
            rule
        );
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        const auto resolved = resolve_scope(req);
        if (resolved.listed_key.empty()) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::INVALID_REQUEST, "scope is required"
            ));
            return;
        }

        // kill_switch.rs:553-559: connection failure -> InternalServerError.
        if (!m_context->cache().is_open()) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "Failed to get a redis connection to clear the UCS kill switch"
            ));
            return;
        }

        // kill_switch.rs:545-552: shadow counter lives in the global store under
        // its listed key; primary counters live in the tenant store under the
        // `ucs_kill_switch_<scope>` hash key. Both hash fields are deleted by the
        // single DEL (the local substitute erases the one cache entry that
        // carries both fields).
        if (!delete_key(resolved)) {
            // kill_switch.rs:570-575: DelReply::KeyNotDeleted -> GenericNotFound.
            send_error(res, send, make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "No UCS kill switch counter found for scope " + resolved.scope
            ));
            return;
        }

        // Flow::UnifiedConnectorServiceKillSwitchReset instrumentation
        // (routes/unified_connector_service.rs:36,42).
        m_context->metrics().record_flow("UnifiedConnectorServiceKillSwitchReset");
        m_context->metrics().counter("unified_connector_service.kill_switch_reset").increment();

        // kill_switch.rs:562-569: ApplicationResponse::StatusOk, no JSON body.
        send_json(res, send, std::string_view{}, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        // routes/app.rs:2444-2454: the entire scope is gated by feature "olap".
        if constexpr (!kOlap) {
            return;
        }

        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "unified-connector-service"}
            .add_router(Router{router, "kill-switch"}.add_route(
                Route{":scope"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_kill_switch_status(req, res, std::move(send));
                    })
                    .summary("Hyperswitch kill_switch_status")
            ))
            .add_router(Router{router, "kill-switch"}.add_route(
                Route{":scope"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_reset_kill_switch(req, res, std::move(send));
                    })
                    .summary("Hyperswitch reset_kill_switch")
            ));
    }

    explicit UnifiedConnectorServiceRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

public:
    /**
     * @brief Upstream `listed_key`/`rollout_scope` pair.
     *
     * `listed_key` is the original final path segment (what upstream compares to
     * UCS_SHADOW_KILL_SWITCH_REDIS_KEY); `scope` is `rollout_scope_in(listed_key)`.
     * Shadow is decided from `listed_key`, NOT the stripped scope: upstream checks
     * the original key (kill_switch.rs:545,584), so a tenant key whose scope
     * happens to equal `ucs_shadow_killed` must stay in the tenant store.
     */
    struct ScopeInfo
    {
        std::string listed_key{};
        std::string scope{};
        bool shadow{false};
    };

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
        [[nodiscard]] const std::optional<std::uint64_t>& get_connector_decline_threshold() const noexcept
        {
            return m_connector_decline_threshold;
        }

    private:
        // Upstream defaults applied at the call site: kill_switch_threshold -> 1
        // (helpers.rs:2719-2721), connector_decline_threshold -> None.
        std::optional<std::uint64_t> m_kill_switch_threshold{};
        std::optional<std::uint64_t> m_connector_decline_threshold{};
    };

    /** @brief Typed upstream KillSwitchStatusResponse wire shape. */
    class KillSwitchStatusResponse
    {
    public:
        void set_rollout_scope(std::string value)
        {
            m_rollout_scope = std::move(value);
        }
        [[nodiscard]] const std::string& get_rollout_scope() const noexcept
        {
            return m_rollout_scope;
        }
        void set_counter(std::uint64_t value) noexcept
        {
            m_counter = value;
        }
        [[nodiscard]] std::uint64_t get_counter() const noexcept
        {
            return m_counter;
        }
        void set_threshold(std::optional<std::uint64_t> value) noexcept
        {
            m_threshold = value;
        }
        [[nodiscard]] const std::optional<std::uint64_t>& get_threshold() const noexcept
        {
            return m_threshold;
        }
        void set_decline_counter(std::uint64_t value) noexcept
        {
            m_decline_counter = value;
        }
        [[nodiscard]] std::uint64_t get_decline_counter() const noexcept
        {
            return m_decline_counter;
        }
        void set_connector_decline_threshold(std::optional<std::uint64_t> value) noexcept
        {
            m_connector_decline_threshold = value;
        }
        [[nodiscard]] const std::optional<std::uint64_t>& get_connector_decline_threshold() const noexcept
        {
            return m_connector_decline_threshold;
        }
        void set_tripped(bool value) noexcept
        {
            m_tripped = value;
        }
        [[nodiscard]] bool get_tripped() const noexcept
        {
            return m_tripped;
        }

    private:
        std::string m_rollout_scope{};
        std::uint64_t m_counter{0};
        std::optional<std::uint64_t> m_threshold{};
        std::uint64_t m_decline_counter{0};
        std::optional<std::uint64_t> m_connector_decline_threshold{};
        bool m_tripped{false};
    };

private:
    /** @brief `count >= threshold` (kill_switch.rs:31-34). */
    [[nodiscard]] static bool exceeds_threshold(std::uint64_t count, std::uint64_t threshold) noexcept
    {
        return count >= threshold;
    }

    /** @brief Port of `rollout_scope_in` (kill_switch.rs:52-63). */
    [[nodiscard]] ScopeInfo resolve_scope(interfaces::io::IRequest& req) const
    {
        ScopeInfo result{};
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() == 0) {
            return result;
        }
        result.listed_key = std::string{segments.at(segments.count() - 1).value_or("")};
        if (result.listed_key.empty()) {
            return result;
        }

        // split_once(prefix): accept any leading tenant prefix, not just a key
        // that starts with the prefix (kill_switch.rs:56-63).
        result.scope = result.listed_key;
        const auto prefix = std::string{kKillSwitchPrefix} + "_";
        if (const auto pos = result.scope.find(prefix); pos != std::string::npos) {
            result.scope.erase(0, pos + prefix.size());
        }

        // Shadow is decided from the *original* listed key (kill_switch.rs:545,584).
        result.shadow = result.listed_key == kShadowKey;
        return result;
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
        return std::string{"unified_connector_service.redis."}
            + (scope.shadow ? "global:" : "tenant:")
            + redis_key(scope);
    }

    [[nodiscard]] std::optional<core::cache::CacheEntry> find_entry(std::string_view key) const
    {
        core::cache::CacheQuerySpec spec;
        spec.prefix = std::string{key};
        for (const auto& entry : m_context->cache().execute(spec)) {
            if (entry.key == key) {
                return entry;
            }
        }
        return std::nullopt;
    }

    /** @brief Internal JSON encoding of one Redis hash value. */
    [[nodiscard]] static std::string encode_hash(std::uint64_t count, std::uint64_t decline)
    {
        return std::format(R"({{"counter":{},"counter_connector_decline":{}}})", count, decline);
    }

    [[nodiscard]] static std::uint64_t decode_hash_field(
        const std::string& json,
        std::string_view field
    )
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
    [[nodiscard]] std::uint64_t read_hash_counter(
        const ScopeInfo& scope,
        std::string_view field
    ) const
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
        const auto raw = entry
            ? core::cache::value_as<std::string>(entry->value, std::string{})
            : std::string{};
        auto count = decode_hash_field(raw, kCounterField);
        auto decline = decode_hash_field(raw, kDeclineField);
        std::uint64_t incremented{0};
        if (field == kDeclineField) {
            incremented = ++decline;
        } else {
            incremented = ++count;
        }
        m_context->cache().put(key, encode_hash(count, decline), core::cache::Ttl{}.seconds(ttl_seconds));
        return incremented;
    }

    /** @brief `increment_counter` TTL selection (kill_switch.rs:485-497,1052-1057). */
    [[maybe_unused]] std::uint64_t increment_counter(
        const ScopeInfo& scope,
        std::string_view field
    ) const
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
        m_context->cache().erase(key);
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
        const auto raw = m_context->repository().get_config(key);
        if (!raw || *raw == kNotConfigured) {
            return std::nullopt;
        }
        auto parsed = serde::Ser::deserialize<RolloutConfigView>("application/json", *raw);
        if (!parsed) {
            // kill_switch.rs:628-636: an unparseable config is logged and treated
            // as absent; fail-closed metrics counter is the local log substitute.
            m_context->metrics()
                .counter("unified_connector_service.rollout_config_parse_error")
                .increment();
            return std::nullopt;
        }
        return *parsed;
    }

    /** @brief Parse an unsigned config integer (no positivity requirement). */
    [[nodiscard]] static std::optional<std::uint64_t> parse_unsigned(std::string_view text) noexcept
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
    [[nodiscard]] static std::optional<std::int64_t> parse_signed(std::string_view text) noexcept
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
            m_context->config_store().get_value("grpc_clients", "unified_connector_service");
        if (!raw) {
            raw = m_context->repository().get_config("grpc_clients.unified_connector_service");
        }
        if (!raw) {
            if (const auto record = m_context->superposition().get_config(
                    "grpc_clients", "unified_connector_service"
                )) {
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
        if (const auto value = m_context->config_store().get_value(
                "comparison_service", "shadow_kill_switch_threshold"
            )) {
            if (const auto parsed = parse_unsigned(*value)) {
                fallback = *parsed;
            }
        }
        if (!config_source_is_superposition()) {
            return fallback;
        }

        std::optional<std::string> raw = m_context->repository().get_config(kSuperpositionThresholdKey);
        if (!raw) {
            if (const auto record = m_context->superposition().get_config(
                    "ucs", "shadow_kill_switch_threshold"
                )) {
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

    [[nodiscard]] core::error::AppError make_error(
        core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::unified_connector_service

// Reflect-cpp is wired to serde::Serializable<>::fields(), so the typed wire
// shapes need an explicit FieldDesc reflection to round-trip through
// Ser::deserialize / Ser::encode_json.
template<>
struct serde::Serializable<
    app::handlers::unified_connector_service::UnifiedConnectorServiceRouteHandlers::RolloutConfigView>
{
    using View = app::handlers::unified_connector_service::
        UnifiedConnectorServiceRouteHandlers::RolloutConfigView;

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

template<>
struct serde::Serializable<
    app::handlers::unified_connector_service::UnifiedConnectorServiceRouteHandlers::KillSwitchStatusResponse>
{
    using Response = app::handlers::unified_connector_service::
        UnifiedConnectorServiceRouteHandlers::KillSwitchStatusResponse;

    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "rollout_scope",
                &Response::get_rollout_scope,
                &Response::set_rollout_scope,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "counter",
                &Response::get_counter,
                &Response::set_counter,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "threshold",
                &Response::get_threshold,
                &Response::set_threshold,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "decline_counter",
                &Response::get_decline_counter,
                &Response::set_decline_counter,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_decline_threshold",
                &Response::get_connector_decline_threshold,
                &Response::set_connector_decline_threshold,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tripped",
                &Response::get_tripped,
                &Response::set_tripped,
                serde::FieldOptions{}>{}
        };
    }
};
