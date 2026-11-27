export module app_store:health_runtime_health;

import std;
import core_error;
import core_serde;
import app_context;
import app_metrics;

import :health_input;

export namespace app::handlers::health {

class HealthTextRuntime
{
public:
    explicit HealthTextRuntime(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Upstream routes::health::health(): no body, no auth. HEALTH_METRIC.add(1, &[])
    // (routes/health.rs:17) is the local metrics substitute, then the exact
    // plain-text "health is good" body.
    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    health(const HealthInput& input) const
    {
        (void)input;
        m_context.metrics().record_flow("health");
        m_context.metrics().counter("health").increment();
        return serde::Value{std::string{"health is good"}};
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::health
