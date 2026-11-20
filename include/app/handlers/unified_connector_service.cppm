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

class UnifiedConnectorServiceRouteHandlers
{
public:
    void handle_kill_switch_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Route rule comes from the single declarative registry
        // (registry.cppm:191 -> unified_connector_service.md:21). PLATFORM_ADMIN
        // still requires a bearer token, so the failure mode is unchanged.
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
        if (resolved.scope.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST, "scope is required"
            ));
            return;
        }

        // Upstream trip_status: both counters are read; the decline counter is
        // only fetched when a connector-decline threshold is configured.
        std::uint64_t counter = read_counter(counter_key(resolved));
        std::optional<std::uint64_t> threshold{};
        std::optional<std::uint64_t> decline_threshold{};
        std::uint64_t decline_counter{0};

        if (resolved.shadow) {
            // Shadow trips against the global (Superposition/comparison-service)
            // threshold, not the per-scope rollout config.
            threshold = shadow_threshold();
        } else {
            const auto config = load_rollout_config(resolved.scope);
            if (config) {
                // helpers.rs:2719 default_kill_switch_threshold() == 1.
                threshold = config->get_kill_switch_threshold().value_or(1);
                decline_threshold = config->get_connector_decline_threshold();
            }
            if (decline_threshold) {
                decline_counter = read_counter(decline_key(resolved));
            }
        }

        const bool tripped = (threshold ? counter >= *threshold : counter > 0)
            || (decline_threshold && decline_counter >= *decline_threshold);

        m_context->metrics().record_flow("UnifiedConnectorServiceKillSwitchStatus");
        m_context->metrics().counter("unified_connector_service.kill_switch_status").increment();

        // Typed response DTO: absent Option<u64> thresholds encode as JSON null,
        // matching upstream KillSwitchStatusResponse serialization.
        KillSwitchStatusResponse response{};
        response.set_rollout_scope(resolved.scope);
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
        if (resolved.scope.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST, "scope is required"
            ));
            return;
        }

        // Upstream delete_key maps DelReply::KeyNotDeleted to GenericNotFound.
        // Primary counter is in the tenant store, the shadow counter in the
        // global store; both are cleared.
        const bool removed = m_context->cache().erase(counter_key(resolved));
        if (!resolved.shadow) {
            m_context->cache().erase(decline_key(resolved));
        }
        if (!removed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::NOT_FOUND,
                "No UCS kill switch counter found for scope " + resolved.scope
            ));
            return;
        }

        m_context->metrics().record_flow("UnifiedConnectorServiceKillSwitchReset");
        m_context->metrics().counter("unified_connector_service.kill_switch_reset").increment();

        // Upstream reset returns ApplicationResponse::StatusOk with no JSON body.
        send_json(res, send, std::string_view{}, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/unified-connector-service"}
            .add_route(
                Route{"/kill-switch/:scope"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_kill_switch_status(req, res, std::move(send));
                    })
                    .summary("Hyperswitch kill_switch_status")
            )
            .add_route(
                Route{"/kill-switch/:scope"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_reset_kill_switch(req, res, std::move(send));
                    })
                    .summary("Hyperswitch reset_kill_switch")
            );
    }

    explicit UnifiedConnectorServiceRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

public:
    /** @brief A scope plus the tenant/global store discriminator. */
    struct ScopeInfo
    {
        std::string scope{};
        bool shadow{false};
    };

    /** @brief Typed RolloutConfig subset used by the threshold lookup. */
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
        // Upstream default_kill_switch_threshold() == 1 when the config is present,
        // applied by the caller so an absent JSON field stays optional here.
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
    /** @brief Upstream rollout_scope_in: split_once(prefix) accepts a full listed key. */
    [[nodiscard]] ScopeInfo resolve_scope(interfaces::io::IRequest& req) const
    {
        constexpr std::string_view counter_prefix{"ucs_kill_switch_"};
        constexpr std::string_view shadow_key{"ucs_shadow_killed"};

        ScopeInfo result{};
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() == 0) {
            return result;
        }
        result.scope = std::string{segments.at(segments.count() - 1).value_or("")};
        if (result.scope.empty()) {
            return result;
        }

        // split_once semantics: accept any leading tenant prefix, not just a
        // key that starts with the prefix (kill_switch.rs:56-63).
        const auto pos = result.scope.find(counter_prefix);
        if (pos != std::string::npos) {
            result.scope.erase(0, pos + counter_prefix.size());
        }
        result.shadow = result.scope == shadow_key;
        return result;
    }

    /**
     * @brief Tenant/global store split represented by key namespaces.
     * Real Redis uses two connections (state.store / state.global_store); the
     * local cache is a single instance, so the two stores are partitioned by
     * namespace. Counter TTLs are owned by the absence enforcement hooks.
     */
    [[nodiscard]] std::string counter_key(const ScopeInfo& scope) const
    {
        return scope.shadow
            ? std::string{"global/ucs_shadow_killed"}
            : std::string{"tenant/ucs_kill_switch_"} + scope.scope;
    }

    [[nodiscard]] std::string decline_key(const ScopeInfo& scope) const
    {
        return counter_key(scope) + ":counter_connector_decline";
    }

    [[nodiscard]] std::uint64_t read_counter(std::string_view key) const
    {
        return m_context->cache().get(key).value_or<std::uint64_t>(0);
    }

    /**
     * @brief Persistent RolloutConfig lookup with the upstream sentinel filter
     * and typed serde parse (kill_switch.rs:613-638, helpers.rs:2690-2713).
     */
    [[nodiscard]] std::optional<RolloutConfigView> load_rollout_config(std::string_view scope) const
    {
        constexpr std::string_view not_configured{"not_configured"};
        const auto key = std::string{"ucs_rollout_config_"} + std::string{scope};
        const auto raw = m_context->repository().get_config(key);
        if (!raw || *raw == not_configured) {
            return std::nullopt;
        }
        auto parsed = serde::Ser::deserialize<RolloutConfigView>("application/json", *raw);
        if (!parsed) {
            // Fail closed: an unreadable config leaves thresholds unset so any
            // recorded failure trips the switch. Mirrors Redis-error logging.
            m_context->metrics()
                .counter("unified_connector_service.rollout_config_parse_error")
                .increment();
            return std::nullopt;
        }
        return *parsed;
    }

    /** @brief Parse a positive unsigned integer from a config value. */
    [[nodiscard]] std::optional<std::uint64_t> parse_positive(std::string_view text) const
    {
        if (text.empty()) {
            return std::nullopt;
        }
        std::uint64_t value{};
        const char* const begin = text.data();
        const char* const end = begin + text.size();
        const auto [ptr, ec] = std::from_chars(begin, end, value);
        if (ec == std::errc{} && ptr == end && value > 0) {
            return value;
        }
        return std::nullopt;
    }

    /**
     * @brief Shadow threshold chain (kill_switch.rs:928-965): Superposition /
     * persistent ucs config first, then comparison-service config, then 1.
     */
    [[nodiscard]] std::uint64_t shadow_threshold() const
    {
        constexpr std::string_view superposition_key{"ucs.shadow_kill_switch_threshold"};
        constexpr std::string_view comparison_key{"comparison_service.shadow_kill_switch_threshold"};

        if (const auto value = m_context->repository().get_config(superposition_key)) {
            if (const auto parsed = parse_positive(*value)) {
                return *parsed;
            }
        }
        if (const auto record = m_context->superposition().get_config("ucs", "shadow_kill_switch_threshold")) {
            if (const auto parsed = parse_positive(record->get_value())) {
                return *parsed;
            }
        }
        if (const auto value = m_context->repository().get_config(comparison_key)) {
            if (const auto parsed = parse_positive(*value)) {
                return *parsed;
            }
        }
        m_context->metrics()
            .counter("unified_connector_service.shadow_threshold_fallback")
            .increment();
        return 1;
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
