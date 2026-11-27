export module app_store:configs_runtime;

import std;
import core_error;
import core_serde;
import app_context;

import :configs_input;
import :configs_result;
import :configs_runtime_common;
import :configs_runtime_config_key_create;
import :configs_runtime_config_key_retrieve;
import :configs_runtime_config_key_update;
import :configs_runtime_config_key_delete;

export namespace app::handlers::configs {

class ConfigsRuntime
{
public:
    explicit ConfigsRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_retrieve{context},
        m_update{context},
        m_delete{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    create(const ConfigKeyCreateInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    retrieve(const ConfigKeyRetrieveInput& input) const
    {
        return m_retrieve.retrieve(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    update(const ConfigKeyUpdateInput& input) const
    {
        return m_update.update(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    remove(const ConfigKeyDeleteInput& input) const
    {
        return m_delete.remove(input);
    }

private:
    ::app::context::AppContext& m_context;
    ConfigKeyCreateRuntime m_create;
    ConfigKeyRetrieveRuntime m_retrieve;
    ConfigKeyUpdateRuntime m_update;
    ConfigKeyDeleteRuntime m_delete;
};

} // namespace app::handlers::configs
