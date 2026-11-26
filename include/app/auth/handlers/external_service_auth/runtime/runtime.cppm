export module app_auth:external_service_auth_runtime;

import std;
import core_error;
import app_context;

import :external_service_auth_input;
import :external_service_auth_result;
import :external_service_auth_runtime_common;
import :external_service_auth_runtime_validate_token;

export namespace app::handlers::external_service_auth {

class ExternalServiceAuthRuntime
{
public:
    explicit ExternalServiceAuthRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_validate_token{context}
    {
    }

    [[nodiscard]] std::expected<ValidateTokenResult, core::error::AppError>
    validate_token(const ValidateTokenInput& input) const
    {
        return m_validate_token.validate_token(input);
    }

private:
    ::app::context::AppContext& m_context;
    ValidateTokenRuntime m_validate_token;
};

} // namespace app::handlers::external_service_auth
