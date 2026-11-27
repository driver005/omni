export module app_routing:gsm_runtime;

import std;
import app_dto;
import core_error;
import app_context;
import core_behavior;

import :gsm_result;
import :gsm_runtime_common;
import :gsm_runtime_create;
import :gsm_runtime_get;
import :gsm_runtime_update;
import :gsm_runtime_delete;

export namespace app::handlers::gsm {

class GsmRuntime
{
public:
    explicit GsmRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_get{context},
        m_update{context},
        m_delete{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::gsm::GsmResponse> create(
        const ::app::dto::gsm::GsmCreateRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_create.create(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::gsm::GsmResponse>
    get(const ::app::dto::gsm::GsmRetrieveRequest& request,
        const ::core::behavior::CallContext& ctx) const
    {
        return m_get.get(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::gsm::GsmResponse> update(
        const ::app::dto::gsm::GsmUpdateRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_update.update(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::gsm::GsmDeleteResponse> remove(
        const ::app::dto::gsm::GsmDeleteRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_delete.remove(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    GsmCreateRuntime m_create;
    GsmGetRuntime m_get;
    GsmUpdateRuntime m_update;
    GsmDeleteRuntime m_delete;
};

} // namespace app::handlers::gsm
