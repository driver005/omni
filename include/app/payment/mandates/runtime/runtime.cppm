export module app_payment:mandates_runtime;
import core_behavior;

import std;
import core_error;
import app_context;

import :mandates_input;
import :mandates_result;
import :mandates_runtime_common;
import :mandates_runtime_get_mandate;
import :mandates_runtime_revoke_mandate;
import :mandates_runtime_retrieve_mandates_list;

export namespace app::handlers::mandates {

class MandatesRuntime
{
public:
    explicit MandatesRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_get_mandate{context},
        m_revoke_mandate{context},
        m_retrieve_mandates_list{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::mandates::MandateResponse>
    get_mandate(const core::behavior::CallContext& ctx) const
    {
        return m_get_mandate.get_mandate(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::model::mandates::MandateRevokedResponse>
    revoke_mandate(const core::behavior::CallContext& ctx) const
    {
        return m_revoke_mandate.revoke_mandate(ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<std::vector<::app::model::mandates::MandateResponse>>
    retrieve_mandates_list(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_retrieve_mandates_list.retrieve_mandates_list(dto, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    GetMandateRuntime m_get_mandate;
    RevokeMandateRuntime m_revoke_mandate;
    RetrieveMandatesListRuntime m_retrieve_mandates_list;
};

} // namespace app::handlers::mandates
