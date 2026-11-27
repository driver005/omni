export module app_auth:hypersense_runtime;

import std;
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

    [[nodiscard]] std::
        expected<::app::dto::external_service_auth::ExternalTokenResponse, core::error::AppError>
        get_token(const GetTokenInput& input) const
    {
        return m_token.get_token(input);
    }

    [[nodiscard]] std::expected<HypersenseVerifyResponse, core::error::AppError>
    verify_token(const VerifyTokenInput& input) const
    {
        return m_verify_token.verify_token(input);
    }

    [[nodiscard]] std::expected<void, core::error::AppError>
    signout_token(const SignoutTokenInput& input) const
    {
        return m_signout_token.signout_token(input);
    }

private:
    ::app::context::AppContext& m_context;
    HypersenseTokenRuntime m_token;
    HypersenseVerifyTokenRuntime m_verify_token;
    HypersenseSignoutTokenRuntime m_signout_token;
};

} // namespace app::handlers::hypersense
