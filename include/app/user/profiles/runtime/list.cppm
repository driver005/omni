export module app_user:profiles_runtime_list;

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

// core/admin.rs:4122-4150 — list_profile(merchant, None).
class ProfilesListRuntime
{
public:
    explicit ProfilesListRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list(const ::core::behavior::CallContext& ctx) const
    {
        auto result =
            m_support.list_profile(ctx_string(ctx, "merchant_id"), ctx_ids(ctx, "profile_id_list"));
        if (!result) {
            return std::unexpected{result.error()};
        }
        m_context.logger().record_flow("profiles.list");
        m_context.logger().record_flow("Flow::ProfileList");
        m_context.logger().counter("profiles_list").increment();
        serde::Value::Array array{};
        for (const auto& item: *result) {
            array.push_back(
                serde::Serializer<ProfileWireResponse>::build_value(item, kProfileWireFields)
            );
        }
        return serde::Value{std::move(array)};
    }

private:
    ::app::context::AppContext& m_context;
    ProfilesRuntimeSupport m_support;
};

} // namespace app::handlers::profiles
