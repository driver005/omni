export module app_store:store_handler;

import std;
import app_context;
import core_router;

import :files_base;
import :cache_base;
import :configs_base;
import :health_base;
import :metrics_base;
import :process_tracker_base;
import :lock_utils_base;
import :app_base;
import :proxy_base;
import :currency_base;

export namespace app::api {

class StoreHandlers
{
public:
    explicit StoreHandlers(::app::context::AppContext& ctx) noexcept :
        m_context{ctx},
        m_app{ctx},
        m_cache{ctx},
        m_configs{ctx},
        m_currency{ctx},
        m_files{ctx},
        m_health{ctx},
        m_lock_utils{ctx},
        m_metrics{ctx},
        m_process_tracker{ctx},
        m_proxy{ctx}
    {
    }

    void install(core::router::RouterContext<>& router)
    {
        m_app.install_routes(router);
        m_cache.install_routes(router);
        m_configs.install_routes(router);
        m_currency.install_routes(router);
        m_files.install_routes(router);
        m_health.install_routes(router);
        m_lock_utils.install_routes(router);
        m_metrics.install_routes(router);
        m_process_tracker.install_routes(router);
        m_proxy.install_routes(router);
    }

private:
    ::app::context::AppContext& m_context;
    ::app::handlers::app::AppRouteHandlers m_app;
    ::app::handlers::cache::CacheRouteHandlers m_cache;
    ::app::handlers::configs::ConfigsRouteHandlers m_configs;
    ::app::handlers::currency::CurrencyRouteHandlers m_currency;
    ::app::handlers::files::FilesRouteHandlers m_files;
    ::app::handlers::health::HealthRouteHandlers m_health;
    ::app::handlers::lock_utils::LockUtilsRouteHandlers m_lock_utils;
    ::app::handlers::metrics::MetricsRouteHandlers m_metrics;
    ::app::handlers::process_tracker::ProcessTrackerRouteHandlers m_process_tracker;
    ::app::handlers::proxy::ProxyRouteHandlers m_proxy;
};

} // namespace app::api
