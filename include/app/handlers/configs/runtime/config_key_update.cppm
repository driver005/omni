export module app_handlers_configs:runtime_config_key_update;

import std;
import core_error;
import app_context;
import app_store;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::configs {

class ConfigKeyUpdateRuntime
{
public:
    explicit ConfigKeyUpdateRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** Upstream `config_key_update` -> `update_config` (core/configs.rs:46-56). */
    [[nodiscard]] std::expected<ConfigKeyUpdateResult, core::error::AppError>
    update(const ConfigKeyUpdateInput& input) const
    {
        // update_config: store.update_config_by_key(&key, update.foreign_into())
        // .to_not_found_response(ConfigNotFound)?
        auto existing = m_context.repository().find_config_by_key(input.config_key);
        if (!existing) {
            return std::unexpected{m_support.config_not_found_error()};
        }
        // foreign_into maps only `value` into the storage update; the path key wins.
        existing->set_config_value(input.request.get_value());
        if (!m_context.repository().update_config(*existing)) {
            // A zero-row update is the local equivalent of the store's not-found error.
            return std::unexpected{m_support.config_not_found_error()};
        }

        m_support.record_flow("ConfigKeyUpdate");

        // Json(config.foreign_into()) of the updated row.
        ConfigKeyUpdateResult result{};
        result.set_key(existing->get_config_key());
        result.set_value(existing->get_config_value());
        return result;
    }

private:
    app::context::AppContext& m_context;
    ConfigsRuntimeSupport m_support;
};

} // namespace app::handlers::configs
