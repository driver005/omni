export module app_store:configs_runtime_config_key_update;

import std;
import core_error;
import core_serde;
import app_context;
import app_context_core;

import :configs_input;
import :configs_result;
import :configs_runtime_common;

export namespace app::handlers::configs {

class ConfigKeyUpdateRuntime
{
public:
    explicit ConfigKeyUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** Upstream `config_key_update` -> `update_config` (core/configs.rs:46-56). */
    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
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
        serde::Value::Object document{};
        document.insert(
            std::string{config_key_wire_fields()[0]},
            serde::Value{existing->get_config_key()}
        );
        document.insert(
            std::string{config_key_wire_fields()[1]},
            serde::Value{existing->get_config_value()}
        );
        return serde::Value{std::move(document)};
    }

private:
    ::app::context::AppContext& m_context;
    ConfigsRuntimeSupport m_support;
};

} // namespace app::handlers::configs
