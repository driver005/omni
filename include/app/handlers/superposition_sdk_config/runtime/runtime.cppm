export module app_handlers_superposition_sdk_config:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_get_sdk_config;
import :runtime_get_profile_sdk_config;

export namespace app::handlers::superposition_sdk_config {

class SuperpositionSdkConfigRuntime
{
public:
    explicit SuperpositionSdkConfigRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_get_sdk_config{context},
        m_get_profile_sdk_config{context}
    {
    }

    [[nodiscard]] std::expected<GetSdkConfigResult, core::error::AppError>
    get_sdk_config(const GetSdkConfigInput& input) const
    {
        return m_get_sdk_config.get_sdk_config(input);
    }

    [[nodiscard]] std::expected<GetProfileSdkConfigResult, core::error::AppError>
    get_profile_sdk_config(const GetProfileSdkConfigInput& input) const
    {
        return m_get_profile_sdk_config.get_profile_sdk_config(input);
    }

private:
    app::context::AppContext& m_context;
    GetSdkConfigRuntime m_get_sdk_config;
    GetProfileSdkConfigRuntime m_get_profile_sdk_config;
};

} // namespace app::handlers::superposition_sdk_config
