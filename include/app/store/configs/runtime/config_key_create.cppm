export module app_store:configs_runtime_config_key_create;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;
import app_context_core;

import :configs_input;
import :configs_result;
import :configs_runtime_common;

export namespace app::handlers::configs {

class ConfigKeyCreateRuntime
{
public:
    explicit ConfigKeyCreateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** Upstream `config_key_create` -> `set_config` (core/configs.rs:22-34). */
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    create(const ConfigRequestWire& dto, const core::behavior::CallContext& ctx) const
    {
        (void)ctx;
        // set_config: store.insert_config(ConfigNew { key: config.key,
        // config: config.value }).to_duplicate_response(DuplicateConfig).
        ::app::store::configs::ConfigRecord record{};
        record.set_config_key(dto.get_key());
        record.set_config_value(dto.get_value());
        auto created = m_context.repository().create_config(record);
        if (!created) {
            return std::unexpected{m_support.duplicate_config_error()};
        }

        // #[instrument(flow = ?Flow::CreateConfigKey)] — LockAction::NotApplicable, no lock.
        m_support.record_flow("CreateConfigKey");

        // Json(config.foreign_into()) emits the stored row as {key, value}.
        serde::Value::Object document{};
        document.insert(
            std::string{config_key_wire_fields()[0]},
            serde::Value{created->get_config_key()}
        );
        document.insert(
            std::string{config_key_wire_fields()[1]},
            serde::Value{created->get_config_value()}
        );
        return serde::Value{std::move(document)};
    }

private:
    ::app::context::AppContext& m_context;
    ConfigsRuntimeSupport m_support;
};

} // namespace app::handlers::configs
