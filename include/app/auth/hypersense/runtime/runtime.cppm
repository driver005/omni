export module app_auth:hypersense_runtime;

import std;
import core_behavior;
import core_error;
import app_context;
import app_dto;

import :hypersense_input;
import :hypersense_result;
import :hypersense_runtime_common;
import :hypersense_runtime_token;
import :hypersense_runtime_verify_token;
import :hypersense_runtime_signout_token;

export namespace app::handlers::hypersense {

class HypersenseRuntime
{
public:
    explicit HypersenseRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_token{context},
        m_verify_token{context},
        m_signout_token{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::external_service_auth::ExternalTokenResponse>
    get_token(const core::behavior::CallContext& ctx) const
    {
        return m_token.get_token(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<HypersenseVerifyResponse> verify_token(
        const ::app::dto::external_service_auth::ExternalVerifyTokenRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_verify_token.verify_token(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<void> signout_token(
        const ::app::dto::external_service_auth::ExternalSignoutTokenRequest& request,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_signout_token.signout_token(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    HypersenseTokenRuntime m_token;
    HypersenseVerifyTokenRuntime m_verify_token;
    HypersenseSignoutTokenRuntime m_signout_token;
};

} // namespace app::handlers::hypersense
