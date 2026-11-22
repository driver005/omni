module;
export module app_handlers:health;

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
import app_metrics;
import app_connector;

export namespace app::handlers::health {

/**
 * Local substitutes for the upstream Cargo features that shape the deep health
 * response (`api_models/health_check.rs:8-17`, `routes/app.rs:812-829`). This port
 * has no Cargo `#[cfg]`, so these constexpr gates are the exact local equivalent:
 * they decide both which fields are emitted and which probes run. Both feature
 * sets are compiled in here, matching an upstream release build.
 */
inline constexpr bool kOlap = true;            // analytics + opensearch fields
inline constexpr bool kDynamicRouting = true;  // grpc_health_check + decision_engine fields
inline constexpr bool kV1 = true;              // /health scope
inline constexpr bool kV2 = true;              // /v2/health scope

class HealthRouteHandlers
{
public:
    void handle_health(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream routes::health::health() has no server_wrap and no auth at all.
        // The registry resolves "health" to AuthScheme::NO_AUTH
        // (auth/registry.cppm:182), so authorize_route is an exact no-op here.
        const auto rule = m_context->auth_rules().rule_for("health");
        const auto auth = m_context->authorizer().authorize_route(
            app::auth::AuthContextBuilder{}(req),
            rule
        );
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        // HEALTH_METRIC.add(1, &[]) (routes/health.rs:17) — local metrics substitute.
        m_context->metrics().record_flow("health");
        m_context->metrics().counter("health").increment();

        // HttpResponse::Ok().body("health is good") — exact plain-text body.
        send_text(res, send, "health is good");
    }

    void handle_deep_health_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // Upstream uses &auth::NoAuth via server_wrap (routes/health.rs:38).
        // "deep_health_check" resolves to AuthScheme::NO_AUTH (auth/registry.cppm:101).
        const auto rule = m_context->auth_rules().rule_for("deep_health_check");
        const auto auth = m_context->authorizer().authorize_route(
            app::auth::AuthContextBuilder{}(req),
            rule
        );
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        // HEALTH_METRIC.add(1, &[]) (routes/health.rs:28) — local metrics substitute.
        m_context->metrics().record_flow("deep_health_check");
        m_context->metrics().counter("health").increment();

        m_probe_error.clear();

        const HealthStateProjection project{};
        using State = app::dto::health_check::HealthState;

        // Sequence and component names mirror deep_health_check_func exactly
        // (routes/health.rs:51-166). Any failure maps to the upstream
        // ApiErrorResponse::HealthCheckError wire shape and returns early.

        // Database: store.health_check_db() (core/health_check.rs:51-55).
        const State database_state = probe_database();
        if (database_state == State::Error) {
            send_health_error(res, send, "Database", m_probe_error);
            return;
        }

        // Redis: set/get/delete cycle (core/health_check.rs:57-85).
        const State redis_state = probe_redis();
        if (redis_state == State::Error) {
            send_health_error(res, send, "Redis", m_probe_error);
            return;
        }

        // Locker/vault: real keystore probe when the locker is active; the
        // disabled/mocked path returns NotApplicable (core/health_check.rs:87-105).
        const State vault_state = probe_vault();
        if (vault_state == State::Error) {
            send_health_error(res, send, "Locker", m_probe_error);
            return;
        }

        // [olap] Analytics provider deep health check (core/health_check.rs:107-144).
        State analytics_state = State::NotApplicable;
        if constexpr (kOlap) {
            analytics_state = probe_analytics();
            if (analytics_state == State::Error) {
                send_health_error(res, send, "Analytics", m_probe_error);
                return;
            }
        }

        // [dynamic_routing] gRPC HealthCheckMap (core/health_check.rs:181-195).
        std::unordered_map<std::string, bool> grpc_map{};
        if constexpr (kDynamicRouting) {
            auto probed = probe_grpc_map();
            if (!probed.has_value()) {
                send_health_error(res, send, "gRPC services", m_probe_error);
                return;
            }
            grpc_map = std::move(*probed);
        }

        // [dynamic_routing] Decision Engine (core/health_check.rs:197-221).
        State decision_state = State::NotApplicable;
        if constexpr (kDynamicRouting) {
            decision_state = probe_decision_engine();
            if (decision_state == State::Error) {
                send_health_error(res, send, "Decision Engine service", m_probe_error);
                return;
            }
        }

        // [olap] OpenSearch (core/health_check.rs:146-159).
        State opensearch_state = State::NotApplicable;
        if constexpr (kOlap) {
            opensearch_state = probe_opensearch();
            if (opensearch_state == State::Error) {
                send_health_error(res, send, "Opensearch", m_probe_error);
                return;
            }
        }

        // Outgoing Request (core/health_check.rs:161-179).
        const State outgoing_state = probe_outgoing();
        if (outgoing_state == State::Error) {
            send_health_error(res, send, "Outgoing Request", m_probe_error);
            return;
        }

        // Unified Connector Service (core/health_check.rs:223-285).
        const State ucs_state = probe_unified_connector_service();
        if (ucs_state == State::Error) {
            send_health_error(res, send, "Unified Connector Service", m_probe_error);
            return;
        }

        // Exact HealthState -> bool / Option<bool> conversions
        // (api_models/health_check.rs:45-61), with feature-gated field presence
        // and `skip_serializing_if = "Option::is_none"` on the two Option fields.
        const std::string body = encode_health_response(
            project.as_bool(database_state),
            project.as_bool(redis_state),
            project.as_optional(vault_state),
            project.as_bool(analytics_state),
            project.as_bool(opensearch_state),
            project.as_bool(outgoing_state),
            grpc_map,
            project.as_bool(decision_state),
            project.as_optional(ucs_state)
        );

        send_json(res, send, body);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Upstream v1: web::scope("health") + resource("") and resource("/ready")
        // (app.rs:812-819). resource("") is /health with NO trailing slash.
        if constexpr (kV1) {
            Router{router, "health"}
                .add_route(
                    Route{""}
                        .get([this](
                                 interfaces::io::IRequest& req,
                                 interfaces::io::IResponse& res,
                                 std::function<void()> send
                             ) {
                                 handle_health(req, res, std::move(send));
                             })
                        .summary("Hyperswitch health")
                )
                .add_route(
                    Route{"ready"}
                        .get([this](
                                 interfaces::io::IRequest& req,
                                 interfaces::io::IResponse& res,
                                 std::function<void()> send
                             ) {
                                 handle_deep_health_check(req, res, std::move(send));
                             })
                        .summary("Hyperswitch deep_health_check")
                );
        }

        // Upstream v2: web::scope("/v2/health") (app.rs:822-829).
        if constexpr (kV2) {
            Router{router, "v2/health"}
                .add_route(
                    Route{""}
                        .get([this](
                                 interfaces::io::IRequest& req,
                                 interfaces::io::IResponse& res,
                                 std::function<void()> send
                             ) {
                                 handle_health(req, res, std::move(send));
                             })
                        .summary("Hyperswitch health v2")
                )
                .add_route(
                    Route{"ready"}
                        .get([this](
                                 interfaces::io::IRequest& req,
                                 interfaces::io::IResponse& res,
                                 std::function<void()> send
                             ) {
                                 handle_deep_health_check(req, res, std::move(send));
                             })
                        .summary("Hyperswitch deep_health_check v2")
                );
        }
    }

    explicit HealthRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    /**
     * @brief Local projection of upstream HealthState to wire types.
     * Running -> true / Some(true); Error -> false / Some(false);
     * NotApplicable -> false / None (api_models/health_check.rs:39-61).
     */
    class HealthStateProjection
    {
    public:
        [[nodiscard]] bool as_bool(app::dto::health_check::HealthState state) const noexcept
        {
            using State = app::dto::health_check::HealthState;
            return state == State::Running;
        }

        [[nodiscard]] std::optional<bool>
        as_optional(app::dto::health_check::HealthState state) const noexcept
        {
            using State = app::dto::health_check::HealthState;
            if (state == State::NotApplicable) {
                return std::nullopt;
            }
            return state == State::Running;
        }
    };

    // ───────────────────────────── Local health probes ─────────────────────────

    [[nodiscard]] app::dto::health_check::HealthState probe_database()
    {
        using State = app::dto::health_check::HealthState;
        try {
            if (!m_context->database().is_open()) {
                m_probe_error = "database is not open";
                return State::Error;
            }
            (void)m_context->repository().list_payments();
            return State::Running;
        } catch (const std::exception& error) {
            m_probe_error = error.what();
            return State::Error;
        } catch (...) {
            m_probe_error = "unknown database error";
            return State::Error;
        }
    }

    [[nodiscard]] app::dto::health_check::HealthState probe_redis()
    {
        using State = app::dto::health_check::HealthState;
        const std::string key{"health.redis.test_key"};
        try {
            m_context->cache().put(
                key,
                core::cache::to_cache_value(std::string{"test_value"}),
                core::cache::Ttl{}.seconds(30)
            );
            const auto probe = m_context->cache().get(key);
            m_context->cache().erase(key);
            if (!probe.hit || probe.value_or<std::string>(std::string{}) != "test_value") {
                m_probe_error = "redis set/get round-trip mismatch";
                return State::Error;
            }
            return State::Running;
        } catch (const std::exception& error) {
            m_probe_error = error.what();
            return State::Error;
        } catch (...) {
            m_probe_error = "unknown redis error";
            return State::Error;
        }
    }

    [[nodiscard]] app::dto::health_check::HealthState probe_vault()
    {
        using State = app::dto::health_check::HealthState;
        // Upstream gate: locker.locker_enabled && !locker.mock_locker. The local
        // ConfigStore holds the equivalent flags; absent values default to an
        // active locker so the keystore is genuinely probed by default.
        const auto enabled = m_context->config_store().get_value("health_check", "locker_enabled");
        const auto mocked = m_context->config_store().get_value("health_check", "mock_locker");
        const bool locker_active =
            (!enabled.has_value() || *enabled == "true") &&
            (!mocked.has_value() || *mocked != "true");
        if (!locker_active) {
            return State::NotApplicable;
        }
        try {
            // The local keystore is the vault/locker substitute; find_record is
            // backed by core::db::Database, so a failed store surfaces as Error.
            (void)m_context->key_store().find_record("health_check", "vault_probe");
            return State::Running;
        } catch (const std::exception& error) {
            m_probe_error = error.what();
            return State::Error;
        } catch (...) {
            m_probe_error = "unknown locker error";
            return State::Error;
        }
    }

    [[nodiscard]] app::dto::health_check::HealthState probe_analytics()
    {
        using State = app::dto::health_check::HealthState;
        // Upstream analytics providers are databases (Sqlx/Clickhouse). The
        // repository read over a distinct table is the real local DB substitute.
        try {
            (void)m_context->repository().list_refunds();
            return State::Running;
        } catch (const std::exception& error) {
            m_probe_error = error.what();
            return State::Error;
        } catch (...) {
            m_probe_error = "unknown analytics error";
            return State::Error;
        }
    }

    [[nodiscard]] app::dto::health_check::HealthState probe_opensearch()
    {
        using State = app::dto::health_check::HealthState;
        // Upstream: if an opensearch client is configured, deep_health_check it,
        // else NotApplicable. The local event bus is a DB-backed searchable store;
        // the configured-client flag is held in ConfigStore.
        const auto configured =
            m_context->config_store().get_value("health_check", "opensearch_enabled");
        if (configured.has_value() && *configured == "false") {
            return State::NotApplicable;
        }
        try {
            (void)m_context->events().find_by_event_id("health_check_probe_missing");
            return State::Running;
        } catch (const std::exception& error) {
            m_probe_error = error.what();
            return State::Error;
        } catch (...) {
            m_probe_error = "unknown opensearch error";
            return State::Error;
        }
    }

    [[nodiscard]] std::optional<std::unordered_map<std::string, bool>> probe_grpc_map()
    {
        if constexpr (!kDynamicRouting) {
            return std::unordered_map<std::string, bool>{};
        }
        try {
            // Upstream perform_health_check returns HealthCheckServices -> bool.
            // ConfigStore records in the grpc_health_check namespace are the local
            // substitute for the configured gRPC service health results.
            std::unordered_map<std::string, bool> result{};
            for (const auto& record : m_context->config_store().list("grpc_health_check")) {
                result[record.get_config_key()] =
                    (record.get_value() == "true" || record.get_value() == "SERVING");
            }
            return result;
        } catch (const std::exception& error) {
            m_probe_error = error.what();
            return std::nullopt;
        } catch (...) {
            m_probe_error = "unknown gRPC health check error";
            return std::nullopt;
        }
    }

    [[nodiscard]] app::dto::health_check::HealthState probe_decision_engine()
    {
        using State = app::dto::health_check::HealthState;
        if constexpr (!kDynamicRouting) {
            return State::NotApplicable;
        }
        // Upstream gates on open_router.dynamic_routing_enabled; the local
        // constexpr gate is on, so evaluate a trivial program through the real
        // in-process DecisionEngine to prove the service is functional.
        constexpr std::string_view program =
            R"({"rules":[],"default_action":"allow","default_connector":""})";
        constexpr std::string_view context_json =
            R"({"amount":"1","currency":"USD","payment_method":"card","country":"US","connector":""})";
        try {
            const auto result =
                m_context->decision_engine().evaluate_checked(program, context_json);
            if (!result.has_value()) {
                m_probe_error = result.error().get_message();
                return State::Error;
            }
            return State::Running;
        } catch (const std::exception& error) {
            m_probe_error = error.what();
            return State::Error;
        } catch (...) {
            m_probe_error = "unknown decision engine error";
            return State::Error;
        }
    }

    [[nodiscard]] app::dto::health_check::HealthState probe_outgoing()
    {
        using State = app::dto::health_check::HealthState;
        try {
            const auto connector = m_context->connector_router().select("health_check");
            if (!connector) {
                return State::NotApplicable;
            }
            const auto response = connector->sync(
                app::connector::ConnectorRequestBuilder{}
                    .operation("health_check")
                    .merchant_id("health_check")
                    .raw_payload("health_check")
                    ()
            );
            if (response.get_status().empty()) {
                m_probe_error = "outgoing connector probe returned no status";
                return State::Error;
            }
            return State::Running;
        } catch (const std::exception& error) {
            m_probe_error = error.what();
            return State::Error;
        } catch (...) {
            m_probe_error = "unknown outgoing probe error";
            return State::Error;
        }
    }

    [[nodiscard]] app::dto::health_check::HealthState probe_unified_connector_service()
    {
        using State = app::dto::health_check::HealthState;
        // Superposition/ConfigStore is the local substitute for the configured
        // UCS gRPC client. Absent record == client not configured == NotApplicable.
        const auto client =
            m_context->superposition().get_config("grpc_clients", "unified_connector_service");
        if (!client.has_value()) {
            return State::NotApplicable;
        }
        if constexpr (!kDynamicRouting) {
            // Upstream non-dynamic_routing fallback: configured client => Running.
            return State::Running;
        }
        const auto& value = client->get_value();
        if (value == "NOT_SERVING") {
            m_probe_error = "Unified Connector Service reported NOT_SERVING";
            return State::Error;
        }
        // SERVING (or any configured client state) is healthy.
        return State::Running;
    }

    // ───────────────────────────── Wire encoding ───────────────────────────────

    /**
     * Emits the exact RouterHealthCheckResponse JSON. Field order matches
     * api_models/health_check.rs:3-18. olap/dynamic_routing fields are gated by
     * the local constexpr feature flags; the two Option fields are omitted when
     * None, mirroring `#[serde(skip_serializing_if = "Option::is_none")]`.
     */
    [[nodiscard]] std::string encode_health_response(
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
        const auto word = [](bool value) constexpr -> std::string_view
        {
            return value ? "true" : "false";
        };

        std::string out{"{"};
        out += std::format("\"database\":{}", word(database));
        out += std::format(",\"redis\":{}", word(redis));
        if (vault.has_value()) {
            out += std::format(",\"vault\":{}", word(*vault));
        }
        if constexpr (kOlap) {
            out += std::format(",\"analytics\":{}", word(analytics));
            out += std::format(",\"opensearch\":{}", word(opensearch));
        }
        out += std::format(",\"outgoing_request\":{}", word(outgoing));
        if constexpr (kDynamicRouting) {
            out += ",\"grpc_health_check\":{";
            bool first = true;
            for (const auto& [service, healthy] : grpc_map) {
                if (!first) {
                    out += ',';
                }
                first = false;
                out += std::format("\"{}\":{}", json_escape(service), word(healthy));
            }
            out += '}';
            out += std::format(",\"decision_engine\":{}", word(decision_engine));
        }
        if (unified_connector_service.has_value()) {
            out += std::format(
                ",\"unified_connector_service\":{}",
                word(*unified_connector_service)
            );
        }
        out += '}';
        return out;
    }

    /**
     * Exact ApiErrorResponse::HealthCheckError wire shape. Upstream converts it
     * to ApiErrorResponse::InternalServerError with code HE_00 and message
     * "{component} health check failed with error: {message}"
     * (hyperswitch_domain_models/errors/api_error_response.rs:60-64,425-427),
     * which serializes as {"type":"api","message":...,"code":"HE_00"} with
     * error_type "api" (api_models/errors/types.rs:60-70,159-176) and HTTP 500.
     */
    void send_health_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view component,
        std::string_view message
    ) const
    {
        const auto body = std::format(
            R"({{"type":"api","message":"{} health check failed with error: {}","code":"HE_00"}})",
            json_escape(component),
            json_escape(message)
        );
        auto bytes = serde::Ser::serialize_raw("application/json", body);
        res.set_status(interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
        res.set_body(std::move(bytes));
        send();
    }

    [[nodiscard]] std::string json_escape(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char ch : value) {
            switch (ch) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(ch); break;
            }
        }
        return out;
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
        std::string_view text
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(interfaces::io::types::Status::OK);
        res.set_body(std::move(bytes));
        send();
    }

    void send_text(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok = true
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("text/plain", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
    std::string m_probe_error{};
};

} // namespace app::handlers::health
