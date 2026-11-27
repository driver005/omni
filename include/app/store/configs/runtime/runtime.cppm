export module app_store:configs_runtime;

import std;
import core_behavior;
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

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    create(const ConfigRequestWire& dto, const core::behavior::CallContext& ctx) const
    {
        return m_create.create(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve(const core::behavior::CallContext& ctx) const
    {
        return m_retrieve.retrieve(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    update(const ConfigUpdateRequestWire& dto, const core::behavior::CallContext& ctx) const
    {
        return m_update.update(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    remove(const core::behavior::CallContext& ctx) const
    {
        return m_delete.remove(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    ConfigKeyCreateRuntime m_create;
    ConfigKeyRetrieveRuntime m_retrieve;
    ConfigKeyUpdateRuntime m_update;
    ConfigKeyDeleteRuntime m_delete;
};

} // namespace app::handlers::configs
