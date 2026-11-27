export module app_store:health_runtime;

import std;
import core_error;
import core_serde;
import app_context;

import :health_input;
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

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    health(const HealthInput& input) const
    {
        return m_health.health(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    deep_health_check(const DeepHealthCheckInput& input) const
    {
        return m_deep_health_check.deep_health_check(input);
    }

private:
    ::app::context::AppContext& m_context;
    HealthTextRuntime m_health;
    DeepHealthCheckRuntime m_deep_health_check;
};

} // namespace app::handlers::health
