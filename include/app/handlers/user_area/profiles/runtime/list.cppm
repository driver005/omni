export module app_user:profiles_runtime_list;

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

// core/admin.rs:4122-4150 — list_profile(merchant, None).
class ProfilesListRuntime
{
public:
    explicit ProfilesListRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ProfilesListResult, core::error::AppError>
    list(const ProfilesListInput& input) const
    {
        auto result = m_support.list_profile(input.merchant_id, input.profile_id_list);
        if (!result) {
            return std::unexpected{result.error()};
        }
        m_context.metrics().record_flow("profiles.list");
        m_context.metrics().record_flow("Flow::ProfileList");
        m_context.metrics().counter("profiles_list").increment();
        return ProfilesListResult{std::move(*result)};
    }

private:
    ::app::context::AppContext& m_context;
    ProfilesRuntimeSupport m_support;
};

} // namespace app::handlers::profiles
