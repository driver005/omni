export module app_handlers_external_service_auth:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_validate_token;

export namespace app::handlers::external_service_auth {

class ExternalServiceAuthRuntime
{
public:
    explicit ExternalServiceAuthRuntime(app::context::AppContext& context) noexcept :
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
    app::context::AppContext& m_context;
    ValidateTokenRuntime m_validate_token;
};

} // namespace app::handlers::external_service_auth
