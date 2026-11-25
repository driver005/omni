export module app_handlers_mandates:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_get_mandate;
import :runtime_revoke_mandate;
import :runtime_retrieve_mandates_list;

export namespace app::handlers::mandates {

class MandatesRuntime
{
public:
    explicit MandatesRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_get_mandate{context},
        m_revoke_mandate{context},
        m_retrieve_mandates_list{context}
    {
    }

    [[nodiscard]] std::expected<GetMandateResult, core::error::AppError>
    get_mandate(const GetMandateInput& input) const
    {
        return m_get_mandate.get_mandate(input);
    }

    [[nodiscard]] std::expected<RevokeMandateResult, core::error::AppError>
    revoke_mandate(const RevokeMandateInput& input) const
    {
        return m_revoke_mandate.revoke_mandate(input);
    }

    [[nodiscard]] std::expected<RetrieveMandatesListResult, core::error::AppError>
    retrieve_mandates_list(const RetrieveMandatesListInput& input) const
    {
        return m_retrieve_mandates_list.retrieve_mandates_list(input);
    }

private:
    app::context::AppContext& m_context;
    GetMandateRuntime m_get_mandate;
    RevokeMandateRuntime m_revoke_mandate;
    RetrieveMandatesListRuntime m_retrieve_mandates_list;
};

} // namespace app::handlers::mandates
