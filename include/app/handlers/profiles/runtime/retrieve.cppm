export module app_handlers_profiles:runtime_retrieve;

import std;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_metrics;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::profiles {

// core/admin.rs:4152-4180 — retrieve_profile.
class ProfileRetrieveRuntime
{
public:
    explicit ProfileRetrieveRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ProfileRetrieveResult, core::error::AppError>
    retrieve(const ProfileRetrieveInput& input) const
    {
        const auto profile = m_support.find_business_profile_by_merchant_id_profile_id(
            input.merchant_id,
            input.profile_id
        );
        if (!profile) {
            return std::unexpected(m_support.profile_not_found(input.profile_id));
        }
        // core/admin.rs:4167-4179 — cache hits preserve the DB ownership constraint.
        if (profile->get_merchant_id() != input.merchant_id && !input.merchant_id.empty()) {
            return std::unexpected(m_support.profile_not_found(input.profile_id));
        }
        m_context.metrics().record_flow("profiles.retrieve");
        m_context.metrics().record_flow("Flow::ProfileRetrieve");
        m_context.metrics().counter("profiles_retrieve").increment();
        return ProfileRetrieveResult{std::move(*profile)};
    }

private:
    app::context::AppContext& m_context;
    ProfilesRuntimeSupport m_support;
};

} // namespace app::handlers::profiles
