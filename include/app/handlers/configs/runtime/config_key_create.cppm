export module app_handlers_configs:runtime_config_key_create;

import std;
import core_error;
import app_context;
import app_store;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::configs {

class ConfigKeyCreateRuntime
{
public:
    explicit ConfigKeyCreateRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** Upstream `config_key_create` -> `set_config` (core/configs.rs:22-34). */
    [[nodiscard]] std::expected<ConfigKeyCreateResult, core::error::AppError>
    create(const ConfigKeyCreateInput& input) const
    {
        // set_config: store.insert_config(ConfigNew { key: config.key,
        // config: config.value }).to_duplicate_response(DuplicateConfig).
        app::store::configs::ConfigRecord record{};
        record.set_config_key(input.request.get_key());
        record.set_config_value(input.request.get_value());
        auto created = m_context.repository().create_config(record);
        if (!created) {
            return std::unexpected{m_support.duplicate_config_error()};
        }

        // #[instrument(flow = ?Flow::CreateConfigKey)] — LockAction::NotApplicable, no lock.
        m_support.record_flow("CreateConfigKey");

        // Json(config.foreign_into()) emits the stored row as {key, value}.
        ConfigKeyCreateResult result{};
        result.set_key(created->get_config_key());
        result.set_value(created->get_config_value());
        return result;
    }

private:
    app::context::AppContext& m_context;
    ConfigsRuntimeSupport m_support;
};

} // namespace app::handlers::configs
