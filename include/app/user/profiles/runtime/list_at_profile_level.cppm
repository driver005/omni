export module app_user:profiles_runtime_list_at_profile_level;

import std;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_metrics;

import :profiles_input;
import :profiles_result;
import :profiles_runtime_common;

export namespace app::handlers::profiles {

// profiles.rs:471-476 — list_profile(merchant, auth.profile.*).
class ProfilesListAtProfileLevelRuntime
{
public:
    explicit ProfilesListAtProfileLevelRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::vector<ProfileWireResponse>, core::error::AppError>
    list(const ProfilesListAtProfileLevelInput& input) const
    {
        auto result = m_support.list_profile(input.merchant_id, input.profile_id_list);
        if (!result) {
            return std::unexpected{result.error()};
        }
        m_context.metrics().record_flow("profiles.list_at_profile_level");
        m_context.metrics().record_flow("Flow::ProfileList");
        m_context.metrics().counter("profiles_list_at_profile_level").increment();
        return std::move(*result);
    }

private:
    ::app::context::AppContext& m_context;
    ProfilesRuntimeSupport m_support;
};

} // namespace app::handlers::profiles
