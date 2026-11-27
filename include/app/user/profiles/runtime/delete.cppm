export module app_user:profiles_runtime_delete;

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

// core/admin.rs:4182-4196 — delete_profile.
class ProfileDeleteRuntime
{
public:
    explicit ProfileDeleteRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    remove(const ::core::behavior::CallContext& ctx) const
    {
        if (!m_support.delete_profile_by_profile_id_merchant_id(
                ctx_string(ctx, "profile_id"),
                ctx_string(ctx, "merchant_id")
            )) {
            return std::unexpected{m_support.profile_not_found(ctx_string(ctx, "profile_id"))};
        }
        m_support.emit_event("delete", ctx_string(ctx, "profile_id"));
        m_context.logger().record_flow("profiles.delete");
        m_context.logger().record_flow("Flow::ProfileDelete");
        m_context.logger().counter("profiles_delete").increment();
        return serde::Value{true};
    }

private:
    ::app::context::AppContext& m_context;
    ProfilesRuntimeSupport m_support;
};

} // namespace app::handlers::profiles
