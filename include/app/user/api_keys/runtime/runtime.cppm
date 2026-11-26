export module app_user:api_keys_runtime;

import std;
import core_error;
import app_context;

import :api_keys_input;
import :api_keys_result;
import :api_keys_runtime_common;
import :api_keys_runtime_create;
import :api_keys_runtime_retrieve;
import :api_keys_runtime_update;
import :api_keys_runtime_revoke;
import :api_keys_runtime_list;

export namespace app::handlers::api_keys {

class ApiKeysRuntime
{
public:
    explicit ApiKeysRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_retrieve{context},
        m_update{context},
        m_revoke{context},
        m_list{context}
    {
    }

    [[nodiscard]] std::expected<ApiKeyCreateResult, core::error::AppError>
    create(const ApiKeyCreateInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<ApiKeyRetrieveResult, core::error::AppError>
    retrieve(const ApiKeyRetrieveInput& input) const
    {
        return m_retrieve.retrieve(input);
    }

    [[nodiscard]] std::expected<ApiKeyUpdateResult, core::error::AppError>
    update(const ApiKeyUpdateInput& input) const
    {
        return m_update.update(input);
    }

    [[nodiscard]] std::expected<ApiKeyRevokeResult, core::error::AppError>
    revoke(const ApiKeyRevokeInput& input) const
    {
        return m_revoke.revoke(input);
    }

    [[nodiscard]] std::expected<ApiKeyListResult, core::error::AppError>
    list(const ApiKeyListInput& input) const
    {
        return m_list.list(input);
    }

private:
    ::app::context::AppContext& m_context;
    ApiKeyCreateRuntime m_create;
    ApiKeyRetrieveRuntime m_retrieve;
    ApiKeyUpdateRuntime m_update;
    ApiKeyRevokeRuntime m_revoke;
    ApiKeyListRuntime m_list;
};

} // namespace app::handlers::api_keys
