export module app_routing:superposition_sdk_config_runtime;

import std;
import app_dto;
import core_error;
import app_context;

import :superposition_sdk_config_input;
import :superposition_sdk_config_result;
import :superposition_sdk_config_runtime_common;
import :superposition_sdk_config_runtime_get_sdk_config;
import :superposition_sdk_config_runtime_get_profile_sdk_config;

export namespace app::handlers::superposition_sdk_config {

class SuperpositionSdkConfigRuntime
{
public:
    explicit SuperpositionSdkConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_get_sdk_config{context},
        m_get_profile_sdk_config{context}
    {
    }

    [[nodiscard]] std::expected<
        ::app::dto::superposition_sdk_config::SuperPositionConfigResponse,
        core::error::AppError>
    get_sdk_config(const GetSdkConfigInput& input) const
    {
        return m_get_sdk_config.get_sdk_config(input);
    }

    [[nodiscard]] std::expected<
        ::app::dto::superposition_sdk_config::SuperPositionConfigResponse,
        core::error::AppError>
    get_profile_sdk_config(const GetProfileSdkConfigInput& input) const
    {
        return m_get_profile_sdk_config.get_profile_sdk_config(input);
    }

private:
    ::app::context::AppContext& m_context;
    GetSdkConfigRuntime m_get_sdk_config;
    GetProfileSdkConfigRuntime m_get_profile_sdk_config;
};

} // namespace app::handlers::superposition_sdk_config
