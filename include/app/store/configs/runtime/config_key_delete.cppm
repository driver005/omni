export module app_store:configs_runtime_config_key_delete;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;
import app_context_core;

import :configs_result;
import :configs_runtime_common;

export namespace app::handlers::configs {

class ConfigKeyDeleteRuntime
{
public:
    explicit ConfigKeyDeleteRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** Upstream `config_key_delete` -> `config_delete` (core/configs.rs:58-65). */
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    remove(const core::behavior::CallContext& ctx) const
    {
        // config_delete: store.delete_config_by_key(&key)
        // .to_not_found_response(ConfigNotFound)?
        const auto record = m_context.repository().find_config_by_key(m_support.config_key(ctx));
        if (!record) {
            return std::unexpected{m_support.config_not_found_error()};
        }
        if (!m_context.repository().remove_config(record->get_id())) {
            return std::unexpected{m_support.config_not_found_error()};
        }

        m_support.record_flow("ConfigKeyDelete");

        // Json(config.foreign_into()) of the deleted row.
        serde::Value::Object document{};
        document.insert(
            std::string{config_key_wire_fields()[0]},
            serde::Value{record->get_config_key()}
        );
        document.insert(
            std::string{config_key_wire_fields()[1]},
            serde::Value{record->get_config_value()}
        );
        return serde::Value{std::move(document)};
    }

private:
    ::app::context::AppContext& m_context;
    ConfigsRuntimeSupport m_support;
};

} // namespace app::handlers::configs
