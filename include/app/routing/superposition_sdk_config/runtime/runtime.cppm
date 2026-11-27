export module app_routing:superposition_sdk_config_runtime;

import std;
import app_dto;
import core_error;
import app_context;
import core_behavior;

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

    [[nodiscard]] core::behavior::Outcome<
        ::app::dto::superposition_sdk_config::SuperPositionConfigResponse>
    get_sdk_config(const ::core::behavior::CallContext& ctx) const
    {
        return m_get_sdk_config.get_sdk_config(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<
        ::app::dto::superposition_sdk_config::SuperPositionConfigResponse>
    get_profile_sdk_config(const ::core::behavior::CallContext& ctx) const
    {
        return m_get_profile_sdk_config.get_profile_sdk_config(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    GetSdkConfigRuntime m_get_sdk_config;
    GetProfileSdkConfigRuntime m_get_profile_sdk_config;
};

} // namespace app::handlers::superposition_sdk_config
