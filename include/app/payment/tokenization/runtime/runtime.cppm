export module app_payment:tokenization_runtime;
import core_behavior;

import std;
import core_error;
import app_context;

import :tokenization_input;
import :tokenization_result;
import :tokenization_runtime_common;
import :tokenization_runtime_create_token_vault_api;
import :tokenization_runtime_delete_tokenized_data_api;

export namespace app::handlers::tokenization {

class TokenizationRuntime
{
public:
    explicit TokenizationRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_delete{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::dto::tokenization::GenericTokenizationResponse>
    create_token_vault(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_create.create_token_vault(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::tokenization::DeleteTokenDataResponse>
    delete_tokenized_data(const core::behavior::CallContext& ctx) const
    {
        return m_delete.delete_tokenized_data(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    CreateTokenVaultRuntime m_create;
    DeleteTokenizedDataRuntime m_delete;
};

} // namespace app::handlers::tokenization
