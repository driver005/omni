export module app_store:configs_runtime_config_key_retrieve;

import std;
import core_error;
import app_context;
import app_context_core;

import :configs_input;
import :configs_result;
import :configs_runtime_common;

export namespace app::handlers::configs {

class ConfigKeyRetrieveRuntime
{
public:
    explicit ConfigKeyRetrieveRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** Upstream `config_key_retrieve` -> `read_config` (core/configs.rs:36-44). */
    [[nodiscard]] std::expected<ConfigKeyRetrieveResult, core::error::AppError>
    retrieve(const ConfigKeyRetrieveInput& input) const
    {
        // read_config: store.find_config_by_key_optional(key)
        // .to_not_found_response(ConfigNotFound)?.ok_or(ConfigNotFound)?
        const auto record = m_context.repository().find_config_by_key(input.config_key);
        if (!record) {
            return std::unexpected{m_support.config_not_found_error()};
        }

        m_support.record_flow("ConfigKeyFetch");

        // Json(config.foreign_into()) uses the stored key/value.
        ConfigKeyRetrieveResult result{};
        result.set_key(record->get_config_key());
        result.set_value(record->get_config_value());
        return result;
    }

private:
    ::app::context::AppContext& m_context;
    ConfigsRuntimeSupport m_support;
};

} // namespace app::handlers::configs
