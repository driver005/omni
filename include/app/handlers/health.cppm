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

class HealthRouteHandlers
{
public:
    void handle_health(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("health");
        const auto auth = m_context->authorizer().authorize_route(
            app::auth::AuthContextBuilder{}(req),
            rule
        );
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        m_context->metrics().record_flow("health");
        m_context->metrics().counter("health").increment();

        // Upstream routes::health::health(): no auth, plain-text liveness body,
        // HEALTH_METRIC incremented. Emit the exact upstream text body.
        send_text(res, send, "health is good");
    }

    void handle_deep_health_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("deep_health_check");
        const auto auth = m_context->authorizer().authorize_route(
            app::auth::AuthContextBuilder{}(req),
            rule
        );
        if (auth) {
            send_error(res, send, *auth);
            return;
        }

        m_context->metrics().record_flow("deep_health_check");
        m_context->metrics().counter("health").increment();

        const HealthStateProjection project{};
        using State = app::dto::health_check::HealthState;

        // Database: upstream store.health_check_db() -> Running/error. The local
        // substitute is a real database liveness check (is_open + a repository
        // query), with the failure mapped to a HealthCheckError{component,message}.
        const State database_state = probe_database();
        if (database_state == State::Error) {
            send_health_error(res, send, "Database", m_probe_error);
            return;
        }

        // Redis: set/get/delete cycle mirror of upstream health_check_redis().
        const State redis_state = probe_redis();
        if (redis_state == State::Error) {
            send_health_error(res, send, "Redis", m_probe_error);
            return;
        }

        // Locker/vault: upstream returns NotApplicable when the locker is disabled
        // or mocked. No locker is configured in this runtime, so the faithful state
        // is NotApplicable -> nullopt (impl From<HealthState> for Option<bool>).
        const State vault_state = State::NotApplicable;

        // Outgoing request: upstream calls call_connector_api(OUTGOING_CALL_URL).
        // Deterministic substitute: route a sync probe through ConnectorRouter; a
        // configured connector means reachable, no connector means NotApplicable.
        const State outgoing_state = probe_outgoing();

        // olap-gated components: no analytics/opensearch client is wired locally,
        // so NotApplicable -> false, matching upstream HealthState::into() when
        // the optional client is absent.
        const State analytics_state = State::NotApplicable;
        const State opensearch_state = State::NotApplicable;

        // dynamic_routing-gated components: no gRPC health client is wired locally.
        // gRPC map stays empty; decision engine and UCS report NotApplicable,
        // matching upstream fallbacks for an unconfigured client/feature.
        const State decision_state = State::NotApplicable;
        const State ucs_state = State::NotApplicable;

        app::dto::health_check::RouterHealthCheckResponse response;
        response.set_database(project.as_bool(database_state));
        response.set_redis(project.as_bool(redis_state));
        response.set_vault(project.as_optional(vault_state));
        response.set_analytics(project.as_bool(analytics_state));
        response.set_opensearch(project.as_bool(opensearch_state));
        response.set_outgoing_request(project.as_bool(outgoing_state));
        response.set_decision_engine(project.as_bool(decision_state));
        response.set_unified_connector_service(project.as_optional(ucs_state));
        // grpc_health_check: no gRPC client -> empty HealthCheckMap.

        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/health"}
            .add_route(
                Route{"/"}
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
                Route{"/ready"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_deep_health_check(req, res, std::move(send));
                    })
                    .summary("Hyperswitch deep_health_check")
            );

        // Upstream also exposes both probes under the /v2 scope (app.rs:822-829).
        Router{router, "/v2/health"}
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
                Route{"/ready"}
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

    void send_health_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view component,
        std::string_view message
    ) const
    {
        const auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INTERNAL_ERROR,
            std::format("HealthCheckError for {}: {}", component, message)
        );
        send_error(res, send, error);
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
