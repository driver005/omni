export module app_handlers_hypersense:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_token;
import :runtime_verify_token;
import :runtime_signout_token;

export namespace app::handlers::hypersense {

class HypersenseRuntime
{
public:
    explicit HypersenseRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_token{context},
        m_verify_token{context},
        m_signout_token{context}
    {
    }

    [[nodiscard]] std::expected<GetTokenResult, core::error::AppError>
    get_token(const GetTokenInput& input) const
    {
        return m_token.get_token(input);
    }

    [[nodiscard]] std::expected<VerifyTokenResult, core::error::AppError>
    verify_token(const VerifyTokenInput& input) const
    {
        return m_verify_token.verify_token(input);
    }

    [[nodiscard]] std::expected<SignoutTokenResult, core::error::AppError>
    signout_token(const SignoutTokenInput& input) const
    {
        return m_signout_token.signout_token(input);
    }

private:
    app::context::AppContext& m_context;
    HypersenseTokenRuntime m_token;
    HypersenseVerifyTokenRuntime m_verify_token;
    HypersenseSignoutTokenRuntime m_signout_token;
};

} // namespace app::handlers::hypersense
