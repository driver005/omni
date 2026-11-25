export module app_handlers_tokenization:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_create_token_vault_api;
import :runtime_delete_tokenized_data_api;

export namespace app::handlers::tokenization {

class TokenizationRuntime
{
public:
    explicit TokenizationRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_delete{context}
    {
    }

    [[nodiscard]] std::expected<CreateTokenVaultResult, core::error::AppError>
    create_token_vault(const CreateTokenVaultInput& input) const
    {
        return m_create.create_token_vault(input);
    }

    [[nodiscard]] std::expected<DeleteTokenizedDataResult, core::error::AppError>
    delete_tokenized_data(const DeleteTokenizedDataInput& input) const
    {
        return m_delete.delete_tokenized_data(input);
    }

private:
    app::context::AppContext& m_context;
    CreateTokenVaultRuntime m_create;
    DeleteTokenizedDataRuntime m_delete;
};

} // namespace app::handlers::tokenization
