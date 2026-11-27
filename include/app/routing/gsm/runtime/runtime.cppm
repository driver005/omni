export module app_routing:gsm_runtime;

import std;
import app_dto;
import core_error;
import app_context;

import :gsm_input;
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

    [[nodiscard]] std::expected<::app::dto::gsm::GsmResponse, core::error::AppError>
    create(const CreateGsmRuleInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<::app::dto::gsm::GsmResponse, core::error::AppError>
    get(const GetGsmRuleInput& input) const
    {
        return m_get.get(input);
    }

    [[nodiscard]] std::expected<::app::dto::gsm::GsmResponse, core::error::AppError>
    update(const UpdateGsmRuleInput& input) const
    {
        return m_update.update(input);
    }

    [[nodiscard]] std::expected<::app::dto::gsm::GsmDeleteResponse, core::error::AppError>
    remove(const DeleteGsmRuleInput& input) const
    {
        return m_delete.remove(input);
    }

private:
    ::app::context::AppContext& m_context;
    GsmCreateRuntime m_create;
    GsmGetRuntime m_get;
    GsmUpdateRuntime m_update;
    GsmDeleteRuntime m_delete;
};

} // namespace app::handlers::gsm
