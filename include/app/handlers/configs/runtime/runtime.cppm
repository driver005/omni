export module app_handlers_configs:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_config_key_create;
import :runtime_config_key_retrieve;
import :runtime_config_key_update;
import :runtime_config_key_delete;

export namespace app::handlers::configs {

class ConfigsRuntime
{
public:
    explicit ConfigsRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_retrieve{context},
        m_update{context},
        m_delete{context}
    {
    }

    [[nodiscard]] std::expected<ConfigKeyCreateResult, core::error::AppError>
    create(const ConfigKeyCreateInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<ConfigKeyRetrieveResult, core::error::AppError>
    retrieve(const ConfigKeyRetrieveInput& input) const
    {
        return m_retrieve.retrieve(input);
    }

    [[nodiscard]] std::expected<ConfigKeyUpdateResult, core::error::AppError>
    update(const ConfigKeyUpdateInput& input) const
    {
        return m_update.update(input);
    }

    [[nodiscard]] std::expected<ConfigKeyDeleteResult, core::error::AppError>
    remove(const ConfigKeyDeleteInput& input) const
    {
        return m_delete.remove(input);
    }

private:
    app::context::AppContext& m_context;
    ConfigKeyCreateRuntime m_create;
    ConfigKeyRetrieveRuntime m_retrieve;
    ConfigKeyUpdateRuntime m_update;
    ConfigKeyDeleteRuntime m_delete;
};

} // namespace app::handlers::configs
