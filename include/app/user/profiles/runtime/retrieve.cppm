export module app_user:profiles_runtime_retrieve;

import std;
import core_behavior;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_metrics;

import :profiles_input;
import :profiles_result;
import :profiles_runtime_common;

export namespace app::handlers::profiles {

// core/admin.rs:4152-4180 — retrieve_profile.
class ProfileRetrieveRuntime
{
public:
    explicit ProfileRetrieveRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<ProfileWireResponse>
    retrieve(const ::core::behavior::CallContext& ctx) const
    {
        const auto profile = m_support.find_business_profile_by_merchant_id_profile_id(
            ctx_string(ctx, "merchant_id"),
            ctx_string(ctx, "profile_id")
        );
        if (!profile) {
            return std::unexpected(m_support.profile_not_found(ctx_string(ctx, "profile_id")));
        }
        // core/admin.rs:4167-4179 — cache hits preserve the DB ownership constraint.
        if (profile->get_merchant_id() != ctx_string(ctx, "merchant_id") &&
            !ctx_string(ctx, "merchant_id").empty()) {
            return std::unexpected(m_support.profile_not_found(ctx_string(ctx, "profile_id")));
        }
        m_context.logger().record_flow("profiles.retrieve");
        m_context.logger().record_flow("Flow::ProfileRetrieve");
        m_context.logger().counter("profiles_retrieve").increment();
        return std::move(*profile);
    }

private:
    ::app::context::AppContext& m_context;
    ProfilesRuntimeSupport m_support;
};

} // namespace app::handlers::profiles
