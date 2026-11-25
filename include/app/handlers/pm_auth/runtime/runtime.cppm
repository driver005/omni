export module app_handlers_pm_auth:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_link_token_create;
import :runtime_exchange_token;

export namespace app::handlers::pm_auth {

class PmAuthRuntime
{
public:
    explicit PmAuthRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_link_token_create{context},
        m_exchange_token{context}
    {
    }

    [[nodiscard]] std::expected<LinkTokenCreateResult, core::error::AppError>
    link_token_create(const LinkTokenCreateInput& input) const
    {
        return m_link_token_create.link_token_create(input);
    }

    [[nodiscard]] std::expected<ExchangeTokenResult, core::error::AppError>
    exchange_token(const ExchangeTokenInput& input) const
    {
        return m_exchange_token.exchange_token(input);
    }

private:
    app::context::AppContext& m_context;
    PmAuthLinkTokenCreateRuntime m_link_token_create;
    PmAuthExchangeTokenRuntime m_exchange_token;
};

} // namespace app::handlers::pm_auth
