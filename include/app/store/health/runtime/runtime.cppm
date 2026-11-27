export module app_store:health_runtime;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;

import :health_runtime_common;
import :health_runtime_health;
import :health_runtime_deep_health_check;

export namespace app::handlers::health {

class HealthRuntime
{
public:
    explicit HealthRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_health{context},
        m_deep_health_check{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    health(const core::behavior::CallContext& ctx) const
    {
        return m_health.health(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    deep_health_check(const core::behavior::CallContext& ctx) const
    {
        return m_deep_health_check.deep_health_check(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    HealthTextRuntime m_health;
    DeepHealthCheckRuntime m_deep_health_check;
};

} // namespace app::handlers::health
