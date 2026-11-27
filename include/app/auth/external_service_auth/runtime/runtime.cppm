export module app_auth:external_service_auth_runtime;

import std;
import core_behavior;
import core_error;
import app_context;
import app_dto;

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

    [[nodiscard]] core::behavior::Outcome<
        ::app::dto::external_service_auth::ExternalVerifyTokenResponse>
    validate_token(
        const ::app::dto::external_service_auth::ValidateTokenRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_validate_token.validate_token(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    ValidateTokenRuntime m_validate_token;
};

} // namespace app::handlers::external_service_auth
