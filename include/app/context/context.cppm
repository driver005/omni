module;
export module app_context:context;

import std;
import core_cache;
import core_database;
import core_workflow;
import app_auth;
import app_connector;
import app_crypto;
import app_db;
import app_events;
import app_http;
import app_keystore;
import app_locking;
import app_metrics;
import app_services;
import app_store;
import app_workflow;
import :serde;

export namespace app::context {

/** @brief Internal state owned by AppContext (pImpl). */
class AppContextState
{
public:
    AppContextState(
        core::db::Database database,
        core::cache::Cache cache,
        core::workflow::Workflow workflow
    ) :
        m_database{std::move(database)},
        m_cache{std::move(cache)},
        m_workflow{std::move(workflow)},
        m_metrics{},
        m_authorizer{},
        m_connector_router{},
        m_ids{},
        m_auth_rules{},
        m_key_store{m_database},
        m_key_resolver{m_key_store},
        m_one_time_store{m_database},
        m_trackers{m_database, m_metrics},
        m_events{m_database, m_metrics},
        m_webhook_delivery{m_database, m_metrics, m_trackers, m_events},
        m_locks{m_cache},
        m_config_store{app::store::AppRepository{m_database}},
        m_decision_engine{},
        m_superposition{m_config_store}
    {
    }

    core::db::Database m_database{};
    core::cache::Cache m_cache;
    core::workflow::Workflow m_workflow{};
    app::metrics::MetricsRegistry m_metrics{};
    app::auth::AppAuthorizer m_authorizer{};
    app::connector::ConnectorRouter m_connector_router{};
    app::crypto::IdGenerator m_ids{};
    app::auth::RouteAuthRegistry m_auth_rules{};
    app::keystore::KeyStore m_key_store;
    app::keystore::KeyStoreSecretResolver m_key_resolver;
    app::keystore::RepositoryOneTimeStateStore m_one_time_store;
    app::workflow::ProcessTrackerService m_trackers;
    app::events::EventBus m_events;
    app::events::WebhookDeliveryService m_webhook_delivery;
    app::locking::LockRegistry m_locks;
    app::services::ConfigStore m_config_store;
    app::services::DecisionEngine m_decision_engine{};
    app::services::SuperpositionClient m_superposition;
};

/** @brief Core service facade shared by every app domain handler. */
class AppContext
{
public:
    AppContext() = default;

    [[nodiscard]] core::db::Database& database() noexcept
    {
        return m_state->m_database;
    }

    [[nodiscard]] core::cache::Cache& cache() noexcept
    {
        return m_state->m_cache;
    }

    [[nodiscard]] core::workflow::Workflow& workflow() noexcept
    {
        return m_state->m_workflow;
    }

    [[nodiscard]] app::store::AppRepository repository() noexcept
    {
        return app::store::AppRepository{m_state->m_database};
    }

    [[nodiscard]] app::auth::AppAuthorizer& authorizer() noexcept
    {
        return m_state->m_authorizer;
    }

    [[nodiscard]] app::auth::RouteAuthRegistry& auth_rules() noexcept
    {
        return m_state->m_auth_rules;
    }

    [[nodiscard]] app::connector::ConnectorRouter& connector_router() noexcept
    {
        return m_state->m_connector_router;
    }

    [[nodiscard]] app::metrics::MetricsRegistry& metrics() noexcept
    {
        return m_state->m_metrics;
    }

    [[nodiscard]] app::crypto::IdGenerator& ids() noexcept
    {
        return m_state->m_ids;
    }

    [[nodiscard]] app::keystore::KeyStore& key_store() noexcept
    {
        return m_state->m_key_store;
    }

    [[nodiscard]] app::crypto::ISecretResolver& secret_resolver() noexcept
    {
        return m_state->m_key_resolver;
    }

    [[nodiscard]] app::crypto::IOneTimeStateStore& one_time_store() noexcept
    {
        return m_state->m_one_time_store;
    }

    [[nodiscard]] app::workflow::ProcessTrackerService& process_trackers() noexcept
    {
        return m_state->m_trackers;
    }

    [[nodiscard]] app::events::EventBus& events() noexcept
    {
        return m_state->m_events;
    }

    [[nodiscard]] app::events::WebhookDeliveryService& webhook_delivery() noexcept
    {
        return m_state->m_webhook_delivery;
    }

    [[nodiscard]] app::locking::LockRegistry& locks() noexcept
    {
        return m_state->m_locks;
    }

    [[nodiscard]] app::services::ConfigStore& config_store() noexcept
    {
        return m_state->m_config_store;
    }

    [[nodiscard]] app::services::DecisionEngine& decision_engine() noexcept
    {
        return m_state->m_decision_engine;
    }

    [[nodiscard]] app::services::SuperpositionClient& superposition() noexcept
    {
        return m_state->m_superposition;
    }

private:
    friend class AppContextBuilder;
    explicit AppContext(std::shared_ptr<AppContextState> state) :
        m_state{std::move(state)}
    {
    }

    std::shared_ptr<AppContextState> m_state{};
};

/** @brief Only public entry point for building an AppContext. */
class AppContextBuilder
{
public:
    AppContextBuilder& database(core::db::Database value)
    {
        m_database = std::move(value);
        return *this;
    }

    AppContextBuilder& cache(core::cache::Cache value)
    {
        m_cache = std::move(value);
        return *this;
    }

    AppContextBuilder& workflow(core::workflow::Workflow value)
    {
        m_workflow = std::move(value);
        return *this;
    }

    [[nodiscard]] AppContext build()
    {
        SerdeSetup{}.install();
        auto database = std::move(m_database);
        app::db::MigrationRunner{app::db::SchemaRegistry{}}.migrate(database);
        return AppContext{std::make_shared<AppContextState>(
            std::move(database),
            std::move(*m_cache),
            std::move(m_workflow)
        )};
    }

private:
    core::db::Database m_database{};
    std::optional<core::cache::Cache> m_cache{};
    core::workflow::Workflow m_workflow{};
};

} // namespace app::context
