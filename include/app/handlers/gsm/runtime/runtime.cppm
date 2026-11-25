export module app_handlers_gsm:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_create;
import :runtime_get;
import :runtime_update;
import :runtime_delete;

export namespace app::handlers::gsm {

class GsmRuntime
{
public:
    explicit GsmRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_get{context},
        m_update{context},
        m_delete{context}
    {
    }

    [[nodiscard]] std::expected<CreateGsmRuleResult, core::error::AppError>
    create(const CreateGsmRuleInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<GetGsmRuleResult, core::error::AppError>
    get(const GetGsmRuleInput& input) const
    {
        return m_get.get(input);
    }

    [[nodiscard]] std::expected<UpdateGsmRuleResult, core::error::AppError>
    update(const UpdateGsmRuleInput& input) const
    {
        return m_update.update(input);
    }

    [[nodiscard]] std::expected<DeleteGsmRuleResult, core::error::AppError>
    remove(const DeleteGsmRuleInput& input) const
    {
        return m_delete.remove(input);
    }

private:
    app::context::AppContext& m_context;
    GsmCreateRuntime m_create;
    GsmGetRuntime m_get;
    GsmUpdateRuntime m_update;
    GsmDeleteRuntime m_delete;
};

} // namespace app::handlers::gsm
