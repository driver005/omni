export module app_user:profile_acquirer_runtime;
import core_behavior;

import std;
import core_error;
import app_context;

import :profile_acquirer_input;
import :profile_acquirer_result;
import :profile_acquirer_runtime_common;
import :profile_acquirer_runtime_create;
import :profile_acquirer_runtime_update;

export namespace app::handlers::profile_acquirer {

class ProfileAcquirerRuntime
{
public:
    explicit ProfileAcquirerRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_update{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::profile_acquirer::ProfileAcquirerResponse>
    create(
        const ::app::dto::profile_acquirer::ProfileAcquirerCreate& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_create.create(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::profile_acquirer::ProfileAcquirerResponse>
    update(
        const ::app::dto::profile_acquirer::ProfileAcquirerUpdate& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_update.update(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    CreateProfileAcquirerRuntime m_create;
    ProfileAcquirerUpdateRuntime m_update;
};

} // namespace app::handlers::profile_acquirer
