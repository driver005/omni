export module app_handlers_api_keys:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_create;
import :runtime_retrieve;
import :runtime_update;
import :runtime_revoke;
import :runtime_list;

export namespace app::handlers::api_keys {

class ApiKeysRuntime
{
public:
    explicit ApiKeysRuntime(app::context::AppContext& context) noexcept :
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
    app::context::AppContext& m_context;
    ApiKeyCreateRuntime m_create;
    ApiKeyRetrieveRuntime m_retrieve;
    ApiKeyUpdateRuntime m_update;
    ApiKeyRevokeRuntime m_revoke;
    ApiKeyListRuntime m_list;
};

} // namespace app::handlers::api_keys
