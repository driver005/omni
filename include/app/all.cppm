export module app_handlers:all;

import std;
import app_context;
import core_cache;
import core_error;
import core_database;
import core_workflow;
import memory_backends;
import app_auth;
import app_payment;
import app_user;
import app_store;
import app_routing;
import app_webhook;

export namespace app::api {

class AppHandlers
{
public:
    AppHandlers() = default;

    [[nodiscard]] app::api::AuthHandlers& auth() noexcept
    {
        return m_auth;
    }

    [[nodiscard]] app::api::PaymentHandlers& payment() noexcept
    {
        return m_payment;
    }

    [[nodiscard]] app::api::UserHandlers& user() noexcept
    {
        return m_user;
    }

    [[nodiscard]] app::api::StoreHandlers& store() noexcept
    {
        return m_store;
    }

    [[nodiscard]] app::api::RoutingHandlers& routing() noexcept
    {
        return m_routing;
    }

    [[nodiscard]] app::api::WebhookHandlers& webhook() noexcept
    {
        return m_webhook;
    }

private:
    app::context::AppContext m_context{
        app::context::AppContextBuilder{}
            .database(
                core::db::DatabaseBuilder{}
                    .use_backend(std::make_unique<core::db::InMemoryBackend>())
                    .build()
            )
            .cache(
                core::cache::CacheBuilder{}
                    .use_backend(std::make_unique<core::cache::LocalCacheBackend>())
                    .build()
            )
            .workflow(
                [&]
                {
                    core::workflow::Workflow workflow;
                    workflow.use_backend(std::make_unique<core::workflow::LocalWorkflowDriver>());
                    return workflow;
                }()
            )
            .build()
    };
    app::api::AuthHandlers m_auth{m_context};
    app::api::PaymentHandlers m_payment{m_context};
    app::api::UserHandlers m_user{m_context};
    app::api::StoreHandlers m_store{m_context};
    app::api::RoutingHandlers m_routing{m_context};
    app::api::WebhookHandlers m_webhook{m_context};
};

} // namespace app::api
